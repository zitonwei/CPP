#include <iostream>
#include "vabs.h"

int main() {
    printf("Addr (int):    %p\n", (bool (*)(int*, std::size_t))vabs);
    printf("Addr (float):  %p\n", (bool (*)(float*, std::size_t))vabs);
    printf("Addr (double): %p\n", (bool (*)(double*, std::size_t))vabs);
    
    return 0;
}