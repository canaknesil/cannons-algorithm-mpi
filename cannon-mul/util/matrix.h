
typedef struct matrix {
	int m;
	int n;
	float **values;
} Matrix;


Matrix *matrix_create(int m, int n);
Matrix *matrix_create_random(int m, int n);
Matrix *matrix_create_zeros(int m, int n);

void matrix_destroy(Matrix *matrix);

void matrix_print(const Matrix *matrix);

int matrix_mul_serial(const Matrix *a, const Matrix *b, Matrix *c);
void matrix_mul_serial_cont(const float *a, const float *b, float *c, int row1, int col1, int col2);
int matrix_add(Matrix *a, const Matrix *b);
void matrix_add_cont(float *a, const float *b, int row, int col);