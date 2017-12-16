#include <stdio.h>
#include <stdlib.h>

int main()
{
    int a1[3]={1,2,3};
    int * ptr = 0x1111;
    int p = 1;
    int * mal1 = (int *)malloc(16);
    void * mal2 = malloc(16);
    *(&p) += 1;
    *a1;
//    **a1; -> you cannot dereference int object!!!!
    printf("p : %d\n", p);
    printf("sizeof a1 : %d\n", sizeof(a1));
    printf("sizeof ptr 0x1111 : %d\n", sizeof(ptr));
    printf("sizeof 1 : %d\n", sizeof(1));
    printf("sizeof malloc pointer casted to int * : %d\n", sizeof(mal1));
    printf("sizeof malloc pointer not casted : %d\n", sizeof(mal2));
    printf("a1 : %d, &a1[1] : %d\n", a1, &a1[1]);
    return 0;
}
