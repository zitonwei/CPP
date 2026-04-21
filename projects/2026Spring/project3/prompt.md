# Project 3 Prompt for Codex

You are helping me complete a university C programming project.

Your task is to generate a **clean, correct, benchmarkable, and reasonably optimized** C project for matrix multiplication, following the requirements below.

---

## 1. Project background

The assignment is about implementing **matrix multiplication in C** and improving its speed as much as reasonably possible.

The project should include:

1. A straightforward baseline implementation: `matmul_plain()`
2. An optimized implementation: `matmul_improved()`
3. A benchmarking program that compares:
   - `matmul_plain()`
   - `matmul_improved()`
   - OpenBLAS (`cblas_sgemm`) when available

The code should be written in a professional style, with proper parameter checking, good structure, and no memory leaks.

---

## 2. Assignment requirements that must be followed

Please strictly follow these requirements:

- Use only `float`
- Implement matrix multiplication in C
- Implement:
  - `matmul_plain()` using ordinary nested loops as a baseline
  - `matmul_improved()` using optimization techniques such as:
    - loop reordering
    - cache blocking / tiling
    - transpose-based optimization if useful
    - OpenMP if available
    - SIMD if available, but only when practical
- Benchmark matrix sizes:
  - 16 x 16
  - 128 x 128
  - 1024 x 1024
  - 8192 x 8192
  - 65536 x 65536
- Compare performance and correctness against OpenBLAS
- The results of my implementation should match OpenBLAS exactly or be very close numerically
- The code should compile with optimization flags such as `-O3`
- The implementation should be clean, readable, and safe
- Parameters must be checked before use
- There must be no memory leaks

---

## 3. Important realism constraints

Be practical and honest:

- A dense `65536 x 65536` float matrix requires enormous memory, so the code must **gracefully detect allocation failure or impossible memory usage** and skip the benchmark with a clear message instead of crashing
- Do not fabricate benchmark data
- Do not fabricate OpenBLAS results
- If OpenBLAS is unavailable, the benchmark should still compile and run while clearly stating that OpenBLAS comparison is skipped
- If OpenMP is unavailable, the project should still compile and run
- If SIMD is used, it must be guarded by compile-time conditions
- Avoid overengineering the project with too many platform-specific branches
- The project should be suitable for a student report and oral explanation

---

## 4. What files to generate

Generate the full contents of these files:

1. `matrix.h`
2. `matrix.c`
3. `matmul.h`
4. `matmul.c`
5. `benchmark.c`
6. `Makefile`
7. `README.md`

You may also add:
- `utils.h`
- `utils.c`

if they improve clarity.

Do **not** merge everything into one giant file.

---

## 5. Matrix abstraction requirements

Use a matrix structure like this:

```c
typedef struct {
    size_t rows;
    size_t cols;
    float *data;
} Matrix;
```

Implement helper functions such as:

- `Matrix *matrix_create(size_t rows, size_t cols);`
- `void matrix_free(Matrix *m);`
- `int matrix_fill_random(Matrix *m, unsigned int seed);`
- `void matrix_fill_zero(Matrix *m);`
- `int matrix_compare(const Matrix *a, const Matrix *b, float atol, float rtol, float *max_abs_diff);`
- `void matrix_print_partial(const Matrix *m, size_t max_rows, size_t max_cols);`

Requirements:
- use contiguous memory
- check for invalid dimensions
- check for overflow in allocation-size calculation
- return sensible error codes or NULL on failure
- keep the API simple and easy to understand

---

## 6. matmul_plain requirements

Implement:

```c
int matmul_plain(const Matrix *a, const Matrix *b, Matrix *c);
```

Requirements:
- verify that `a`, `b`, and `c` are valid
- verify dimension compatibility
- compute `C = A x B`
- define clear behavior: `c` should be overwritten from scratch
- keep this implementation straightforward and readable
- this version serves as the baseline benchmark

---

## 7. matmul_improved requirements

Implement:

```c
int matmul_improved(const Matrix *a, const Matrix *b, Matrix *c);
```

Optimization priorities:

1. correct result
2. better cache behavior
3. good readability
4. optional OpenMP parallelization
5. optional SIMD intrinsics

