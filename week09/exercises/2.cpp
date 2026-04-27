#include <iostream>

using namespace std;

class ConstMember {
private:
    const int m_a;

public:
    ConstMember(int a) : m_a(a) {}

    void display() const {
        cout << "The value of the const member variable m_a is: " << m_a << endl;
    }
};

int main() {
    ConstMember o1{666};
    ConstMember o2{42};

    o1.display();
    o2.display();

    cout << "If \"o1 = o2\" is uncommented, compilation fails." << endl;
    cout << "Reason: m_a is a const member, so it cannot be reassigned after construction." << endl;
    cout << "Therefore, the compiler deletes the default copy-assignment operator." << endl;

    // o1 = o2;

    return 0;
}
