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


	int master_rank = 0;
	

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




	
	
	float *vector_a;
	float *vector_b;
	float result;

	int n_worker = world_size;
	int total_job = ROW1 * COL2;
	int extra_job = total_job % n_worker;
	int n_round = total_job / n_worker;
	

	srand(time(NULL));


	if (world_rank == master_rank) {
		//master

		double begin;
		double end;
		double time_spent;


		vector_b = malloc(COL1 * sizeof(float));

		
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




		//calculate
		if (!output_for_octave) printf("Calculating...\n");


		float *my_vector_a;
		float *my_vector_b = malloc(COL1 * sizeof(float));

		int round;
		for (round=0; round<n_round; round++) {

			//EXECUTE ROUND

			int my_row;
			int my_col;

			//send
			int proc;
			int col_prev = -1;
			for (proc=0; proc<world_size; proc++) {


				int job_count = world_size * round + proc;
				int row = job_count % COL2;
				int col = job_count / COL2; //they are inverse in purpose


				if (proc == 0) {
					my_row = row;
					my_col = col;
				}


				//assign vectors
				if (col_prev != col) {

					int i;
					for (i=0; i<ROW2; i++) {
						if (proc == 0) my_vector_b[i] = b->values[i][col];
						vector_b[i] = b->values[i][col];
					}

				}
				col_prev = col;

				if (proc == 0) my_vector_a = &a->values[row][0];
				else vector_a = &a->values[row][0];

				

				//send to workers
				if (proc != 0) {

					MPI_Send((void *)vector_a, COL1, MPI_FLOAT, proc, 0, MPI_COMM_WORLD);
					MPI_Send((void *)vector_b, COL1, MPI_FLOAT, proc, 0, MPI_COMM_WORLD);

				}

			}



			//calculate own job
			float my_result = calculate_point(my_vector_a, my_vector_b);
			c->values[my_row][my_col] = my_result;

			if (!output_for_octave) printf("\r%.0f %%", (float) (round * world_size) / total_job * 100);
			

			//recieve
			for (proc=1; proc<world_size; proc++) {

				int job_count = world_size * round + proc;
				int row = job_count % COL2;
				int col = job_count / COL2;

				MPI_Recv(&result, 1, MPI_FLOAT, proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				c->values[row][col] = result;

				if (!output_for_octave) printf("\r%.0f %%", (float) job_count / total_job * 100);

			}


		}

		//execute extra
		int i;
		int col_prev = -1;
		for (i=0; i<extra_job; i++) {

			int proc = i + 1;

			int job_count = world_size * n_round + i;
			int row = job_count % COL2;
			int col = job_count / COL2; //they are inverse in purpose


			//assign vectors
			if (col_prev != col) {

				int i;
				for (i=0; i<ROW2; i++) {
					vector_b[i] = b->values[i][col];
				}

			}
			col_prev = col;

			vector_a = &a->values[row][0];
			

			//send to workers
			MPI_Send((void *)vector_a, COL1, MPI_FLOAT, proc, 0, MPI_COMM_WORLD);
			MPI_Send((void *)vector_b, COL1, MPI_FLOAT, proc, 0, MPI_COMM_WORLD);

		}

		
		for (i=0; i<extra_job; i++) {

			int proc = i + 1;

			int job_count = world_size * n_round + i;
			int row = job_count % COL2;
			int col = job_count / COL2; //they are inverse in purpose


			//revieve
			MPI_Recv(&result, 1, MPI_FLOAT, proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			c->values[row][col] = result;

			if (!output_for_octave) printf("\r%.0f %%", (float) job_count / total_job * 100);

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
		free(my_vector_b);


	}

	else {
		//worker

		float *vector_a = malloc(COL1 * sizeof(float));
		float *vector_b = malloc(COL1 * sizeof(float));

		

		int n_job = total_job / n_worker;
		if (world_rank-1 < extra_job) n_job++;

		int i;
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