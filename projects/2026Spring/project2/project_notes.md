# Project 2 Notes

## Compile and Run Commands

### C
Compile:

```bash
gcc -O3 -std=c11 dotproduct.c -o dotproduct
```

Run:

```bash
./dotproduct
```

### Java
Compile:

```bash
javac Dotproduct.java
```

Run:

```bash
java Dotproduct
```

## Concise Report Outline

1. Introduction
   - State the goal: compare C and Java implementations of vector dot product.
   - Briefly explain why data type and vector length matter.
2. Method
   - Describe the five supported types and separate dot product functions.
   - Explain random data generation ranges and larger accumulation types.
3. Experimental Setup
   - List hardware, OS, compiler, JVM version, and optimization settings.
   - List tested vector lengths, warm-up runs, and averaged timing runs.
4. Results
   - Present CSV data as tables or charts.
   - Compare runtime by language, type, and vector length.
5. Analysis
   - Discuss JIT warm-up, compiler optimization, cache effects, and memory bandwidth.
   - Explain why smaller element types do not always run faster than `int`.
6. Conclusion
   - Summarize the main performance observations.
   - State which language and data types performed best under the tested conditions.

## Expected Performance Analysis

- C is often faster at the start because native optimized code runs immediately without JVM startup or JIT warm-up.
- Java can become more competitive after warm-up because the JIT compiler optimizes hot loops during execution.
- As vector length grows, runtime usually scales roughly linearly until memory bandwidth and cache behavior become dominant.
- For very large arrays, the benchmark becomes increasingly memory-bound, so language differences may shrink.
- `byte` and `short` are not guaranteed to outperform `int` because many CPUs naturally process 32-bit integers efficiently, and narrower types may require extra sign extension or conversion.
- `float` may be faster or use less memory bandwidth than `double`, but actual results depend on the CPU vector units, compiler/JIT optimizations, and how accumulation is implemented.
- `double` uses more memory per element, which can hurt cache efficiency, especially for large vectors.
- Small input sizes are more affected by fixed overhead such as loop setup, timer noise, JVM startup, and warm-up effects.
