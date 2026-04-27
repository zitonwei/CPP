#include "matmul.h"

#include <stddef.h>
#include <stdlib.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#define MATMUL_BLOCK_SIZE 64u
#define STRASSEN_THRESHOLD 128u

static int matmul_validate_args(const Matrix *a, const Matrix *b, Matrix *c) {
    if (!matrix_is_valid(a) || !matrix_is_valid(b) || !matrix_is_valid(c)) return -1;
    if (a->cols != b->rows) return -2;
    if (c->rows != a->rows || c->cols != b->cols) return -3;
    return 0;
}

static int matrix_add(const Matrix *a, const Matrix *b, Matrix *out, int sign_b) {
    size_t i, total;

    if (!matrix_is_valid(a) || !matrix_is_valid(b) || !matrix_is_valid(out)) return -1;
    if (a->rows != b->rows || a->cols != b->cols || a->rows != out->rows || a->cols != out->cols) return -2;

    total = out->rows * out->cols;
    for (i = 0; i < total; ++i) out->data[i] = a->data[i] + (sign_b > 0 ? b->data[i] : -b->data[i]);
    return 0;
}

static int matrix_extract_block(const Matrix *src, size_t row0, size_t col0, Matrix *dst) {
    size_t i, j;

    if (!matrix_is_valid(src) || !matrix_is_valid(dst)) return -1;
    if (row0 + dst->rows > src->rows || col0 + dst->cols > src->cols) return -2;

    for (i = 0; i < dst->rows; ++i)
        for (j = 0; j < dst->cols; ++j)
            dst->data[i * dst->cols + j] = src->data[(row0 + i) * src->cols + (col0 + j)];

    return 0;
}

static void matrix_store_block(Matrix *dst, size_t row0, size_t col0, const Matrix *src) {
    size_t i, j;

    for (i = 0; i < src->rows; ++i)
        for (j = 0; j < src->cols; ++j)
            dst->data[(row0 + i) * dst->cols + (col0 + j)] = src->data[i * src->cols + j];
}

int matmul_plain(const Matrix *a, const Matrix *b, Matrix *c) {
    size_t i, j, k;
    int rc;

    rc = matmul_validate_args(a, b, c);
    if (rc != 0) return rc;

    matrix_fill_zero(c);
    for (i = 0; i < a->rows; ++i)
        for (j = 0; j < b->cols; ++j)
            for (k = 0; k < a->cols; ++k)
                c->data[i * c->cols + j] += a->data[i * a->cols + k] * b->data[k * b->cols + j];

    return 0;
}

int matmul_improved_v1(const Matrix *a, const Matrix *b, Matrix *c) {
    size_t i, k;
    int rc;

    rc = matmul_validate_args(a, b, c);
    if (rc != 0) return rc;

    matrix_fill_zero(c);

    /*
     * v1: reorder the baseline from i-j-k to i-k-j.
     * The inner loop then walks across a full row of B and C, which is more
     * cache-friendly and also easier for the compiler to auto-vectorize.
     */
    for (i = 0; i < a->rows; ++i) {
        const float *a_row = &a->data[i * a->cols];
        float *c_row = &c->data[i * c->cols];

        for (k = 0; k < a->cols; ++k) {
            const float a_ik = a_row[k];
            const float *b_row = &b->data[k * b->cols];
            size_t j;

            for (j = 0; j < b->cols; ++j) c_row[j] += a_ik * b_row[j];
        }
    }

    return 0;
}

