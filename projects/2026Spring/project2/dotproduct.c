#define _POSIX_C_SOURCE 200809L

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

enum {
    WARMUP_RUNS = 5,
    MEASURE_RUNS = 10
};

typedef enum {
    MODE_SAFE = 0,
    MODE_OVERFLOW = 1
} FillMode;

static const size_t SIZES[] = {1000u, 10000u, 100000u, 1000000u, 5000000u, 10000000u};

static volatile int sink_int;
static volatile short sink_short;
static volatile signed char sink_schar;
static volatile float sink_float;
static volatile double sink_double;

static FILE *csv_file;

static const char *mode_name(FillMode mode) {
    return mode == MODE_SAFE ? "safe" : "overflow";
}

static long long elapsed_ns(const struct timespec *start, const struct timespec *end) {
    return (long long) (end->tv_sec - start->tv_sec) * 1000000000LL
        + (long long) (end->tv_nsec - start->tv_nsec);
}

static int random_int_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

static int random_sign(void) {
    return rand() % 2 == 0 ? 1 : -1;
}

static float random_float_range(float min, float max) {
    return min + (max - min) * ((float) rand() / (float) RAND_MAX);
}

static double random_double_range(double min, double max) {
    return min + (max - min) * ((double) rand() / (double) RAND_MAX);
}

static int safe_int_bound(long double max_value, size_t n) {
    long double bound;
    if (n == 0 || max_value <= 0.0L) {
        return 1;
    }
    bound = sqrtl(max_value / (long double) n);
    if (bound < 1.0L) {
        return 1;
    }
    if (bound > (long double) INT_MAX) {
        return INT_MAX;
    }
    return (int) bound;
}

static void fill_int_array(int *arr, size_t n, FillMode mode) {
    int min_value = safe_int_bound((long double) INT_MAX, n);
    for (size_t i = 0; i < n; ++i) {
        if (mode == MODE_SAFE) {
            arr[i] = random_int_range(-min_value, min_value);
        } else {
            arr[i] = random_sign() * random_int_range(min_value, INT_MAX);
        }
    }
}

static void fill_short_array(short *arr, size_t n, FillMode mode) {
    int min_value = safe_int_bound((long double) SHRT_MAX, n);
    for (size_t i = 0; i < n; ++i) {
        if (mode == MODE_SAFE) {
            arr[i] = (short) random_int_range(-min_value, min_value);
        } else {
            arr[i] = (short) (random_sign() * random_int_range(min_value, SHRT_MAX));
        }
    }
}

static void fill_schar_array(signed char *arr, size_t n, FillMode mode) {
    int min_value = safe_int_bound((long double) SCHAR_MAX, n);
    for (size_t i = 0; i < n; ++i) {
        if (mode == MODE_SAFE) {
            arr[i] = (signed char) random_int_range(-min_value, min_value);
        } else {
            arr[i] = (signed char) (random_sign() * random_int_range(min_value, SCHAR_MAX));
        }
    }
}

static void fill_float_array(float *arr, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        arr[i] = random_float_range(-1.0f, 1.0f);
    }
}

static void fill_double_array(double *arr, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        arr[i] = random_double_range(-1.0, 1.0);
    }
}

static int dot_int(const int *a, const int *b, size_t n) {
    int sum = 0;
    for (size_t i = 0; i < n; ++i) {
        sum += a[i] * b[i];
    }
    return sum;
}

static short dot_short(const short *a, const short *b, size_t n) {
    short sum = 0;
    for (size_t i = 0; i < n; ++i) {
        sum += a[i] * b[i];
    }
    return sum;
}

static signed char dot_schar(const signed char *a, const signed char *b, size_t n) {
    signed char sum = 0;
    for (size_t i = 0; i < n; ++i) {
        sum += a[i] * b[i];
    }
    return sum;
}

