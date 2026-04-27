# Project 3 实验报告

## 1. 实验题目

基于 C 语言的矩阵乘法实现与优化

## 2. 实验环境

- 操作系统：Linux / WSL2
- 内核：`Linux ztw 6.6.87.2-microsoft-standard-WSL2 x86_64`
- 编译器：`cc (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0`
- 编译选项：`-std=c11 -O3 -Wall -Wextra -pedantic`
- 并行支持：OpenMP
- 数学库：OpenBLAS

本次测试使用的编译命令为：

```sh
make clean
make USE_OPENBLAS=1 USE_OPENMP=1
./benchmark > 1.out
```

## 3. 实验内容

本实验实现了四种矩阵乘法方案：

- `plain`：基础三重循环版本
- `v1`：将循环顺序从 `ijk` 改为 `ikj`
- `v2`：在 `ikj` 基础上加入分块优化和 OpenMP
- `v3`：Strassen 递归版本
- `openblas`：调用 `cblas_sgemm` 作为参考实现

矩阵元素类型统一使用 `float`。

## 4. 基础实现 plain

`plain` 采用最直接的三重循环：

```c
for (i = 0; i < a->rows; ++i)
    for (j = 0; j < b->cols; ++j)
        for (k = 0; k < a->cols; ++k)
            c->data[i * c->cols + j] += a->data[i * a->cols + k] * b->data[k * b->cols + j];
```

该实现优点是简单、清晰，便于作为基准版本；缺点是对 `B[k][j]` 的访问是跨行取列，缓存局部性较差，因此大矩阵下性能较低。

## 5. 三种改进实现

### 5.1 v1：循环重排

`v1` 的思路是将最基础的 `ijk` 三重循环改成 `ikj`。这样做以后，内层循环会连续访问 `B` 的一整行和 `C` 的一整行，从而减少跨列访问带来的 cache miss。

关键代码如下：

```c
for (i = 0; i < a->rows; ++i) {
    const float *a_row = &a->data[i * a->cols];
    float *c_row = &c->data[i * c->cols];

    for (k = 0; k < a->cols; ++k) {
        const float a_ik = a_row[k];
        const float *b_row = &b->data[k * b->cols];

        for (j = 0; j < b->cols; ++j)
            c_row[j] += a_ik * b_row[j];
    }
}
```

这一版没有改变矩阵乘法的数学公式，只是改变了访存顺序。其主要作用是：

- 提高 `B` 和 `C` 的连续访问能力
- 降低访存跳跃
- 提高编译器自动向量化机会

本次使用 GCC 向量化报告进行检查，编译命令为：

```sh
cc -O3 -fopenmp -fopt-info-vec-optimized -fopt-info-vec-missed -c matmul.c -o /tmp/matmul_vec.o
```

编译器输出中出现了：

```text
matmul.c:91:27: optimized: loop vectorized using 16 byte vectors
matmul.c:91:27: optimized: loop versioned for vectorization because of possible aliasing
```

这说明 `v1` 中最内层的 `j` 循环确实被 GCC 自动向量化了，因此报告中更准确的说法是：`v1` 是 SIMD-friendly 写法，并且在当前编译选项下成功触发了编译器自动向量化。

### 5.2 v2：分块优化 + OpenMP

`v2` 在 `ikj` 的基础上进一步加入分块。程序将矩阵按固定块大小 `64` 划分为若干子块，然后按块进行乘法。这样可以让当前计算所需的数据尽量留在 cache 中。

关键代码如下：

```c
for (i0 = 0; i0 < a->rows; i0 += MATMUL_BLOCK_SIZE) {
    size_t i1 = i0 + MATMUL_BLOCK_SIZE < a->rows ? i0 + MATMUL_BLOCK_SIZE : a->rows;

    for (k0 = 0; k0 < a->cols; k0 += MATMUL_BLOCK_SIZE) {
        size_t k1 = k0 + MATMUL_BLOCK_SIZE < a->cols ? k0 + MATMUL_BLOCK_SIZE : a->cols;

        for (j0 = 0; j0 < b->cols; j0 += MATMUL_BLOCK_SIZE) {
            size_t j1 = j0 + MATMUL_BLOCK_SIZE < b->cols ? j0 + MATMUL_BLOCK_SIZE : b->cols;

            for (i = i0; i < i1; ++i)
                for (k = k0; k < k1; ++k)
                    for (j = j0; j < j1; ++j)
                        c_row[j] += a_ik * b_row[j];
        }
    }
}
```

