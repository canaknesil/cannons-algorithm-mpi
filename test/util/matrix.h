
typedef struct matrix {
	int m;
	int n;
	float **values;
} Matrix;

Matrix *matrix_create(int m, int n);
void matrix_destroy(Matrix *matrix);

void matrix_print(Matrix * matrix);


int mm_mul_serial(Matrix *a, Matrix *b, Matrix *c);