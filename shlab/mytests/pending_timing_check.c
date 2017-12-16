#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>


static int count = 0;

void myhandler(int sig)
{
    printf("handler sleep start\n");
    sleep(5);
    printf("handler waked up!\n");
    count ++;
}

int main()
{
    pid_t pid;

    signal(SIGCHLD, myhandler);

    if((pid = fork()) == 0)
    {
        // child
        sleep(2);
        printf("child1 exits\n");
        exit(0);
    }
    if((pid = fork()) == 0)
    {
        printf("child2 exits\n");
        exit(0);
    }
    sleep(4);
    waitpid(-1, NULL, 0);
    printf("count : %d\n", count); // -> count = 2 -> pending is released before the jump to the myhandler!
    return 0;

}