同时在外层块循环上加入 OpenMP：

```c
#pragma omp parallel for schedule(static) private(j0, k0)
```

这一版的主要目标是：

- 通过 blocking 改善 cache 局部性
- 通过 OpenMP 利用多核 CPU
- 在中大规模矩阵上取得更高吞吐率

同样地，GCC 的向量化报告中还出现了：

```text
matmul.c:134:40: optimized: loop vectorized using 16 byte vectors
matmul.c:134:40: optimized: loop versioned for vectorization because of possible aliasing
```

说明 `v2` 在块内最内层 `j` 循环上也触发了自动向量化。因此 `v2` 的性能提升不仅来自分块和 OpenMP，也来自编译器在连续访存循环上的自动 SIMD 化。

### 5.3 v3：Strassen 递归算法

`v3` 采用的是 Strassen 矩阵乘法。它不是简单的循环优化，而是算法级优化。普通矩阵乘法的时间复杂度是 `O(n^3)`，而 Strassen 通过把一个大方阵拆成四个子块，并用 7 次子矩阵乘法代替 8 次子矩阵乘法，从理论上降低了复杂度。

其基本分块形式为：

```text
A = [A11 A12]    B = [B11 B12]
    [A21 A22]        [B21 B22]
```

然后构造 7 个中间矩阵：

```text
M1 = (A11 + A22)(B11 + B22)
M2 = (A21 + A22)B11
M3 = A11(B12 - B22)
M4 = A22(B21 - B11)
M5 = (A11 + A12)B22
M6 = (A21 - A11)(B11 + B12)
M7 = (A12 - A22)(B21 + B22)
```

最后合并得到结果块：

```text
C11 = M1 + M4 - M5 + M7
C12 = M3 + M5
C21 = M2 + M4
C22 = M1 - M2 + M3 + M6
```

代码中的关键部分如下：

```c
if ((rc = matrix_add(a11, a22, s1, 1)) != 0) goto cleanup;
if ((rc = matrix_add(b11, b22, s2, 1)) != 0) goto cleanup;
if ((rc = matmul_strassen_recursive(s1, s2, m1)) != 0) goto cleanup;
...
if ((rc = matrix_add(m1, m4, c11, 1)) != 0) goto cleanup;
if ((rc = matrix_add(c11, m5, c11, -1)) != 0) goto cleanup;
if ((rc = matrix_add(c11, m7, c11, 1)) != 0) goto cleanup;
```

这一版的理论优势是复杂度更低，但它的缺点也很明显：

- 递归调用较多
- 临时矩阵分配很多
- 数据拷贝和矩阵加减开销较大
- 在实际机器上不一定快于 `v2`

## 6. 正确性测试

`1.out` 中的小规模正确性测试结果如下：

- `plain vs v1`：`n=4,7,16,31` 全部 `PASS`
- `plain vs v2`：`n=4,7,16,31` 全部 `PASS`
- `plain vs v3`：`n=4,7,16,31` 全部 `PASS`
- `v3 vs OpenBLAS`：`n=4,7,16,31` 全部 `PASS`

说明三种改进版本在当前测试范围内都与基准结果一致，并且 `v3` 与 OpenBLAS 结果一致。

## 7. Benchmark 结果

### 7.1 n = 16

| 实现 | 时间（s） | GFLOPS | 正确性 |
|---|---:|---:|---|
| plain | 0.000003 | 2.990 | PASS |
| v1 | 0.000001 | 6.776 | PASS |
| v2 | 0.000042 | 0.194 | PASS |
| v3 | 0.000042 | 0.194 | PASS |
| openblas | 0.000000 | 16.483 | PASS |

### 7.2 n = 128

