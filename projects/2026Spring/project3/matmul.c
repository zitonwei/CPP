#include "matmul.h"

#include <stddef.h>
#include <stdlib.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#define MATMUL_BLOCK_SIZE 64u

static int matmul_validate_args(const Matrix *a, const Matrix *b, Matrix *c)
{
    if (!matrix_is_valid(a) || !matrix_is_valid(b) || !matrix_is_valid(c)) {
        return -1;
    }

    if (a->cols != b->rows) {
        return -2;
    }

    if (c->rows != a->rows || c->cols != b->cols) {
        return -3;
    }

    return 0;
}

int matmul_plain(const Matrix *a, const Matrix *b, Matrix *c)
{
    size_t i;
    size_t j;
    size_t k;
    int rc;

    rc = matmul_validate_args(a, b, c);
    if (rc != 0) {
        return rc;
    }

    matrix_fill_zero(c);

    for (i = 0; i < a->rows; ++i) {
        for (j = 0; j < b->cols; ++j) {
            float sum = 0.0f;
            for (k = 0; k < a->cols; ++k) {
                sum += a->data[i * a->cols + k] * b->data[k * b->cols + j];
            }
            c->data[i * c->cols + j] = sum;
        }
    }

    return 0;
}

int matmul_improved(const Matrix *a, const Matrix *b, Matrix *c)
{
    size_t i0;
    size_t j0;
    size_t k0;
    size_t total_bt;
    float *bt;
    int rc;

    rc = matmul_validate_args(a, b, c);
    if (rc != 0) {
        return rc;
    }

    total_bt = b->rows * b->cols;
    bt = (float *)malloc(total_bt * sizeof(float));
    if (bt == NULL) {
        return -4;
    }

    for (i0 = 0; i0 < b->rows; ++i0) {
        for (j0 = 0; j0 < b->cols; ++j0) {
            bt[j0 * b->rows + i0] = b->data[i0 * b->cols + j0];
        }
    }

    matrix_fill_zero(c);

    /*
     * Faster than the baseline for two reasons:
     * 1. B is transposed once so the inner loop reads both A and B with
     *    contiguous access patterns.
     * 2. Blocking keeps working data in cache longer, reducing memory traffic.
     */
#ifdef _OPENMP
#pragma omp parallel for schedule(static) private(j0, k0)
#endif
    for (i0 = 0; i0 < a->rows; i0 += MATMUL_BLOCK_SIZE) {
        size_t i1 = i0 + MATMUL_BLOCK_SIZE < a->rows ? i0 + MATMUL_BLOCK_SIZE : a->rows;

        for (j0 = 0; j0 < b->cols; j0 += MATMUL_BLOCK_SIZE) {
            size_t j1 = j0 + MATMUL_BLOCK_SIZE < b->cols ? j0 + MATMUL_BLOCK_SIZE : b->cols;

            for (k0 = 0; k0 < a->cols; k0 += MATMUL_BLOCK_SIZE) {
                size_t k1 = k0 + MATMUL_BLOCK_SIZE < a->cols ? k0 + MATMUL_BLOCK_SIZE : a->cols;
                size_t i;
                size_t j;

                for (i = i0; i < i1; ++i) {
                    const float *a_row = &a->data[i * a->cols];
                    float *c_row = &c->data[i * c->cols];

                    for (j = j0; j < j1; ++j) {
                        const float *bt_row = &bt[j * b->rows];
                        float sum = c_row[j];
                        size_t k;

                        for (k = k0; k < k1; ++k) {
                            sum += a_row[k] * bt_row[k];
                        }

                        c_row[j] = sum;
                    }
                }
            }
        }
    }

    free(bt);
    return 0;
}