int matmul_improved_v2(const Matrix *a, const Matrix *b, Matrix *c) {
    size_t i0, j0, k0;
    int rc;

    rc = matmul_validate_args(a, b, c);
    if (rc != 0) return rc;

    matrix_fill_zero(c);

    /*
     * v2: add cache blocking on top of the ikj kernel.
     * Working on smaller tiles reduces cache misses on larger matrices.
     */
#ifdef _OPENMP
#pragma omp parallel for schedule(static) private(j0, k0)
#endif
    for (i0 = 0; i0 < a->rows; i0 += MATMUL_BLOCK_SIZE) {
        size_t i1 = i0 + MATMUL_BLOCK_SIZE < a->rows ? i0 + MATMUL_BLOCK_SIZE : a->rows;

        for (k0 = 0; k0 < a->cols; k0 += MATMUL_BLOCK_SIZE) {
            size_t k1 = k0 + MATMUL_BLOCK_SIZE < a->cols ? k0 + MATMUL_BLOCK_SIZE : a->cols;

            for (j0 = 0; j0 < b->cols; j0 += MATMUL_BLOCK_SIZE) {
                size_t j1 = j0 + MATMUL_BLOCK_SIZE < b->cols ? j0 + MATMUL_BLOCK_SIZE : b->cols;
                size_t i;

                for (i = i0; i < i1; ++i) {
                    const float *a_row = &a->data[i * a->cols];
                    float *c_row = &c->data[i * c->cols];
                    size_t k;

                    for (k = k0; k < k1; ++k) {
                        const float a_ik = a_row[k];
                        const float *b_row = &b->data[k * b->cols];
                        size_t j;

                        for (j = j0; j < j1; ++j) c_row[j] += a_ik * b_row[j];
                    }
                }
            }
        }
    }

    return 0;
}

