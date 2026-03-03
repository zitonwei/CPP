#include <stdio.h>

int main() {
    double a = 0.1, b = 0.2, c = 0.3;
    
    // FILE *fp = fopen("output.txt", "w");
    // if (fp == NULL) {
    //     perror("not found output.txt");
    //     return 1;
    // }
    
    fprintf(stdout, "%.2lf\n%.2lf\n%.2lf\n", a, b, c);

    // fclose(fp);
    return 0;
}
