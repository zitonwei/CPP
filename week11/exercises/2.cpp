#include <iostream>
#include <vector>
#include <memory>
#include <iomanip>

class Matrix {
private:
    int rows, cols;
    std::shared_ptr<float[]> data; // 使用智能指针共享数据

public:
    Matrix(int r, int c) : rows(r), cols(c), data(new float[r * c]()) {
        // () 会将数组初始化为 0
    }

    void set(int r, int c, float val) {
        data[r * cols + c] = val;
    }

    Matrix operator+(const Matrix& other) const {
        if (this->rows != other.rows || this->cols != other.cols) {
            throw std::invalid_argument("Matrix dimensions must agree for addition.");
        }

        Matrix result(rows, cols);
        for (int i = 0; i < rows * cols; ++i) {
            result.data[i] = this->data[i] + other.data[i];
        }
    
        return result;
    }
    
    void print(const std::string& name) const {
        std::cout << name << " is:" << std::endl;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                std::cout << data[i * cols + j] << " ";
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    Matrix a(3, 4);
    a.set(1, 2, 3.0f);
    Matrix b(3, 4);
    b.set(2, 3, 4.0f);
    Matrix c = a + b;
    
    a.print("a");
    b.print("b");
    c.print("c");

    Matrix d = a;
    d.print("Before assignment, d");

    d = b;
    d.print("After assignment, d");

    return 0;
}