static int matmul_strassen_recursive(const Matrix *a, const Matrix *b, Matrix *c) {
    size_t n, half;
    Matrix *a11, *a12, *a21, *a22, *b11, *b12, *b21, *b22;
    Matrix *s1, *s2, *m1, *m2, *m3, *m4, *m5, *m6, *m7;
    Matrix *c11, *c12, *c21, *c22;
    int rc = 0;

    n = a->rows;
    if (n <= STRASSEN_THRESHOLD) return matmul_improved_v2(a, b, c);
    if (a->rows != a->cols || b->rows != b->cols || c->rows != c->cols ||
        a->rows != b->rows || a->rows != c->rows || (n & 1u) != 0u)
        return matmul_improved_v2(a, b, c);

    half = n / 2u;
    a11 = matrix_create(half, half);
    a12 = matrix_create(half, half);
    a21 = matrix_create(half, half);
    a22 = matrix_create(half, half);
    b11 = matrix_create(half, half);
    b12 = matrix_create(half, half);
    b21 = matrix_create(half, half);
    b22 = matrix_create(half, half);
    s1 = matrix_create(half, half);
    s2 = matrix_create(half, half);
    m1 = matrix_create(half, half);
    m2 = matrix_create(half, half);
    m3 = matrix_create(half, half);
    m4 = matrix_create(half, half);
    m5 = matrix_create(half, half);
    m6 = matrix_create(half, half);
    m7 = matrix_create(half, half);
    c11 = matrix_create(half, half);
    c12 = matrix_create(half, half);
    c21 = matrix_create(half, half);
    c22 = matrix_create(half, half);

    if (a11 == NULL || a12 == NULL || a21 == NULL || a22 == NULL ||
        b11 == NULL || b12 == NULL || b21 == NULL || b22 == NULL ||
        s1 == NULL || s2 == NULL || m1 == NULL || m2 == NULL || m3 == NULL ||
        m4 == NULL || m5 == NULL || m6 == NULL || m7 == NULL ||
        c11 == NULL || c12 == NULL || c21 == NULL || c22 == NULL) {
        rc = -4;
        goto cleanup;
    }

    if ((rc = matrix_extract_block(a, 0, 0, a11)) != 0) goto cleanup;
    if ((rc = matrix_extract_block(a, 0, half, a12)) != 0) goto cleanup;
    if ((rc = matrix_extract_block(a, half, 0, a21)) != 0) goto cleanup;
    if ((rc = matrix_extract_block(a, half, half, a22)) != 0) goto cleanup;
    if ((rc = matrix_extract_block(b, 0, 0, b11)) != 0) goto cleanup;
    if ((rc = matrix_extract_block(b, 0, half, b12)) != 0) goto cleanup;
    if ((rc = matrix_extract_block(b, half, 0, b21)) != 0) goto cleanup;
    if ((rc = matrix_extract_block(b, half, half, b22)) != 0) goto cleanup;
    if ((rc = matrix_add(a11, a22, s1, 1)) != 0) goto cleanup;
    if ((rc = matrix_add(b11, b22, s2, 1)) != 0) goto cleanup;
    if ((rc = matmul_strassen_recursive(s1, s2, m1)) != 0) goto cleanup;
    if ((rc = matrix_add(a21, a22, s1, 1)) != 0) goto cleanup;
    if ((rc = matmul_strassen_recursive(s1, b11, m2)) != 0) goto cleanup;
    if ((rc = matrix_add(b12, b22, s2, -1)) != 0) goto cleanup;
    if ((rc = matmul_strassen_recursive(a11, s2, m3)) != 0) goto cleanup;
    if ((rc = matrix_add(b21, b11, s2, -1)) != 0) goto cleanup;
    if ((rc = matmul_strassen_recursive(a22, s2, m4)) != 0) goto cleanup;
    if ((rc = matrix_add(a11, a12, s1, 1)) != 0) goto cleanup;
    if ((rc = matmul_strassen_recursive(s1, b22, m5)) != 0) goto cleanup;
    if ((rc = matrix_add(a21, a11, s1, -1)) != 0) goto cleanup;
    if ((rc = matrix_add(b11, b12, s2, 1)) != 0) goto cleanup;
    if ((rc = matmul_strassen_recursive(s1, s2, m6)) != 0) goto cleanup;
    if ((rc = matrix_add(a12, a22, s1, -1)) != 0) goto cleanup;
    if ((rc = matrix_add(b21, b22, s2, 1)) != 0) goto cleanup;
    if ((rc = matmul_strassen_recursive(s1, s2, m7)) != 0) goto cleanup;
    if ((rc = matrix_add(m1, m4, c11, 1)) != 0) goto cleanup;
    if ((rc = matrix_add(c11, m5, c11, -1)) != 0) goto cleanup;
    if ((rc = matrix_add(c11, m7, c11, 1)) != 0) goto cleanup;
    if ((rc = matrix_add(m3, m5, c12, 1)) != 0) goto cleanup;
    if ((rc = matrix_add(m2, m4, c21, 1)) != 0) goto cleanup;
    if ((rc = matrix_add(m1, m2, c22, -1)) != 0) goto cleanup;
    if ((rc = matrix_add(c22, m3, c22, 1)) != 0) goto cleanup;
    if ((rc = matrix_add(c22, m6, c22, 1)) != 0) goto cleanup;

    matrix_store_block(c, 0, 0, c11);
    matrix_store_block(c, 0, half, c12);
    matrix_store_block(c, half, 0, c21);
    matrix_store_block(c, half, half, c22);

cleanup:
    matrix_free(a11);
    matrix_free(a12);
    matrix_free(a21);
    matrix_free(a22);
    matrix_free(b11);
    matrix_free(b12);
    matrix_free(b21);
    matrix_free(b22);
    matrix_free(s1);
    matrix_free(s2);
    matrix_free(m1);
    matrix_free(m2);
    matrix_free(m3);
    matrix_free(m4);
    matrix_free(m5);
    matrix_free(m6);
    matrix_free(m7);
    matrix_free(c11);
    matrix_free(c12);
    matrix_free(c21);
    matrix_free(c22);
    return rc;
}

int matmul_improved_v3(const Matrix *a, const Matrix *b, Matrix *c) {
    int rc;

    rc = matmul_validate_args(a, b, c);
    if (rc != 0) return rc;

    /*
     * v3: algorithm-level optimization with Strassen recursion.
     * This only activates for larger square matrices with even dimensions.
     * In all other cases it safely falls back to the blocked v2 kernel.
     */
    if (a->rows != a->cols || b->rows != b->cols || c->rows != c->cols ||
        a->rows != b->rows || a->rows != c->rows || (a->rows & 1u) != 0u)
        return matmul_improved_v2(a, b, c);

    return matmul_strassen_recursive(a, b, c);
}

int matmul_improved(const Matrix *a, const Matrix *b, Matrix *c) {
    return matmul_improved_v2(a, b, c);
}
