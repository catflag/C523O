#include <stdio.h>
#include <stdlib.h>
#include <wait.h>
#include <sys/types.h>
#include <unistd.h>

#define PIDNUM 20
#define N 20
int main()
{
/*
    pid_t pids[PIDNUM];
    int * c_status; // this version does not work, for whatever reason...
    pid_t returned; // it always print abnormal terminate...
    int i;

    for(i = 0; i < PIDNUM; ++i)
    {
        if((pids[i] = fork()) == 0)
        {
            exit(100+i);
        }
    } 
    
    for(i = 0; i < PIDNUM; ++i)
    {
        returned = wait(c_status);
        if(WIFEXITED(*c_status))
        {
            printf("child process %d exited with status %d\n", returned, WEXITSTATUS(*c_status));
        }
        else
        {
            printf("abnormal exit!\n");
        }
    }

*/ 
    pid_t pid[N];
    int i, child_status;
    for (i = 0; i < N; i++)
        if ((pid[i] = fork()) == 0) {
            exit(100+i); 
        }

    for (i = 0; i < N; i++) { 
        pid_t wpid = wait(&child_status);
	  if (WIFEXITED(child_status))
            printf("Child %d terminated with exit status %d\n", wpid, WEXITSTATUS(child_status));
	  else
            printf("Child %d terminate abnormally\n", wpid);
    }

    return 0;
}
