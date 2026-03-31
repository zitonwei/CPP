#define _POSIX_C_SOURCE 200809L

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

enum {
    WARMUP_RUNS = 3,
    MEASURE_RUNS = 5
};

static const size_t SIZES[] = {
    1000u,
    10000u,
    100000u,
    1000000u,
    5000000u,
    10000000u
};

static volatile long long sink_ll = 0;
static volatile double sink_double = 0.0;

static int random_int_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

static double random_unit_value(void) {
    return -1.0 + 2.0 * ((double) rand() / (double) RAND_MAX);
}

static void fill_int_array(int *arr, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        arr[i] = random_int_range(-100, 100);
    }
}

static void fill_short_array(short *arr, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        arr[i] = (short) random_int_range(-100, 100);
    }
}

static void fill_schar_array(signed char *arr, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        arr[i] = (signed char) random_int_range(-100, 100);
    }
}

static void fill_float_array(float *arr, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        arr[i] = (float) random_unit_value();
    }
}

static void fill_double_array(double *arr, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        arr[i] = random_unit_value();
    }
}

static long long dot_int(const int *a, const int *b, size_t n) {
    long long sum = 0;
    for (size_t i = 0; i < n; ++i) {
        sum += (long long) a[i] * (long long) b[i];
    }
    return sum;
}

static long long dot_short(const short *a, const short *b, size_t n) {
    long long sum = 0;
    for (size_t i = 0; i < n; ++i) {
        sum += (long long) a[i] * (long long) b[i];
    }
    return sum;
}

static long long dot_schar(const signed char *a, const signed char *b, size_t n) {
    long long sum = 0;
    for (size_t i = 0; i < n; ++i) {
        sum += (long long) a[i] * (long long) b[i];
    }
    return sum;
}

static double dot_float(const float *a, const float *b, size_t n) {
    double sum = 0.0;
    for (size_t i = 0; i < n; ++i) {
        sum += (double) a[i] * (double) b[i];
    }
    return sum;
}

static double dot_double(const double *a, const double *b, size_t n) {
    double sum = 0.0;
    for (size_t i = 0; i < n; ++i) {
        sum += a[i] * b[i];
    }
    return sum;
}

static long long elapsed_ns(const struct timespec *start, const struct timespec *end) {
    return (long long) (end->tv_sec - start->tv_sec) * 1000000000LL
        + (long long) (end->tv_nsec - start->tv_nsec);
}

static void benchmark_int(size_t n) {
    int *a = malloc(n * sizeof(*a));
    int *b = malloc(n * sizeof(*b));
    if (a == NULL || b == NULL) {
        fprintf(stderr, "Allocation failed for int, n=%zu\n", n);
        free(a);
        free(b);
        exit(EXIT_FAILURE);
    }

    fill_int_array(a, n);
    fill_int_array(b, n);

    for (int i = 0; i < WARMUP_RUNS; ++i) {
        sink_ll = dot_int(a, b, n);
    }

    struct timespec start;
    struct timespec end;
    long long total_ns = 0;
    long long result = 0;

    for (int i = 0; i < MEASURE_RUNS; ++i) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        result = dot_int(a, b, n);
        clock_gettime(CLOCK_MONOTONIC, &end);
        sink_ll = result;
        total_ns += elapsed_ns(&start, &end);
    }

    printf("C,int,%zu,%lld,%lld\n", n, total_ns / MEASURE_RUNS, result);
    free(a);
    free(b);
}

static void benchmark_short(size_t n) {
    short *a = malloc(n * sizeof(*a));
    short *b = malloc(n * sizeof(*b));
    if (a == NULL || b == NULL) {
        fprintf(stderr, "Allocation failed for short, n=%zu\n", n);
        free(a);
        free(b);
        exit(EXIT_FAILURE);
    }

    fill_short_array(a, n);
    fill_short_array(b, n);

    for (int i = 0; i < WARMUP_RUNS; ++i) {
        sink_ll = dot_short(a, b, n);
    }

    struct timespec start;
    struct timespec end;
    long long total_ns = 0;
    long long result = 0;

    for (int i = 0; i < MEASURE_RUNS; ++i) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        result = dot_short(a, b, n);
        clock_gettime(CLOCK_MONOTONIC, &end);
        sink_ll = result;
        total_ns += elapsed_ns(&start, &end);
    }

    printf("C,short,%zu,%lld,%lld\n", n, total_ns / MEASURE_RUNS, result);
    free(a);
    free(b);
}

