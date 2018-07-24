/*
 -----------------------------------------------------------------
 ** Top contributors:
 **   Shiqi Wang and Suman Jana
 ** This file is part of the ReluVal project.
 ** Copyright (c) 2018-2019 by the authors listed in the file LICENSE
 ** and their institutional affiliations.
 ** All rights reserved.
 -----------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <cblas.h>

#ifndef MATRIX_H
#define MATRIX_H


/* Define the structure of Matrix */
struct Matrix
{
    float* data;
    int row, col;
};


/* add the constant to matrix */
void add_constant(struct Matrix* A, float alpha);


/*matrix multiplication with factors */
void matmul_with_factor(struct Matrix* A,\
                        struct Matrix* B,\
                        struct Matrix* C,\
                        float alpha,\
                        float beta);

/* matrix multiplication */
void matmul(struct Matrix* A,\
            struct Matrix* B,\
            struct Matrix* C);


/* matrix multiplication with bias */
void matmul_with_bias(struct Matrix* A,\
                      struct Matrix* B,\
                      struct Matrix* C);


/* element-wise multiplication */
void multiply(struct Matrix* A, struct Matrix* B);


/* print matrix */
void printMatrix(struct Matrix* A);


/* print matrix to the file */
void fprintMatrix(FILE *fp, struct Matrix* A);


/* takes the relu of the matrix */
void relu(struct Matrix* A);


#endif