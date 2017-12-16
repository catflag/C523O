#include<stdio.h>

#define MAX 0x7FFFFFFF
#define MIN 0xFFFFFFFF

int main()
{
    int a = MAX;
    int b = MIN;
    int tx,ty;
    unsigned int ux, uy;
    ux = 0xFFFFFFFF;
    tx = ux;
    ty = -1;
    uy = ty;
    printf("tx, ty, ux, uy : %d, %d, %u, %u\n", \
            tx, ty, ux, uy);
    printf("1+2**32 : %ld\n", 1+4294967296);
    printf("a,b = %d,%d\n", a, b);
    
    return 0;
}
