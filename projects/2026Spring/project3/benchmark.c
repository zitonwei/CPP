#define _POSIX_C_SOURCE 200809L

#include "matmul.h"
#include "matrix.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef USE_OPENBLAS
#include <cblas.h>
#endif

#define BENCH_ATOL 1.0e-4f
#define BENCH_RTOL 1.0e-3f

typedef int (*matmul_fn)(const Matrix *, const Matrix *, Matrix *);

static double now_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1.0e9;
}

static int safe_matrix_bytes(size_t rows, size_t cols, size_t *bytes) {
    size_t elements;

    if (rows == 0 || cols == 0) return -1;
    if (rows > SIZE_MAX / cols) return -1;
    elements = rows * cols;
    if (elements > SIZE_MAX / sizeof(float)) return -1;

    *bytes = elements * sizeof(float);
    return 0;
}

static int safe_benchmark_footprint(size_t n, size_t *bytes) {
    size_t one;

    if (safe_matrix_bytes(n, n, &one) != 0) return -1;
    if (one > SIZE_MAX / 4u) return -1;

    *bytes = one * 4u;
    return 0;
}

static double gflops_for_square(size_t n, double seconds) {
    double ops = 2.0 * (double)n * (double)n * (double)n;
    if (seconds <= 0.0) return 0.0;
    return ops / seconds / 1.0e9;
}

static void print_result_line(const char *label, size_t n, double seconds, int ok, float max_abs_diff) {
    printf(
        "%-12s n=%-6zu time=%10.6f s  gflops=%10.3f  check=%s",
        label,
        n,
        seconds,
        gflops_for_square(n, seconds),
        ok ? "PASS" : "FAIL"
    );
    if (!ok) printf("  max_abs_diff=%g", max_abs_diff);
    printf("\n");
}

static int run_timed_impl(
    const char *label, matmul_fn fn, const Matrix *a, const Matrix *b, Matrix *out, const Matrix *ref, int repeats
) {
    int attempt, ok;
    double best = 0.0;
    float max_abs_diff = 0.0f;

    for (attempt = 0; attempt < repeats; ++attempt) {
        double start = now_seconds(), elapsed;
        int rc = fn(a, b, out);

        elapsed = now_seconds() - start;
        if (rc != 0) return printf("%-12s n=%-6zu failed with rc=%d\n", label, a->rows, rc), rc;
        if (attempt == 0 || elapsed < best) best = elapsed;
    }

    ok = matrix_compare(out, ref, BENCH_ATOL, BENCH_RTOL, &max_abs_diff);
    print_result_line(label, a->rows, best, ok == 1, max_abs_diff);
    return ok == 1 ? 0 : -1;
}

#ifdef USE_OPENBLAS
static int matmul_openblas(const Matrix *a, const Matrix *b, Matrix *c) {
    if (!matrix_is_valid(a) || !matrix_is_valid(b) || !matrix_is_valid(c)) return -1;
    if (a->cols != b->rows || c->rows != a->rows || c->cols != b->cols) return -2;

    matrix_fill_zero(c);
    cblas_sgemm(
        CblasRowMajor, CblasNoTrans, CblasNoTrans,
        (int)a->rows, (int)b->cols, (int)a->cols,
        1.0f, a->data, (int)a->cols, b->data, (int)b->cols, 0.0f, c->data, (int)c->cols
    );
    return 0;
}
#endif

static int run_correctness_suite(void) {
    const size_t tests[] = {4u, 7u, 16u, 31u};
    size_t i;

    printf("Correctness checks\n");
    for (i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
        size_t n = tests[i];
        Matrix *a = matrix_create(n, n), *b = matrix_create(n, n), *plain = matrix_create(n, n);
        Matrix *v1 = matrix_create(n, n), *v2 = matrix_create(n, n), *v3 = matrix_create(n, n);

        if (a == NULL || b == NULL || plain == NULL || v1 == NULL || v2 == NULL || v3 == NULL) {
            printf("  failed to allocate correctness test matrices for n=%zu\n", n);
            matrix_free(a);
            matrix_free(b);
            matrix_free(plain);
            matrix_free(v1);
            matrix_free(v2);
            matrix_free(v3);
            return -1;
        }

        matrix_fill_random(a, 100u + (unsigned int)n);
        matrix_fill_random(b, 200u + (unsigned int)n);

        if (matmul_plain(a, b, plain) != 0 ||
            matmul_improved_v1(a, b, v1) != 0 ||
            matmul_improved_v2(a, b, v2) != 0 ||
            matmul_improved_v3(a, b, v3) != 0) {
            printf("  multiplication failed during correctness test for n=%zu\n", n);
            matrix_free(a);
            matrix_free(b);
            matrix_free(plain);
            matrix_free(v1);
            matrix_free(v2);
            matrix_free(v3);
            return -1;
        }

        {
            float max_abs_diff = 0.0f;
            int ok = matrix_compare(plain, v1, BENCH_ATOL, BENCH_RTOL, &max_abs_diff);
            printf("  plain vs v1      n=%-4zu : %s", n, ok == 1 ? "PASS" : "FAIL");
            if (ok != 1) printf("  max_abs_diff=%g", max_abs_diff);
            printf("\n");
        }

        {
            float max_abs_diff = 0.0f;
            int ok = matrix_compare(plain, v2, BENCH_ATOL, BENCH_RTOL, &max_abs_diff);
            printf("  plain vs v2      n=%-4zu : %s", n, ok == 1 ? "PASS" : "FAIL");
            if (ok != 1) printf("  max_abs_diff=%g", max_abs_diff);
            printf("\n");
        }

        {
            float max_abs_diff = 0.0f;
            int ok = matrix_compare(plain, v3, BENCH_ATOL, BENCH_RTOL, &max_abs_diff);
            printf("  plain vs v3      n=%-4zu : %s", n, ok == 1 ? "PASS" : "FAIL");
            if (ok != 1) printf("  max_abs_diff=%g", max_abs_diff);
            printf("\n");
        }

#ifdef USE_OPENBLAS
        {
            Matrix *blas = matrix_create(n, n);
            float max_abs_diff = 0.0f;
            int ok;

            if (blas == NULL) {
                printf("  failed to allocate OpenBLAS reference for n=%zu\n", n);
                matrix_free(a);
                matrix_free(b);
                matrix_free(plain);
                matrix_free(v1);
                matrix_free(v2);
                matrix_free(v3);
                return -1;
            }

            if (matmul_openblas(a, b, blas) != 0) {
                printf("  OpenBLAS call failed for n=%zu\n", n);
                matrix_free(blas);
                matrix_free(a);
                matrix_free(b);
                matrix_free(plain);
                matrix_free(v1);
                matrix_free(v2);
                matrix_free(v3);
                return -1;
            }

            ok = matrix_compare(v3, blas, BENCH_ATOL, BENCH_RTOL, &max_abs_diff);
            printf("  v3 vs OpenBLAS   n=%-4zu : %s", n, ok == 1 ? "PASS" : "FAIL");
            if (ok != 1) printf("  max_abs_diff=%g", max_abs_diff);
            printf("\n");
            matrix_free(blas);
        }
#else
        printf("  v3 vs OpenBLAS   n=%-4zu : skipped (OpenBLAS not enabled)\n", n);
#endif

        matrix_free(a);
        matrix_free(b);
        matrix_free(plain);
        matrix_free(v1);
        matrix_free(v2);
        matrix_free(v3);
    }

    printf("\n");
    return 0;
}

