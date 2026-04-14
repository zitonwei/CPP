#include <iostream>
#include "swap.h"
using namespace std;

// void swap(int&a, int&b) {
//     int tmp = a;
//     a = b;
//     b = tmp;
// }

int main() {
    int a = 1, b = 2;
    cout << "before swap, a = " << a << ", b = " << b << endl;
    swap(a, b);
    cout << "after swap, a = " << a << ", b = " << b << endl;
    return 0;
}