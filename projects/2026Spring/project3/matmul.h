#ifndef MATMUL_H
#define MATMUL_H

#include "matrix.h"

int matmul_plain(const Matrix *a, const Matrix *b, Matrix *c);
int matmul_improved_v1(const Matrix *a, const Matrix *b, Matrix *c);
int matmul_improved_v2(const Matrix *a, const Matrix *b, Matrix *c);
int matmul_improved_v3(const Matrix *a, const Matrix *b, Matrix *c);

#endif
