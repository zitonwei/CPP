#ifndef MATMUL_H
#define MATMUL_H

#include "matrix.h"

int matmul_plain(const Matrix *a, const Matrix *b, Matrix *c);
int matmul_improved(const Matrix *a, const Matrix *b, Matrix *c);

#endif
