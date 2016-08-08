
typedef struct matrix {
	int m;
	int n;
	float **values;
} Matrix;


Matrix *matrix_create(int m, int n);
void matrix_destroy(Matrix *matrix);

void matrix_print(const Matrix * matrix);
int mm_mul_serial(const Matrix *a, const Matrix *b, Matrix *c);
