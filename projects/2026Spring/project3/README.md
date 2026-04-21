# Matrix Multiplication Project

This project implements dense matrix multiplication in C using `float`, with both a straightforward baseline and a more optimized version intended for performance benchmarking.

## Files

- `matrix.h`, `matrix.c`: matrix allocation, initialization, comparison, and utility helpers
- `matmul.h`, `matmul.c`: baseline and improved matrix multiplication implementations
- `benchmark.c`: correctness tests and performance benchmark driver
- `Makefile`: build rules with optional OpenMP and OpenBLAS support

## Build

Default build:

```sh
make
```

Build with OpenMP:

```sh
make USE_OPENMP=1
```

Build with OpenBLAS:

```sh
make USE_OPENBLAS=1
```

Build with both:

```sh
make USE_OPENMP=1 USE_OPENBLAS=1
```

## Run

```sh
make run
```

The benchmark first performs correctness checks on small random matrices, then benchmarks square matrices of size:

- `16`
- `128`
- `1024`
- `8192`
- `65536`

## `matmul_plain`

`matmul_plain()` is the baseline implementation. It uses the classic triple-nested-loop algorithm and computes `C = A x B` directly. This version is easy to read and useful as a correctness and performance reference.

## `matmul_improved`

`matmul_improved()` keeps the code understandable while improving cache behavior:

- it first transposes matrix `B`
- it uses blocked multiplication to improve locality
- it can use OpenMP when compiled with `USE_OPENMP=1`

This version is typically faster because the inner loop reads contiguous memory from both `A` and the transposed copy of `B`, and blocking reduces cache misses.

## OpenBLAS Comparison

When compiled with `USE_OPENBLAS=1`, the benchmark also runs `cblas_sgemm` and compares its output against the project implementations. If OpenBLAS is not enabled, the benchmark prints a clear message and skips that comparison.

## Large Matrix Sizes

Very large dense matrices may be skipped. A dense `65536 x 65536` matrix requires a huge amount of memory, and the benchmark includes safety checks to avoid crashes or unrealistic allocations. The driver also skips the plain `O(n^3)` baseline for very large sizes when it would be impractical to run in a normal environment.

## Notes for a Report

This project balances:

- correctness
- readable C code
- basic but meaningful optimization
- practical benchmarking

The improved implementation is suitable for explanation in a student report because its optimizations are standard and easy to justify: better memory locality, blocking, and optional parallel execution.

## Possible Future Work

- SIMD intrinsics for the innermost loop
- adaptive block-size tuning for different CPUs
- packing panels instead of only transposing `B`
- support for non-square benchmark cases
