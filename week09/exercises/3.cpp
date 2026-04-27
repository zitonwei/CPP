#include <iostream>

using namespace std;

class Complex {
private:
    double realPart;
    double imaginaryPart;

public:
    Complex(double real = 0.0, double imaginary = 0.0)
        : realPart(real), imaginaryPart(imaginary) {}

    Complex add(const Complex& other) const {
        return Complex(realPart + other.realPart,
                       imaginaryPart + other.imaginaryPart);
    }

    Complex subtract(const Complex& other) const {
        return Complex(realPart - other.realPart,
                       imaginaryPart - other.imaginaryPart);
    }

    void display() const {
        cout << realPart;
        if (imaginaryPart >= 0) {
            cout << " + " << imaginaryPart << "i";
        } else {
            cout << " - " << -imaginaryPart << "i";
        }
        cout << endl;
    }
};

int main() {
    Complex c1;
    Complex c2(1.5, -4.0);

    cout << "c1 = ";
    c1.display();

    cout << "c2 = ";
    c2.display();

    Complex sum = c1.add(c2);
    Complex diff = c1.subtract(c2);

    cout << "c1 + c2 = ";
    sum.display();

    cout << "c1 - c2 = ";
    diff.display();

    return 0;
}
