#include "vabs.h"
#include <cmath>

template <typename T>
bool compute_vabs(T* p, std::size_t n) {
    if (p == nullptr) return false; // Pointer validity check
    for (std::size_t i = 0; i < n; ++i) {
        p[i] = std::abs(p[i]);
    }
    return true;
}

bool vabs(int* p, std::size_t n) { return compute_vabs(p, n); }
bool vabs(float* p, std::size_t n) { return compute_vabs(p, n); }
bool vabs(double* p, std::size_t n) { return compute_vabs(p, n); }