| 实现 | 时间（s） | GFLOPS | 正确性 |
|---|---:|---:|---|
| plain | 0.001897 | 2.211 | PASS |
| v1 | 0.000326 | 12.856 | PASS |
| v2 | 0.000645 | 6.502 | PASS |
| v3 | 0.000246 | 17.032 | PASS |
| openblas | 0.000038 | 110.009 | PASS |

### 7.3 n = 1024

| 实现 | 时间（s） | GFLOPS | 正确性 |
|---|---:|---:|---|
| plain | 2.227743 | 0.964 | PASS |
| v1 | 0.087690 | 24.490 | PASS |
| v2 | 0.019736 | 108.811 | PASS |
| v3 | 0.291508 | 7.367 | PASS |
| openblas | 0.002139 | 1004.035 | PASS |

### 7.4 n = 8192

| 实现 | 时间（s） | GFLOPS | 正确性 |
|---|---:|---:|---|
| plain | 7013.361078 | 0.157 | PASS |
| v1 | 113.915864 | 9.652 | PASS |
| v2 | 8.246949 | 133.323 | PASS |
| v3 | 66.821979 | 16.454 | PASS |
| openblas | 1.632326 | 673.586 | PASS |

### 7.5 三组改进版本对比汇总

| 规模 | v1 时间（s） | v1 GFLOPS | v2 时间（s） | v2 GFLOPS | v3 时间（s） | v3 GFLOPS |
|---|---:|---:|---:|---:|---:|---:|
| 16 | 0.000001 | 6.776 | 0.000042 | 0.194 | 0.000042 | 0.194 |
| 128 | 0.000326 | 12.856 | 0.000645 | 6.502 | 0.000246 | 17.032 |
| 1024 | 0.087690 | 24.490 | 0.019736 | 108.811 | 0.291508 | 7.367 |
| 8192 | 113.915864 | 9.652 | 8.246949 | 133.323 | 66.821979 | 16.454 |

### 7.6 n = 65536

| 实现 | 结果 |
|---|---|
| 全部实现 | allocation failed |

`1.out` 中显示该规模估计工作集约为 `65536.00 MiB`，当前环境无法完成分配，因此该测试被跳过。

## 8. 结果分析

1. `plain` 始终是最慢的版本，尤其在 `1024` 和 `8192` 规模下性能很低，说明基础三重循环在大矩阵上缓存局部性较差。
2. `v1` 相比 `plain` 有明显提升，说明仅通过 `ikj` 循环重排就可以显著改善访存模式。
3. `v2` 是本项目中最有效的自实现版本。在 `1024` 时达到 `108.811 GFLOPS`，在 `8192` 时达到 `133.323 GFLOPS`，明显优于 `plain`、`v1` 和 `v3`。
4. `v3` 使用的是 Strassen 递归算法，理论复杂度优于普通 `O(n^3)` 乘法，但当前实现中递归、临时矩阵分配和数据拷贝开销较大，因此实际运行结果不如 `v2`。例如在 `1024` 时只有 `7.367 GFLOPS`，在 `8192` 时为 `16.454 GFLOPS`。
5. 从三组改进版本对比表可以看出，小规模时结果波动较大，但在真正有代表性的 `1024` 和 `8192` 规模下，`v2` 明显优于 `v1` 和 `v3`。
6. OpenBLAS 在所有有效规模下都明显快于自实现版本，说明成熟库在缓存打包、向量化和并行调度方面更充分。

## 9. 实验结论

本实验实现了 `plain`、`v1`、`v2`、`v3` 四种矩阵乘法方案，并与 OpenBLAS 进行了对比测试。实验结果表明：

- `v1` 证明了循环重排是有效的优化手段
- `v2` 证明了分块优化在中大规模矩阵上效果最好，是本项目最成功的自实现版本
- `v3` 作为 Strassen 递归实现，理论上更优，但当前实现下没有取得实际性能优势
- OpenBLAS 作为成熟库，性能仍明显领先于自实现版本

因此，如果以“自实现版本中性能最好”为标准，本项目的最佳方案是 `v2`；如果以“整体最快”为标准，则 OpenBLAS 表现最好。
