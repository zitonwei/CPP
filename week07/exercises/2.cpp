#include <iostream>
#include <string>

using namespace std;

// The structure definition
struct stuinfo {
    string name;
    int age;
};

// 1. General Function Template
template <typename T>
int Compare(T a, T b) {
    if (a > b) return 1;
    if (a < b) return -1;
    return 0;
}

// 2. Explicit Specialization for stuinfo
// Syntax: template <> return_type function_name<type>(params)
template <>
int Compare<stuinfo>(stuinfo s1, stuinfo s2) {
    if (s1.age > s2.age) return 1;
    if (s1.age < s2.age) return -1;
    return 0;
}

int main() {
    // Testing Integer
    cout << "Compare(10, 20):    " << Compare(10, 20) << endl;

    // Testing Character
    cout << "Compare('z', 'a'):  " << Compare('z', 'a') << endl;

    // Testing Floating-point
    cout << "Compare(3.14, 3.15):" << Compare(3.14, 3.15) << endl;

    // Testing struct stuinfo
    stuinfo alice = {"Alice", 20};
    stuinfo bob = {"Bob", 22};
    cout << "Compare(Alice(20), Bob(22) by age): " << Compare(alice, bob) << endl;

    return 0;
}