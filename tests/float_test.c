#include <stdio.h>

int main()
{
    int x = 0x7fffffff;
    


    if (x == (int)(float) x)
        printf("x == (int)(float) x, passed\n");
    
    if (x == (int)(double) x)
        printf("x == (int)(double) x, passed\n");
    
    if (2/3 == 2/3.0)
        printf("2/3 == 2/3.0 : passed\n");
    return 0;
}
