#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

int main()
{
    pid_t pid;
    void * p = malloc(16);
    printf("void * arithmetic test\n");
    printf("void * p : %x, p + 1 : %x\n", p, p + 1);

    printf("test starts\n");
    wait(NULL);

    printf("test finished\n");
    return 0;
}
