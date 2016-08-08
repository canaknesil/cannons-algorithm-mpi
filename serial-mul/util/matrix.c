#include "matrix.h"
#include <stdlib.h>
#include <stdio.h>



Matrix *matrix_create(int m, int n) {

	Matrix *mat = malloc(sizeof(Matrix));
	int i;

	mat->m = m;
	mat->n = n;

	mat->values = malloc(m * sizeof(float *));

	for (i = 0; i < m; i++) {
		mat->values[i] = malloc(n * sizeof(float));
	}
	
	return mat;
}

void matrix_destroy(Matrix *matrix) {
	int i, j;

	for (i = 0; i < matrix->m; i++) {
		free(matrix->values[i]);
	}
	free(matrix->values);
	free(matrix);
}

void matrix_print(const Matrix * matrix) {
	int i, j;
	for (i = 0; i < matrix->m; i++) {
		for (j = 0; j < matrix->n; j++) {
			printf("%f ", matrix->values[i][j]);
		}
		printf("\n");
	}
}

int mm_mul_serial(const Matrix *a, const Matrix *b, Matrix *c) {

	int i, j, k;

	if (a->n != b->m) {
		return 0;
	}

	c->m = a->m;
	c->n = c->n;

	for (i=0; i < a->m; i++) {
		for (j=0; j < b->n; j++) {

			c->values[i][j] = 0;
			for (k=0; k<a->n; k++) {
		
				c->values[i][j] += a->values[i][k] * b->values[k][j];

			}

		}
	}

	return 1;
}