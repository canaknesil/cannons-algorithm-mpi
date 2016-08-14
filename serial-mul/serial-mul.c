#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <mpi.h>

#include "util/matrix.h"


/*#define ROW1 3
#define COL1 3

#define ROW2 COL1
#define COL2 3*/


void create_random_matrix(Matrix *matrix);
float calculate_point(const float *vector_a, const float *vector_b);
void print_vector(const float *vector);



int ROW1 = 3;
int COL1 = 3;
int ROW2 = 3;
int COL2 = 3;

int print_matrices = 0;
int output_for_octave = 0;


int main(int argc, char *argv[]) {


	MPI_Init(&argc, &argv);

	int world_size;
	int world_rank;

	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);



	int master_rank = world_size - 1;

	//checking number of process
	if (world_size < 2) {
		if (world_rank == master_rank) printf("Number of process must be minimum 2. Quitting...\n");
		MPI_Finalize();
		exit(1);
	}




	//getting arguments
	int c;
	while ((c = getopt(argc, argv, "s:po")) != -1) {

		switch (c) {
			case 's': 
				ROW1 = ROW2 = COL1 = COL2 = atoi(optarg);
				break;
			case 'p':
				print_matrices = 1;
				break;
			case 'o': 
				output_for_octave = 1;
				break;
			default:
				if (world_rank == master_rank) printf("Illegal arguments. Quitting...\n");
				MPI_Finalize();
				exit(1);
		}

	}




	int i, j;
	float result;
	int n_worker = world_size - 1;
	int total_job = ROW1 * COL2;
	int extra = total_job % n_worker;

	srand(time(NULL));


	if (world_rank == master_rank) {
		//master

		double begin;
		double end;
		double time_spent;

		float *vector_a;
		float *vector_b = malloc(COL1 * sizeof(float));

		
		//create matrices
		Matrix *a = matrix_create(ROW1, COL1);
		Matrix *b = matrix_create(ROW2, COL2);
		Matrix *c = matrix_create(ROW1, COL2);

		create_random_matrix(a);
		create_random_matrix(b);
		
		//print matrices
		if (print_matrices) {
			printf("Matrix A:\n");
			matrix_print(a);

			printf("Matrix B:\n");
			matrix_print(b);
		}
		

		begin = MPI_Wtime();

		//send
		if (!output_for_octave) printf("Sending...\n");
		int count = 0;
		for (j=0; j<COL2; j++) {

			int k;
			for (k=0; k<ROW2; k++) {
				vector_b[k] = b->values[k][j];
			}

			for (i=0; i<ROW1; i++) {


				vector_a = &a->values[i][0];
				int job_count = COL2 * i + j;
				int destination = (job_count) % n_worker;

				MPI_Send((void *)vector_a, COL1, MPI_FLOAT, destination, 0, MPI_COMM_WORLD);
				MPI_Send((void *)vector_b, COL1, MPI_FLOAT, destination, 0, MPI_COMM_WORLD);

				count++;
				if (!output_for_octave)
					printf("\r%.0f %%", (float) count / total_job * 100);

			}

		}
		if (!output_for_octave) printf("\n");



		//recieve
		if (!output_for_octave) printf("Recieving...\n");
		for (i=0; i<total_job; i++) {

			
			MPI_Recv(&result, 1, MPI_FLOAT, i % n_worker, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			c->values[i/COL2][i%COL2] = result;

			if (!output_for_octave) printf("\r%.0f %%", (float) i / total_job * 100);

		}
		if (!output_for_octave) printf("\n");



		end = MPI_Wtime();
		time_spent = end - begin;

		if (!output_for_octave) printf("Turnaround time: ");
		printf("%f\n", time_spent);


		//print result
		if (print_matrices) {
			printf("Matrix C:\n");
			matrix_print(c);
		}
		


		//free allocated spaces
		matrix_destroy(a);
		matrix_destroy(b);
		matrix_destroy(c);

		free(vector_b);


	}

	else {
		//worker

		float *vector_a = malloc(COL1 * sizeof(float));
		float *vector_b = malloc(COL1 * sizeof(float));

		

		int n_job = total_job / n_worker;
		if (world_rank < extra) n_job++;

		
		for (i=0; i<n_job; i++) {


			//receive vectors
			MPI_Recv(vector_a, COL1, MPI_FLOAT, master_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(vector_b, COL1, MPI_FLOAT, master_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			
			//calculate point
			result = calculate_point(vector_a, vector_b);

			//send result
			MPI_Send(&result, 1, MPI_FLOAT, master_rank, 0, MPI_COMM_WORLD);


		}

		

		free(vector_a);
		free(vector_b);

	}


	

	MPI_Finalize();


	return 0;
}


void create_random_matrix(Matrix *matrix) {

	int i, j;
	int m = matrix->m;
	int n = matrix->n;

	for (i = 0; i < m; i++) {
		for (j = 0; j < n; j++) {
			matrix->values[i][j] = (float)rand() / (float)RAND_MAX;
		}
	}

}

float calculate_point(const float *vector_a, const float *vector_b) {


	float result = 0;
	int i;
	for (i=0; i<COL1; i++) {
		result += vector_a[i] * vector_b[i];
	}
	
	return result;

}

void print_vector(const float *vector) {

	int i;
	for (i=0; i<COL1; i++) printf("%f ", vector[i]);
		printf("\n");

}