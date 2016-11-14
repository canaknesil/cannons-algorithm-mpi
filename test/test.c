#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#include "util/matrix.h"


int main() {

	int world_size;
	int world_rank;

	MPI_Init(NULL, NULL);

	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);



	if (world_size != 9) {
		if (world_rank == 0) printf("There must be 9 processes. Quitting\n");
		MPI_Finalize();
		exit(1);
	}


	if (world_rank == 0) {

		int n = 3;
		Matrix *a = matrix_create(n, n);
		Matrix *b = matrix_create(n, n);
		Matrix *c = matrix_create(n, n);

		int i, j;

		srand(time(NULL));

		for (i = 0; i < n; i++) {
			for (j = 0; j < n; j++) {
				a->values[i][j] = (float)rand() / (float)RAND_MAX;
				b->values[i][j] = (float)rand() / (float)RAND_MAX;
			}
		}

		mm_mul_serial(a, b, c);

		printf("Matrix A:\n");
		matrix_print(a);

		printf("Matrix B:\n");
		matrix_print(b);

		printf("Matrix C:\n");
		matrix_print(c);


		matrix_destroy(a);
		matrix_destroy(b);
		matrix_destroy(c);


	}
	else {




	}



	MPI_Finalize();


	return 0;
}