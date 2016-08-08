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
	while ((c = getopt(argc, argv, "s:")) != -1) {

		switch (c) {
			case 's': 
				ROW1 = ROW2 = COL1 = COL2 = atoi(optarg);
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

		MPI_Status *statSend = malloc(total_job * sizeof(MPI_Status));
		MPI_Status *statRecv = malloc(total_job * sizeof(MPI_Status));

		MPI_Request *reqSend_1 = malloc(total_job * sizeof(MPI_Request));
		MPI_Request *reqSend_2 = malloc(total_job * sizeof(MPI_Request));
		MPI_Request *reqRecv = malloc(total_job * sizeof(MPI_Request));


		//create matrices
		Matrix *a = matrix_create(ROW1, COL1);
		Matrix *b = matrix_create(ROW2, COL2);
		Matrix *c = matrix_create(ROW1, COL2);

		create_random_matrix(a);
		create_random_matrix(b);
		
		//print matrices
		/*printf("Matrix A:\n");
		matrix_print(a);

		printf("Matrix B:\n");
		matrix_print(b);*/

		begin = MPI_Wtime();

		//send
		printf("Sending...\n");
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

				MPI_Isend((void *)vector_a, COL1, MPI_FLOAT, destination, 0, MPI_COMM_WORLD, &reqSend_1																																																																																																																																																																																																																																																									[job_count]);
				MPI_Isend((void *)vector_b, COL1, MPI_FLOAT, destination, 0, MPI_COMM_WORLD, &reqSend_2[job_count]);

				count++;
				printf("\r%.0f %%", (float) count / total_job * 100);

			}

		}
		printf("\n");



		//recieve
		printf("Recieving...\n");
		for (i=0; i<total_job; i++) {

			
			MPI_Irecv(&result, 1, MPI_FLOAT, i % n_worker, 0, MPI_COMM_WORLD, &reqRecv[i]);
			MPI_Wait(&reqRecv[i], &statRecv[i]);
			c->values[i/COL2][i%COL2] = result;

			printf("\r%.0f %%", (float) i / total_job * 100);

		}
		printf("\n");



		end = MPI_Wtime();
		time_spent = end - begin;

		printf("Turnaround time: %f\n", time_spent);


		//print result
		/*printf("Matrix C:\n");
		matrix_print(c);*/


		//free allocated spaces
		matrix_destroy(a);
		matrix_destroy(b);
		matrix_destroy(c);

		free(vector_b);

		free(statSend);
		free(statRecv);
		free(reqSend_1);
		free(reqSend_2);
		free(reqRecv);


	}

	else {
		//worker

		float *vector_a = malloc(COL1 * sizeof(float));
		float *vector_b = malloc(COL1 * sizeof(float));

		MPI_Status statRecv[2];

		MPI_Request reqSend;
		MPI_Request reqRecv[2];


		int n_job = total_job / n_worker;
		if (world_rank < extra) n_job++;

		
		for (i=0; i<n_job; i++) {


			//receive vectors
			MPI_Irecv(vector_a, COL1, MPI_FLOAT, master_rank, 0, MPI_COMM_WORLD, &reqRecv[0]);
			MPI_Wait(&reqRecv[0], &statRecv[0]);

			MPI_Irecv(vector_b, COL1, MPI_FLOAT, master_rank, 0, MPI_COMM_WORLD, &reqRecv[1]);
			MPI_Wait(&reqRecv[1], &statRecv[1]);

			
			//calculate point
			result = calculate_point(vector_a, vector_b);

			//send result
			MPI_Isend(&result, 1, MPI_FLOAT, master_rank, 0, MPI_COMM_WORLD, &reqSend);


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