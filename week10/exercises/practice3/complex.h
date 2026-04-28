#pragma once

#include <iostream>

class Complex {
private:
  int real;
  int imag;

public:
  Complex(int r = 0, int i = 0) : real(r), imag(i) {}

  int getReal() const { return real; }
  int getImag() const { return imag; }

  Complex operator-() const { return Complex(-real, -imag); }

  Complex &operator~() {
    imag = -imag;
    return *this;
  }

  friend Complex operator+(const Complex &lhs, const Complex &rhs) {
    return Complex(lhs.real + rhs.real, lhs.imag + rhs.imag);
  }

  friend Complex operator-(const Complex &lhs, const Complex &rhs) {
    return Complex(lhs.real - rhs.real, lhs.imag - rhs.imag);
  }

  friend Complex operator-(const Complex &lhs, int rhs) {
    return Complex(lhs.real - rhs, lhs.imag);
  }

  friend Complex operator-(int lhs, const Complex &rhs) {
    return Complex(lhs - rhs.real, -rhs.imag);
  }

  friend Complex operator*(const Complex &lhs, const Complex &rhs) {
    return Complex(lhs.real * rhs.real - lhs.imag * rhs.imag,
                   lhs.real * rhs.imag + lhs.imag * rhs.real);
  }

  friend Complex operator*(int lhs, const Complex &rhs) {
    return Complex(lhs * rhs.real, lhs * rhs.imag);
  }

  friend Complex operator*(const Complex &lhs, int rhs) {
    return Complex(lhs.real * rhs, lhs.imag * rhs);
  }

  friend bool operator==(const Complex &lhs, const Complex &rhs) {
    return lhs.real == rhs.real && lhs.imag == rhs.imag;
  }

  friend bool operator!=(const Complex &lhs, const Complex &rhs) {
    return !(lhs == rhs);
  }

  friend std::ostream &operator<<(std::ostream &os, const Complex &value) {
    os << value.real;
    if (value.imag >= 0) {
      os << "+" << value.imag;
    } else {
      os << value.imag;
    }
    os << "i";
    return os;
  }

  friend std::istream &operator>>(std::istream &is, Complex &value) {
    is >> value.real >> value.imag;
    return is;
  }
};
