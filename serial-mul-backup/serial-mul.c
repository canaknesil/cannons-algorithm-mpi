#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#include "util/matrix.h"


#define ROW1 3
#define COL1 3

#define ROW2 COL1
#define COL2 3


void create_random_matrix(Matrix *matrix);


int main() {

	int world_size;
	int world_rank;
	int n_proc = ROW1 * COL2;


	MPI_Init(NULL, NULL);

	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);


	if (world_size != n_proc) {
		if (world_rank == 0) printf("There must be %d processes. Quitting\n", n_proc);
		MPI_Finalize();
		exit(1);
	}




	if (world_rank == 0) {
		//master

		int i, j;
		Vector *va[ROW1];
		Vector *vb[COL2];

		//create matrices
		Matrix *a = matrix_create(ROW1, COL1);
		Matrix *b = matrix_create(ROW2, COL2);
		Matrix *c = matrix_create(ROW1, COL2);

		create_random_matrix(a);
		create_random_matrix(b);
		

		//multiply
		for (i=0; i<ROW1; i++) {
			va[i] = vector_create(COL1);
			va[i]->values = a->values[i];
		}

		for (i=0; i<COL2; i++) {
			vb[i] = vector_create(ROW2);

			for (j=0; j<ROW2; j++) {
				vb[i]->values[j] = b->values[j][i];
			}

		}


		for (i=0; i<ROW1; i++) {
			for (j=0; j<COL2; j++) {
				vector_mul(va[i], vb[j], &c->values[i][j]);
			}

		}



		//print matrices
		printf("Matrix A:\n");
		matrix_print(a);

		printf("Matrix B:\n");
		matrix_print(b);

		printf("Matrix C:\n");
		matrix_print(c);


		//destroy matrices
		matrix_destroy(a);
		matrix_destroy(b);
		matrix_destroy(c);


	}

	else {
		//worker




	}



	MPI_Finalize();


	return 0;
}


void create_random_matrix(Matrix *matrix) {

	int i, j;
	int m = matrix->m;
	int n = matrix->n;

	srand(time(NULL));

	for (i = 0; i < m; i++) {
		for (j = 0; j < n; j++) {
			matrix->values[i][j] = (float)rand() / (float)RAND_MAX;
		}
	}

}