A good design would be one of the following:
- transpose matrix B and improve locality
- use block multiplication / tiling
- combine both if still readable

Please choose an optimization strategy that is **effective but still understandable**.

Add comments explaining:
- why this version is faster than the baseline
- what optimization ideas are used
- any tradeoffs made

Avoid making the code unreadable.

---

## 8. OpenBLAS comparison requirements

In `benchmark.c`, add optional support for OpenBLAS:

- when OpenBLAS is available, compare against `cblas_sgemm`
- when OpenBLAS is not available, print a message and skip it
- compilation should still succeed without OpenBLAS installed

Use compile-time guards such as:
- `USE_OPENBLAS=1` in `Makefile`
- conditional includes and conditional compilation in C source

---

## 9. Benchmark requirements

Write a benchmark driver that does the following:

### 9.1 Correctness testing
Before benchmarking, test correctness on small matrices:
- small random matrices
- compare `matmul_plain`, `matmul_improved`, and OpenBLAS when available

### 9.2 Performance testing
Benchmark these sizes:
- 16
- 128
- 1024
- 8192
- 65536

For each size:
- allocate matrices safely
- initialize matrices with random data
- run each available implementation
- measure elapsed time with a proper timer
- print:
  - matrix size
  - elapsed time
  - approximate GFLOPS
  - correctness check result
- skip the size gracefully if allocation fails

### 9.3 Timing details
- use a high-resolution timer when possible
- for small/medium sizes, it is okay to repeat runs and keep the best or average
- for huge sizes, avoid excessive repeated runs
- do not benchmark in debug mode
- do not include allocation time in the multiplication timing

### 9.4 Numerical comparison
Use a reasonable tolerance for floating-point comparison.
Print the maximum absolute difference if mismatch occurs.

---

## 10. Makefile requirements

Provide a clean `Makefile` with targets such as:

- `make`
- `make run`
- `make clean`

Compiler requirements:
- `-std=c11`
- `-O3`
- `-Wall`
- `-Wextra`
- preferably `-pedantic`

Optional feature switches:
- `make USE_OPENMP=1`
- `make USE_OPENBLAS=1`

The default build should work even if OpenMP and OpenBLAS are absent.

If OpenBLAS is enabled, link the correct library flags.
If OpenMP is enabled, add the correct compiler flags.

Keep the Makefile readable and easy to modify.

---

## 11. README requirements

Write a concise but professional `README.md` that includes:

1. project purpose
2. file structure
3. how to build
4. how to run
5. explanation of `matmul_plain`
6. explanation of `matmul_improved`
7. explanation of OpenBLAS comparison
8. why very large matrices may be skipped due to memory limits
9. possible future optimization directions
10. brief notes suitable for a student report

Do not invent experimental results in the README.

---

## 12. Code quality requirements

The generated code must be:

- modular
- readable
- reasonably commented
- safe with memory
- robust to invalid inputs
- suitable for a course project
- easy for me to explain in a report or interview

Avoid:
- giant monolithic code
- magic constants without explanation
- unsafe unchecked pointer arithmetic
- fake benchmark outputs
- needlessly complex macro metaprogramming

---

## 13. Nice-to-have extras

If reasonable, you may also include:

- compile-time detection of SIMD support
- a transposed copy of B for improved cache locality
- block size constants with comments
- helper functions for timer utilities
- benchmark output formatted as a table

But only include these if they keep the project understandable.

---

## 14. Output format requirements

Output the **full contents of every file**, each in a separate markdown code block.

Use this exact style:

```c
// filename: matrix.h
...
```

```c
// filename: matrix.c
...
```

```c
// filename: matmul.h
...
```

```c
// filename: matmul.c
...
```

```c
// filename: benchmark.c
...
```

```makefile
# filename: Makefile
...
```

```md
<!-- filename: README.md -->
...
```

Do not omit any file contents.

---

## 15. Final instruction

This is a student project, so balance:
1. correctness
2. clarity
3. robustness
4. optimization

A clean and fully working project with moderate optimization is better than a very complicated project that is hard to compile, hard to explain, or fragile.

Now generate the full project.
