#include "matrix.h"

void matmul_with_factor(struct Matrix* A, struct Matrix* B, struct Matrix* C, 
						float alpha, float beta) {
	int m = A->row;
    int k = A->col;
    int n = B->col;
	cblas_sgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, m, 
						n, k, alpha, A->data, m, B->data, k, beta, C->data, m);
}

void add_constant(struct Matrix* A, float alpha){
	int m = A->row;
    int k = A->col;
	for(int i=0;i<m*k;i++){
		A->data[i] += alpha;
	}
}

void matmul_with_bias(struct Matrix* A, struct Matrix* B, struct Matrix* C)
{
    //cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, m, \
                        n, k, alpha, A, k, B, n, beta, C, n);
    int m = A->row;
    int k = A->col;
    int n = B->col;
    C->row = A->row;
    C->col = B->col;
    cblas_sgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, m, 
                        n, k, 1, A->data, m, B->data, k, 1, C->data, m);
}

void matmul(struct Matrix* A, struct Matrix* B, struct Matrix* C)
{
    //cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, m, \
    					n, k, alpha, A, k, B, n, beta, C, n);
    int m = A->row;
    int k = A->col;
    int n = B->col;
    C->row = A->row;
    C->col = B->col;
    cblas_sgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, m, 
    					n, k, 1, A->data, m, B->data, k, 0, C->data, m);
}

void multiply(struct Matrix* A, struct Matrix* B){
    for(int i=0;i<A->row*A->col;i++){
        A->data[i] = A->data[i]*B->data[i];
    }
}

void *multiply_args(void *args){
    compute_multiply *actual_args = args;
    for(int i=0;i<10000000;i++){
        multiply(actual_args->A, actual_args->B);
    }
}

void relu(struct Matrix* A){
    for(int i=0;i<A->col*A->row;i++){
        if(A->data[i]<0){
            A->data[i] = 0;
        }
    }
}

void printMatrix(struct Matrix* A){
    //printf("%d %d\n", A->row, A->col);

    if(A->col==1){
        printf("[");
        for(int i=0;i<A->row;i++){
            printf("%f ", A->data[i]);
        }
        printf("]\n");
        return;
    }
    if(A->row==1){
        printf("[");
        for(int i=0;i<A->col;i++){
            printf("%f ", A->data[i]);
        }
        printf("]\n");
        return;
    }
    printf("[" );
    for(int i=0;i<A->col;i++){
        printf("[");
        for(int j=0;j<A->row;j++){
            printf("%f ",A->data[A->row*i+j]);
        }
        if(i==A->col-1){
            printf("]");
        }
        else{
            printf("]\n");
        }
        
    }
    printf("]\n");
}

void fprintMatrix(FILE *fp, struct Matrix* A){
    //printf("%d %d\n", A->row, A->col);

    if(A->col==1){
        fprintf(fp, "[");
        for(int i=0;i<A->row;i++){
            fprintf(fp, "%f ", A->data[i]);
        }
        fprintf(fp, "]\n");
        return;
    }
    if(A->row==1){
        fprintf(fp, "[");
        for(int i=0;i<A->col;i++){
            fprintf(fp, "%f ", A->data[i]);
        }
        fprintf(fp, "]\n");
        return;
    }
    fprintf(fp, "[" );
    for(int i=0;i<A->col;i++){
        fprintf(fp, "[");
        for(int j=0;j<A->row;j++){
            fprintf(fp, "%f ",A->data[A->row*i+j]);
        }
        if(i==A->col-1){
            fprintf(fp,"]");
        }
        else{
            fprintf(fp, "]\n");
        }
        
    }
    fprintf(fp, "]\n");
}
