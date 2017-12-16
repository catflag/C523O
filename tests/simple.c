#include <stdio.h>
#include <math.h>
int main()
{
    //printf("%d\n",(int)0.99f); -> 0
    char c1[5] = {'a','b','c','\0', 'e'};
    char c2[3] = "abc";
    printf("%d\n", 1 >> -1);
    printf("what is 1==1? : %d\n", 1==1);
    if(!strncmp(c1, c2, 5))
        printf("wow its same!\n");
    else
        printf("ooh its not same...\n");
    
    printf("2/3? %d\n", 2/3);
    return 0;
}
