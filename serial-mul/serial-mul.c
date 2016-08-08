#include <stdio.h>
#include <stdlib.h>
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



int ROW1;
int COL1;
int ROW2;
int COL2;


int main(int argc, char *argv[]) {

	MPI_Init(&argc, &argv);

	int world_size;
	int world_rank;

	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);


	if (argc != 2) {
		if (world_rank == 0) printf("Enter size of the matrices. Quitting\n");
		MPI_Finalize();
		exit(1);
	}
	ROW1 = ROW2 = COL1 = COL2 = atoi(argv[1]);

	int n_proc = ROW1 * COL2;

	


	if (world_size != n_proc) {
		if (world_rank == 0) printf("There must be %d processes. Quitting\n", n_proc);
		MPI_Finalize();
		exit(1);
	}



	int i, j;
	float result;

	srand(time(NULL));


	if (world_rank == 0) {
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
		/*printf("Matrix A:\n");
		matrix_print(a);

		printf("Matrix B:\n");
		matrix_print(b);*/

		begin = MPI_Wtime();

		//send
		for (j=0; j<COL2; j++) {

			int k;
			for (k=0; k<ROW2; k++) {
				vector_b[k] = b->values[k][j];
			}

			for (i=0; i<ROW1; i++) {


				vector_a = &a->values[i][0];

				if (i!=0 || j!=0) {

					int destination = COL2 * i + j;

					MPI_Send((void *)vector_a, COL1, MPI_FLOAT, destination, 0, MPI_COMM_WORLD);
					MPI_Send((void *)vector_b, COL1, MPI_FLOAT, destination, 0, MPI_COMM_WORLD);

				}
				else {

					//do master's own job
					result = calculate_point(vector_a, vector_b);
					c->values[0][0] = result;

				}

				
			}

		}



		//recieve
		for (i=1; i<n_proc; i++) {

			
			MPI_Recv(&result, 1, MPI_FLOAT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			c->values[i/COL2][i%COL2] = result;


		}


		end = MPI_Wtime();
		time_spent = end - begin;

		printf("%f\n", time_spent);


		//print result
		/*printf("Matrix C:\n");
		matrix_print(c);*/


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
		//receive vectors
		MPI_Recv(vector_a, COL1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(vector_b, COL1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		
		//calculate point
		result = calculate_point(vector_a, vector_b);


		//send result
		MPI_Send(&result, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);

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