static float dot_float(const float *a, const float *b, size_t n) {
    float sum = 0.0f;
    for (size_t i = 0; i < n; ++i) {
        sum += a[i] * b[i];
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

static void print_header(void) {
    printf("%-8s %-10s %-12s %10s %16s %16s %20s\n",
        "language", "mode", "type", "n", "warmup_avg_ns", "measure_avg_ns", "result");
    fprintf(csv_file, "language,mode,type,n,warmup_avg_ns,measure_avg_ns,result\n");
}

static void print_int_row(const char *type_name, FillMode mode, size_t n,
    long long warmup_avg_ns, long long measure_avg_ns, int result) {
    printf("%-8s %-10s %-12s %10zu %16lld %16lld %20d\n",
        "C", mode_name(mode), type_name, n, warmup_avg_ns, measure_avg_ns, result);
    fprintf(csv_file, "C,%s,%s,%zu,%lld,%lld,%d\n",
        mode_name(mode), type_name, n, warmup_avg_ns, measure_avg_ns, result);
}

static void print_short_row(const char *type_name, FillMode mode, size_t n,
    long long warmup_avg_ns, long long measure_avg_ns, short result) {
    printf("%-8s %-10s %-12s %10zu %16lld %16lld %20hd\n",
        "C", mode_name(mode), type_name, n, warmup_avg_ns, measure_avg_ns, result);
    fprintf(csv_file, "C,%s,%s,%zu,%lld,%lld,%hd\n",
        mode_name(mode), type_name, n, warmup_avg_ns, measure_avg_ns, result);
}

static void print_schar_row(const char *type_name, FillMode mode, size_t n,
    long long warmup_avg_ns, long long measure_avg_ns, signed char result) {
    printf("%-8s %-10s %-12s %10zu %16lld %16lld %20hhd\n",
        "C", mode_name(mode), type_name, n, warmup_avg_ns, measure_avg_ns, result);
    fprintf(csv_file, "C,%s,%s,%zu,%lld,%lld,%hhd\n",
        mode_name(mode), type_name, n, warmup_avg_ns, measure_avg_ns, result);
}

static void print_fp_row(const char *type_name, size_t n,
    long long warmup_avg_ns, long long measure_avg_ns, double result) {
    printf("%-8s %-10s %-12s %10zu %16lld %16lld %20.10e\n",
        "C", "safe", type_name, n, warmup_avg_ns, measure_avg_ns, result);
    fprintf(csv_file, "C,safe,%s,%zu,%lld,%lld,%.10e\n",
        type_name, n, warmup_avg_ns, measure_avg_ns, result);
}

static void benchmark_int(size_t n) {
    int *a_safe = malloc(n * sizeof(*a_safe));
    int *b_safe = malloc(n * sizeof(*b_safe));
    int *a_overflow = malloc(n * sizeof(*a_overflow));
    int *b_overflow = malloc(n * sizeof(*b_overflow));
    struct timespec start;
    struct timespec end;
    long long warmup_total[2] = {0, 0};
    long long measure_total[2] = {0, 0};
    int warmup_count[2] = {0, 0};
    int measure_count[2] = {0, 0};
    int result[2] = {0, 0};

    if (a_safe == NULL || b_safe == NULL || a_overflow == NULL || b_overflow == NULL) {
        fprintf(stderr, "Allocation failed for int, n=%zu\n", n);
        free(a_safe);
        free(b_safe);
        free(a_overflow);
        free(b_overflow);
        exit(EXIT_FAILURE);
    }

    fill_int_array(a_safe, n, MODE_SAFE);
    fill_int_array(b_safe, n, MODE_SAFE);
    fill_int_array(a_overflow, n, MODE_OVERFLOW);
    fill_int_array(b_overflow, n, MODE_OVERFLOW);

    while (warmup_count[MODE_SAFE] < WARMUP_RUNS || warmup_count[MODE_OVERFLOW] < WARMUP_RUNS) {
        FillMode current = (FillMode) (rand() % 2);
        if (warmup_count[current] >= WARMUP_RUNS) {
            current = current == MODE_SAFE ? MODE_OVERFLOW : MODE_SAFE;
        }
        clock_gettime(CLOCK_MONOTONIC, &start);
        result[current] = current == MODE_SAFE
            ? dot_int(a_safe, b_safe, n)
            : dot_int(a_overflow, b_overflow, n);
        clock_gettime(CLOCK_MONOTONIC, &end);
        sink_int = result[current];
        warmup_total[current] += elapsed_ns(&start, &end);
        ++warmup_count[current];
    }

    while (measure_count[MODE_SAFE] < MEASURE_RUNS || measure_count[MODE_OVERFLOW] < MEASURE_RUNS) {
        FillMode current = (FillMode) (rand() % 2);
        if (measure_count[current] >= MEASURE_RUNS) {
            current = current == MODE_SAFE ? MODE_OVERFLOW : MODE_SAFE;
        }
        clock_gettime(CLOCK_MONOTONIC, &start);
        result[current] = current == MODE_SAFE
            ? dot_int(a_safe, b_safe, n)
            : dot_int(a_overflow, b_overflow, n);
        clock_gettime(CLOCK_MONOTONIC, &end);
        sink_int = result[current];
        measure_total[current] += elapsed_ns(&start, &end);
        ++measure_count[current];
    }

    print_int_row("int", MODE_SAFE, n, warmup_total[MODE_SAFE] / WARMUP_RUNS,
        measure_total[MODE_SAFE] / MEASURE_RUNS, result[MODE_SAFE]);
    print_int_row("int", MODE_OVERFLOW, n, warmup_total[MODE_OVERFLOW] / WARMUP_RUNS,
        measure_total[MODE_OVERFLOW] / MEASURE_RUNS, result[MODE_OVERFLOW]);

    free(a_safe);
    free(b_safe);
    free(a_overflow);
    free(b_overflow);
}

static void benchmark_short(size_t n) {
    short *a_safe = malloc(n * sizeof(*a_safe));
    short *b_safe = malloc(n * sizeof(*b_safe));
    short *a_overflow = malloc(n * sizeof(*a_overflow));
    short *b_overflow = malloc(n * sizeof(*b_overflow));
    struct timespec start;
    struct timespec end;
    long long warmup_total[2] = {0, 0};
    long long measure_total[2] = {0, 0};
    int warmup_count[2] = {0, 0};
    int measure_count[2] = {0, 0};
    short result[2] = {0, 0};

    if (a_safe == NULL || b_safe == NULL || a_overflow == NULL || b_overflow == NULL) {
        fprintf(stderr, "Allocation failed for short, n=%zu\n", n);
        free(a_safe);
        free(b_safe);
        free(a_overflow);
        free(b_overflow);
        exit(EXIT_FAILURE);
    }

    fill_short_array(a_safe, n, MODE_SAFE);
    fill_short_array(b_safe, n, MODE_SAFE);
    fill_short_array(a_overflow, n, MODE_OVERFLOW);
    fill_short_array(b_overflow, n, MODE_OVERFLOW);

    while (warmup_count[MODE_SAFE] < WARMUP_RUNS || warmup_count[MODE_OVERFLOW] < WARMUP_RUNS) {
        FillMode current = (FillMode) (rand() % 2);
        if (warmup_count[current] >= WARMUP_RUNS) {
            current = current == MODE_SAFE ? MODE_OVERFLOW : MODE_SAFE;
        }
        clock_gettime(CLOCK_MONOTONIC, &start);
        result[current] = current == MODE_SAFE
            ? dot_short(a_safe, b_safe, n)
            : dot_short(a_overflow, b_overflow, n);
        clock_gettime(CLOCK_MONOTONIC, &end);
        sink_short = result[current];
        warmup_total[current] += elapsed_ns(&start, &end);
        ++warmup_count[current];
    }

    while (measure_count[MODE_SAFE] < MEASURE_RUNS || measure_count[MODE_OVERFLOW] < MEASURE_RUNS) {
        FillMode current = (FillMode) (rand() % 2);
        if (measure_count[current] >= MEASURE_RUNS) {
            current = current == MODE_SAFE ? MODE_OVERFLOW : MODE_SAFE;
        }
        clock_gettime(CLOCK_MONOTONIC, &start);
        result[current] = current == MODE_SAFE
            ? dot_short(a_safe, b_safe, n)
            : dot_short(a_overflow, b_overflow, n);
        clock_gettime(CLOCK_MONOTONIC, &end);
        sink_short = result[current];
        measure_total[current] += elapsed_ns(&start, &end);
        ++measure_count[current];
    }

    print_short_row("short", MODE_SAFE, n, warmup_total[MODE_SAFE] / WARMUP_RUNS,
        measure_total[MODE_SAFE] / MEASURE_RUNS, result[MODE_SAFE]);
    print_short_row("short", MODE_OVERFLOW, n, warmup_total[MODE_OVERFLOW] / WARMUP_RUNS,
        measure_total[MODE_OVERFLOW] / MEASURE_RUNS, result[MODE_OVERFLOW]);

    free(a_safe);
    free(b_safe);
    free(a_overflow);
    free(b_overflow);
}

static void benchmark_schar(size_t n) {
    signed char *a_safe = malloc(n * sizeof(*a_safe));
    signed char *b_safe = malloc(n * sizeof(*b_safe));
    signed char *a_overflow = malloc(n * sizeof(*a_overflow));
    signed char *b_overflow = malloc(n * sizeof(*b_overflow));
    struct timespec start;
    struct timespec end;
    long long warmup_total[2] = {0, 0};
    long long measure_total[2] = {0, 0};
    int warmup_count[2] = {0, 0};
    int measure_count[2] = {0, 0};
    signed char result[2] = {0, 0};

    if (a_safe == NULL || b_safe == NULL || a_overflow == NULL || b_overflow == NULL) {
        fprintf(stderr, "Allocation failed for signed char, n=%zu\n", n);
        free(a_safe);
        free(b_safe);
        free(a_overflow);
        free(b_overflow);
        exit(EXIT_FAILURE);
    }

    fill_schar_array(a_safe, n, MODE_SAFE);
    fill_schar_array(b_safe, n, MODE_SAFE);
    fill_schar_array(a_overflow, n, MODE_OVERFLOW);
    fill_schar_array(b_overflow, n, MODE_OVERFLOW);

    while (warmup_count[MODE_SAFE] < WARMUP_RUNS || warmup_count[MODE_OVERFLOW] < WARMUP_RUNS) {
        FillMode current = (FillMode) (rand() % 2);
        if (warmup_count[current] >= WARMUP_RUNS) {
            current = current == MODE_SAFE ? MODE_OVERFLOW : MODE_SAFE;
        }
        clock_gettime(CLOCK_MONOTONIC, &start);
        result[current] = current == MODE_SAFE
            ? dot_schar(a_safe, b_safe, n)
            : dot_schar(a_overflow, b_overflow, n);
        clock_gettime(CLOCK_MONOTONIC, &end);
        sink_schar = result[current];
        warmup_total[current] += elapsed_ns(&start, &end);
        ++warmup_count[current];
    }

    while (measure_count[MODE_SAFE] < MEASURE_RUNS || measure_count[MODE_OVERFLOW] < MEASURE_RUNS) {
        FillMode current = (FillMode) (rand() % 2);
        if (measure_count[current] >= MEASURE_RUNS) {
            current = current == MODE_SAFE ? MODE_OVERFLOW : MODE_SAFE;
        }
        clock_gettime(CLOCK_MONOTONIC, &start);
        result[current] = current == MODE_SAFE
            ? dot_schar(a_safe, b_safe, n)
            : dot_schar(a_overflow, b_overflow, n);
        clock_gettime(CLOCK_MONOTONIC, &end);
        sink_schar = result[current];
        measure_total[current] += elapsed_ns(&start, &end);
        ++measure_count[current];
    }

    print_schar_row("signed char", MODE_SAFE, n, warmup_total[MODE_SAFE] / WARMUP_RUNS,
        measure_total[MODE_SAFE] / MEASURE_RUNS, result[MODE_SAFE]);
    print_schar_row("signed char", MODE_OVERFLOW, n, warmup_total[MODE_OVERFLOW] / WARMUP_RUNS,
        measure_total[MODE_OVERFLOW] / MEASURE_RUNS, result[MODE_OVERFLOW]);

    free(a_safe);
    free(b_safe);
    free(a_overflow);
    free(b_overflow);
}

static void benchmark_float(size_t n) {
    float *a = malloc(n * sizeof(*a));
    float *b = malloc(n * sizeof(*b));
    struct timespec start;
    struct timespec end;
    long long warmup_total = 0;
    long long measure_total = 0;
    float result = 0.0f;

    if (a == NULL || b == NULL) {
        fprintf(stderr, "Allocation failed for float, n=%zu\n", n);
        free(a);
        free(b);
        exit(EXIT_FAILURE);
    }

    fill_float_array(a, n);
    fill_float_array(b, n);

    for (int i = 0; i < WARMUP_RUNS; ++i) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        result = dot_float(a, b, n);
        clock_gettime(CLOCK_MONOTONIC, &end);
        sink_float = result;
        warmup_total += elapsed_ns(&start, &end);
    }

    for (int i = 0; i < MEASURE_RUNS; ++i) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        result = dot_float(a, b, n);
        clock_gettime(CLOCK_MONOTONIC, &end);
        sink_float = result;
        measure_total += elapsed_ns(&start, &end);
    }

    print_fp_row("float", n, warmup_total / WARMUP_RUNS, measure_total / MEASURE_RUNS, result);
    free(a);
    free(b);
}

