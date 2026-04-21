#include <iostream>

using namespace std;
template <typename T> 
void swap(T &a, T &b)
{
    T temp; 
    temp = a;
    a = b;
    b = temp;
}
signed main() {
    return 0;
}