#include <iostream>

using namespace std;

class Demo {
private:
    int id;

public:
    static int num;

    Demo(int cid = 0) : id(cid) {}

    void displayByObject() const {
        cout << "this is: " << this << ", id is: " << id << endl;
    }

    static void displayByClass() {
        cout << "The value of the static num is: " << num << endl;
    }
};

int Demo::num = 2025;

int main() {
    cout << "Reason of the original errors:" << endl;
    cout << "1. Demo obj; needs a default constructor, but the original class only had Demo(int)." << endl;
    cout << "2. The original code defined two display() functions with the same parameter list." << endl;
    cout << "   Static/non-static alone cannot distinguish overloaded member functions." << endl;
    cout << endl;

    Demo obj;
    Demo obj1(1);

    obj.displayByObject();
    obj1.displayByObject();
    Demo::displayByClass();

    return 0;
}



// #include<iostream>
// using namespace std;
// class Demo
// {
//     private:
//         int id;
//         void display(){
//             cout<<"thisis:"<<this<<",idis:"<<this->id<<endl;
//         }
//     public:
//     Demo(int cid=0){
//         this->id=cid;
//     }
//     static int num;
//     void display() {
//         cout<<"The value of the static num is:"<<num<<endl;
//     }
// } ;
// int main() {
//     Demo obj;
//     Demo obj1(1);
//     obj.display();
//     obj1.display();
//     Demo::display();
//     return 0;
// }