static void benchmark_double(size_t n) {
    double *a = malloc(n * sizeof(*a));
    double *b = malloc(n * sizeof(*b));
    struct timespec start;
    struct timespec end;
    long long warmup_total = 0;
    long long measure_total = 0;
    double result = 0.0;

    if (a == NULL || b == NULL) {
        fprintf(stderr, "Allocation failed for double, n=%zu\n", n);
        free(a);
        free(b);
        exit(EXIT_FAILURE);
    }

    fill_double_array(a, n);
    fill_double_array(b, n);

    for (int i = 0; i < WARMUP_RUNS; ++i) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        result = dot_double(a, b, n);
        clock_gettime(CLOCK_MONOTONIC, &end);
        sink_double = result;
        warmup_total += elapsed_ns(&start, &end);
    }

    for (int i = 0; i < MEASURE_RUNS; ++i) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        result = dot_double(a, b, n);
        clock_gettime(CLOCK_MONOTONIC, &end);
        sink_double = result;
        measure_total += elapsed_ns(&start, &end);
    }

    print_fp_row("double", n, warmup_total / WARMUP_RUNS, measure_total / MEASURE_RUNS, result);
    free(a);
    free(b);
}

int main(void) {
    srand(42);
    csv_file = fopen("c_results.csv", "w");
    if (csv_file == NULL) {
        fprintf(stderr, "Failed to open c_results.csv for writing\n");
        return EXIT_FAILURE;
    }

    print_header();
    for (size_t i = 0; i < sizeof(SIZES) / sizeof(SIZES[0]); ++i) {
        size_t n = SIZES[i];
        benchmark_int(n);
        benchmark_short(n);
        benchmark_schar(n);
        benchmark_float(n);
        benchmark_double(n);
    }

    fclose(csv_file);
    return 0;
}