static void run_benchmark_for_size(size_t n) {
    Matrix *a, *b, *plain_ref, *v1_out, *v2_out, *v3_out;
#ifdef USE_OPENBLAS
    Matrix *blas_out;
#endif
    size_t bytes_needed = 0;
    int repeats;

    printf("Benchmark for n=%zu\n", n);
    if (safe_benchmark_footprint(n, &bytes_needed) != 0) {
        printf("  skipped: matrix size overflow in memory calculation\n\n");
        return;
    }

    printf("  estimated working set for four n x n matrices: %.2f MiB\n", bytes_needed / (1024.0 * 1024.0));

    a = matrix_create(n, n);
    b = matrix_create(n, n);
    plain_ref = matrix_create(n, n);
    v1_out = matrix_create(n, n);
    v2_out = matrix_create(n, n);
    v3_out = matrix_create(n, n);
#ifdef USE_OPENBLAS
    blas_out = matrix_create(n, n);
#endif

    if (a == NULL || b == NULL || plain_ref == NULL || v1_out == NULL || v2_out == NULL || v3_out == NULL
#ifdef USE_OPENBLAS
        || blas_out == NULL
#endif
    ) {
        printf("  skipped: allocation failed\n\n");
        matrix_free(a);
        matrix_free(b);
        matrix_free(plain_ref);
        matrix_free(v1_out);
        matrix_free(v2_out);
        matrix_free(v3_out);
#ifdef USE_OPENBLAS
        matrix_free(blas_out);
#endif
        return;
    }

    matrix_fill_random(a, 1000u + (unsigned int)n);
    matrix_fill_random(b, 2000u + (unsigned int)n);
    repeats = n <= 128u ? 5 : (n <= 1024u ? 2 : 1);

    if (run_timed_impl("plain", matmul_plain, a, b, plain_ref, plain_ref, repeats) != 0) {
        printf("  baseline failed, stopping this size\n\n");
        goto cleanup;
    }

    if (run_timed_impl("v1", matmul_improved_v1, a, b, v1_out, plain_ref, repeats) != 0) printf("  v1 implementation failed correctness check\n");
    if (run_timed_impl("v2", matmul_improved_v2, a, b, v2_out, plain_ref, repeats) != 0) printf("  v2 implementation failed correctness check\n");
    if (run_timed_impl("v3", matmul_improved_v3, a, b, v3_out, plain_ref, repeats) != 0) printf("  v3 implementation failed correctness check\n");

#ifdef USE_OPENBLAS
    if (run_timed_impl("openblas", matmul_openblas, a, b, blas_out, plain_ref, repeats) != 0) printf("  OpenBLAS result failed correctness check\n");
#else
    printf("openblas     n=%-6zu skipped: OpenBLAS not enabled at compile time\n", n);
#endif

    printf("\n");

cleanup:
    matrix_free(a);
    matrix_free(b);
    matrix_free(plain_ref);
    matrix_free(v1_out);
    matrix_free(v2_out);
    matrix_free(v3_out);
#ifdef USE_OPENBLAS
    matrix_free(blas_out);
#endif
}

int main(void) {
    const size_t sizes[] = {16u, 128u, 1024u, 8192u, 65536u};
    size_t i;

#ifdef _OPENMP
    printf("OpenMP: enabled\n");
#else
    printf("OpenMP: disabled\n");
#endif

#ifdef USE_OPENBLAS
    printf("OpenBLAS: enabled\n\n");
#else
    printf("OpenBLAS: disabled\n\n");
#endif

    if (run_correctness_suite() != 0) return EXIT_FAILURE;
    for (i = 0; i < sizeof(sizes) / sizeof(sizes[0]); ++i) run_benchmark_for_size(sizes[i]);
    return EXIT_SUCCESS;
}
