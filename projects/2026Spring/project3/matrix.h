#ifndef MATRIX_H
#define MATRIX_H

#include <stddef.h>

typedef struct {
    size_t rows;
    size_t cols;
    float *data;
} Matrix;

Matrix *matrix_create(size_t rows, size_t cols);
void matrix_free(Matrix *m);

int matrix_is_valid(const Matrix *m);
int matrix_fill_random(Matrix *m, unsigned int seed);
void matrix_fill_zero(Matrix *m);
int matrix_copy(Matrix *dst, const Matrix *src);

int matrix_compare(
    const Matrix *a,
    const Matrix *b,
    float atol,
    float rtol,
    float *max_abs_diff
);

void matrix_print_partial(const Matrix *m, size_t max_rows, size_t max_cols);

#endif
