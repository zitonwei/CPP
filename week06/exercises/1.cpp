#include <iostream>
using namespace std;
int * create_array(int size)
{
    int* arr = new int[size];// int arr[size];

    for(int i = 0; i < size; i++)
        arr[i] = i * 10;
    return arr;
}
int main()
{
    int len = 16;
    int *ptr = create_array(len);
    for(int i = 0; i < len; i++)
        cout << ptr[i] << " ";

    delete ptr;
    ptr = nullptr;
    
    return 0;
}

// 1.cpp: In function ‘int* create_array(int)’:
// 1.cpp:8:12: warning: address of local variable ‘arr’ returned [-Wreturn-local-addr]
//     8 |     return arr;
//       |            ^~~
// 1.cpp:5:9: note: declared here
//     5 |     int arr[size];
//       |         ^~~

// Segmentation fault (core dumped)