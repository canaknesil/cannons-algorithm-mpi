
typedef struct vector {
	int n;
	float *values;
} Vector;

Vector *vector_create(int n);
void vector_destroy(Vector *vector);
int vector_mul(const Vector *a, const Vector *b, float *result);



typedef struct matrix {
	int m;
	int n;
	float **values;
} Matrix;


Matrix *matrix_create(int m, int n);
void matrix_destroy(Matrix *matrix);

void matrix_print(const Matrix * matrix);
int mm_mul_serial(const Matrix *a, const Matrix *b, Matrix *c);
