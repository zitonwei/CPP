#ifndef VABS_H
#define VABS_H
#include <cstddef> // for size_t

bool vabs(int* p, std::size_t n);
bool vabs(float* p, std::size_t n);
bool vabs(double* p, std::size_t n);

#endif