static void benchmark_schar(size_t n) {
    signed char *a = malloc(n * sizeof(*a));
    signed char *b = malloc(n * sizeof(*b));
    if (a == NULL || b == NULL) {
        fprintf(stderr, "Allocation failed for signed char, n=%zu\n", n);
        free(a);
        free(b);
        exit(EXIT_FAILURE);
    }

    fill_schar_array(a, n);
    fill_schar_array(b, n);

    for (int i = 0; i < WARMUP_RUNS; ++i) {
        sink_ll = dot_schar(a, b, n);
    }

    struct timespec start;
    struct timespec end;
    long long total_ns = 0;
    long long result = 0;

    for (int i = 0; i < MEASURE_RUNS; ++i) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        result = dot_schar(a, b, n);
        clock_gettime(CLOCK_MONOTONIC, &end);
        sink_ll = result;
        total_ns += elapsed_ns(&start, &end);
    }

    printf("C,signed char,%zu,%lld,%lld\n", n, total_ns / MEASURE_RUNS, result);
    free(a);
    free(b);
}

static void benchmark_float(size_t n) {
    float *a = malloc(n * sizeof(*a));
    float *b = malloc(n * sizeof(*b));
    if (a == NULL || b == NULL) {
        fprintf(stderr, "Allocation failed for float, n=%zu\n", n);
        free(a);
        free(b);
        exit(EXIT_FAILURE);
    }

    fill_float_array(a, n);
    fill_float_array(b, n);

    for (int i = 0; i < WARMUP_RUNS; ++i) {
        sink_double = dot_float(a, b, n);
    }

    struct timespec start;
    struct timespec end;
    long long total_ns = 0;
    double result = 0.0;

    for (int i = 0; i < MEASURE_RUNS; ++i) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        result = dot_float(a, b, n);
        clock_gettime(CLOCK_MONOTONIC, &end);
        sink_double = result;
        total_ns += elapsed_ns(&start, &end);
    }

    printf("C,float,%zu,%lld,%.10f\n", n, total_ns / MEASURE_RUNS, result);
    free(a);
    free(b);
}

static void benchmark_double(size_t n) {
    double *a = malloc(n * sizeof(*a));
    double *b = malloc(n * sizeof(*b));
    if (a == NULL || b == NULL) {
        fprintf(stderr, "Allocation failed for double, n=%zu\n", n);
        free(a);
        free(b);
        exit(EXIT_FAILURE);
    }

    fill_double_array(a, n);
    fill_double_array(b, n);

    for (int i = 0; i < WARMUP_RUNS; ++i) {
        sink_double = dot_double(a, b, n);
    }

    struct timespec start;
    struct timespec end;
    long long total_ns = 0;
    double result = 0.0;

    for (int i = 0; i < MEASURE_RUNS; ++i) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        result = dot_double(a, b, n);
        clock_gettime(CLOCK_MONOTONIC, &end);
        sink_double = result;
        total_ns += elapsed_ns(&start, &end);
    }

    printf("C,double,%zu,%lld,%.10f\n", n, total_ns / MEASURE_RUNS, result);
    free(a);
    free(b);
}

int main(void) {
    srand(42);

    puts("language,type,n,avg_ns,result");
    for (size_t i = 0; i < sizeof(SIZES) / sizeof(SIZES[0]); ++i) {
        size_t n = SIZES[i];
        benchmark_int(n);
        benchmark_short(n);
        benchmark_schar(n);
        benchmark_float(n);
        benchmark_double(n);
    }

    return 0;
}
