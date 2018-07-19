#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <cblas.h>

struct Matrix
{
	float* data;
	int row, col;
};

typedef struct {
    //Or whatever information that you need
    struct Matrix *A;
    struct Matrix *B;
} compute_multiply;


void add_constant(struct Matrix* A, float alpha);

void matmul_with_factor(struct Matrix* A, struct Matrix* B, struct Matrix* C, 
						float alpha, float beta);

void matmul(struct Matrix* A, struct Matrix* B, struct Matrix* C);

void matmul_with_bias(struct Matrix* A, struct Matrix* B, struct Matrix* C);

void multiply(struct Matrix* A, struct Matrix* B);


void *multiply_args(void *args);

void printMatrix(struct Matrix* A);

void fprintMatrix(FILE *fp, struct Matrix* A);

void relu(struct Matrix* A);