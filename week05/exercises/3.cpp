#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int numA = 0x11223344;
    
    // 1. 判定大小端 (基于 byte0 的存储位置)
    // 获取 numA 的起始字节地址
    unsigned char *pA = (unsigned char *)&numA;
    const char *endian = (*pA == 0x44) ? "LE" : "BE";
    printf("Data A_addr: %p, A_data: 0x%x, This is %s\n", (void*)&numA, numA, endian);

    if (argc == 2) {
        if (argv[1][0] == 'H') {
            int *pnumB = (int*)malloc(sizeof(int));
            if (pnumB != NULL) {
                /* complete code here */
                unsigned char *pb = (unsigned char *)pnumB;
                // 将 numA 的字节逆序存入 pnumB 指向的空间
                for (int i = 0; i < sizeof(int); i++) {
                    pb[i] = pA[sizeof(int) - 1 - i];
                }
                
                printf("Data B_addr: %p, B_data: 0x%x\n", (void*)pnumB, *pnumB);
                
                // 3-2. Valgrind 提示：记得释放堆内存
                free(pnumB);
            }
        } 
        else if (argv[1][0] == 'S') {
            /* complete code here */
            int numB;
            unsigned char *pb = (unsigned char *)&numB;
            // 同样执行字节反转
            for (int i = 0; i < sizeof(int); i++) {
                pb[i] = pA[sizeof(int) - 1 - i];
            }

            printf("Data B_addr: %p, B_data: 0x%x\n", (void*)&numB, numB);
        }
    }
    return 0;
}