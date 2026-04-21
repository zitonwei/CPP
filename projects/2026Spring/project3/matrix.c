#include "matrix.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

static int matrix_mul_overflow(size_t a, size_t b, size_t *out)
{
    if (a == 0 || b == 0) {
        *out = 0;
        return 0;
    }

    if (a > SIZE_MAX / b) {
        return 1;
    }

    *out = a * b;
    return 0;
}

int matrix_is_valid(const Matrix *m)
{
    if (m == NULL) {
        return 0;
    }

    if (m->rows == 0 || m->cols == 0) {
        return 0;
    }

    return m->data != NULL;
}

Matrix *matrix_create(size_t rows, size_t cols)
{
    Matrix *m;
    size_t elements;
    size_t bytes;

    if (rows == 0 || cols == 0) {
        return NULL;
    }

    if (matrix_mul_overflow(rows, cols, &elements)) {
        return NULL;
    }

    if (matrix_mul_overflow(elements, sizeof(float), &bytes)) {
        return NULL;
    }

    m = (Matrix *)malloc(sizeof(*m));
    if (m == NULL) {
        return NULL;
    }

    m->rows = rows;
    m->cols = cols;
    m->data = (float *)calloc(elements, sizeof(float));
    if (m->data == NULL) {
        free(m);
        return NULL;
    }

    return m;
}

void matrix_free(Matrix *m)
{
    if (m == NULL) {
        return;
    }

    free(m->data);
    free(m);
}

int matrix_fill_random(Matrix *m, unsigned int seed)
{
    size_t total;
    size_t i;
    unsigned int state;

    if (!matrix_is_valid(m)) {
        return -1;
    }

    total = m->rows * m->cols;
    state = seed;
    for (i = 0; i < total; ++i) {
        state = state * 1664525u + 1013904223u;
        m->data[i] = ((float)(state & 0xFFFFu) / 65535.0f) - 0.5f;
    }

    return 0;
}

void matrix_fill_zero(Matrix *m)
{
    size_t total;
    size_t i;

    if (!matrix_is_valid(m)) {
        return;
    }

    total = m->rows * m->cols;
    for (i = 0; i < total; ++i) {
        m->data[i] = 0.0f;
    }
}

int matrix_copy(Matrix *dst, const Matrix *src)
{
    size_t total;
    size_t i;

    if (!matrix_is_valid(dst) || !matrix_is_valid(src)) {
        return -1;
    }

    if (dst->rows != src->rows || dst->cols != src->cols) {
        return -2;
    }

    total = src->rows * src->cols;
    for (i = 0; i < total; ++i) {
        dst->data[i] = src->data[i];
    }

    return 0;
}

int matrix_compare(
    const Matrix *a,
    const Matrix *b,
    float atol,
    float rtol,
    float *max_abs_diff
)
{
    size_t total;
    size_t i;
    float local_max = 0.0f;

    if (!matrix_is_valid(a) || !matrix_is_valid(b)) {
        return -1;
    }

    if (a->rows != b->rows || a->cols != b->cols) {
        return -2;
    }

    total = a->rows * a->cols;
    for (i = 0; i < total; ++i) {
        float av = a->data[i];
        float bv = b->data[i];
        float diff = av - bv;
        float abs_diff = diff >= 0.0f ? diff : -diff;
        float abs_b = bv >= 0.0f ? bv : -bv;
        float tol = atol + rtol * abs_b;

        if (abs_diff > local_max) {
            local_max = abs_diff;
        }

        if (abs_diff > tol) {
            if (max_abs_diff != NULL) {
                *max_abs_diff = local_max;
            }
            return 0;
        }
    }

    if (max_abs_diff != NULL) {
        *max_abs_diff = local_max;
    }

    return 1;
}

void matrix_print_partial(const Matrix *m, size_t max_rows, size_t max_cols)
{
    size_t i;
    size_t j;
    size_t rows;
    size_t cols;

    if (!matrix_is_valid(m)) {
        printf("<invalid matrix>\n");
        return;
    }

    rows = m->rows < max_rows ? m->rows : max_rows;
    cols = m->cols < max_cols ? m->cols : max_cols;

    for (i = 0; i < rows; ++i) {
        for (j = 0; j < cols; ++j) {
            printf("%10.4f ", m->data[i * m->cols + j]);
        }
        if (cols < m->cols) {
            printf("...");
        }
        printf("\n");
    }

    if (rows < m->rows) {
        printf("...\n");
    }
}
