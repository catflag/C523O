#include <stdio.h>

int main()
{
    int pid;
    int * x;

    *x = 1; // it is an invalid access, since I did not initialize the int * x.
/*
    //pid = fork();
    if(pid == 0)
    {
        //*x += 1;
        //printf("child : x =  %d\n", ++(*x));
        exit(0);
    }

    //printf("parent : x = %d\n", --(*x));
*/
    return 0;
}
