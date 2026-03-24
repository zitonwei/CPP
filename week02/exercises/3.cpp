#include <iostream>
using namespace std;

const float exp = 1e-6;

int main() {
    cout << fixed;
    float f1 = 1.0f;
    cout<<"f1 = "<<f1<<endl;
    float a = 0.1f;
    float f2 = a+a+a+a+a+a+a+a+a+a;
    cout<<"f2 = "<<f2<<endl;
    if(f1 == f2)  //TIPS: Modify the code here
        cout << "f1 == f2" << endl;
    else
        cout << "f1 != f2" << endl;
    if(abs(f1-f2) < exp)  //TIPS: Modify the code here
        cout << "f1 == f2" << endl;
    else
        cout << "f1 != f2" << endl;
return 0;
}