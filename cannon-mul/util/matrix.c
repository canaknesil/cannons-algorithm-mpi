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

Matrix *matrix_create_random(int m, int n) {

	Matrix *matrix = matrix_create(m, n);

	int i, j;

	for (i = 0; i < m; i++) {
		for (j = 0; j < n; j++) {
			matrix->values[i][j] = (float)rand() / (float)RAND_MAX;
		}
	}

	return matrix;

}

Matrix *matrix_create_zeros(int m, int n) {

	Matrix *matrix = matrix_create(m, n);

	int i, j;

	for (i = 0; i < m; i++) {
		for (j = 0; j < n; j++) {
			matrix->values[i][j] = 0;
		}
	}

	return matrix;

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

int matrix_mul_serial(const Matrix *a, const Matrix *b, Matrix *c) {

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

void matrix_mul_serial_cont(const float *a, const float *b, float *c, int row1, int col1, int col2) {

	int i, j, k;

	for (i=0; i < row1; i++) {
		for (j=0; j < col2; j++) {

			c[i * col2 + j] = 0;
			for (k=0; k<col1; k++) {
		
				c[i * col2 + j] += a[i * col1 + k] * b[k * col2 + j];

			}

		}
	}

}

int matrix_add(Matrix *a, const Matrix *b) {

	if (a->m != b->m || a->n != b->n) {
		return 0;
	}

	int i, j;
	for (i=0; i<a->m; i++) {
		for (j=0; j<a->n; j++) {
			a->values[i][j] += b->values[i][j];
		}
	}

	return 1;

}

void matrix_add_cont(float *a, const float *b, int row, int col) {

	int i, j;
	for (i=0; i<row; i++) {
		for (j=0; j<col; j++) {
			a[i * col + j] += b[i * col + j];
		}
	}

}