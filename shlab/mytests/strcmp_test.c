#include<stdio.h>
#include<stdlib.h>
#include<string.h>


int main()
{
    char mynewline[200];
    if(mynewline == NULL)
        printf("ooo it's null\n");
    if(fgets(mynewline, 200, stdin) == NULL)
        printf("fgets error\n");

    if(!strcmp(mynewline, "\n"))
        printf("mynewline is \\n!\n");
    else
        printf("fail...\n");
    return 0;
}
