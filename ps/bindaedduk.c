#include<stdio.h>
#include<stdlib.h>
#include<string.h>


typedef struct {
    int size;
    int ud;
} dd;

//typedef dd* dp;

typedef struct {
    dd* dstack[6];
    int scale;
} ds;

void dd_init(dd* ddukp, int size, int ud){
    printf("let's start making dduk!\n");
    ddukp->size = size;
    ddukp->ud = ud;
    printf("I made dduk!\n");
}

void dd_flip(dd* ddukp){
    ddukp->ud = ddukp->ud ^ 1;
}

dd* dd_flipped(dd* ddukp, dd* res){
    dd_init(res, ddukp->size, ddukp->ud);
    return res;
}

void ds_init(ds* dsp, int scale){
    for(int i = 0; i < 6; ++i){
        dsp->dstack[i] = (dd*)0;
    }
    dsp->scale = scale;
}

void ds_init_stack(ds* dsp, int scale, dd** stack){
    printf("making ds with stack\n");
    for(int i = 0; i < 6; ++i){
        dsp->dstack[i] = *(stack+i);
    }
    dsp->scale = scale;
} 





int main(){
    int N;
    int a;
    char b;
    ds* ourdsp;
    dd d0, d1, d2, d3, d4, d5;
    dd* tempds[6] = {&d0, &d1, &d2, &d3, &d4, &d5}; 

    scanf("%d\n", &N);
    printf("passed?\n");
    ds_init_stack(ourdsp, N, tempds);
    printf("passed!\n");
    for(int i = 0; i < N; ++i){
        scanf("%d %c", &a, &b);
        printf("got it\n");
        dd_init(ourdsp->dstack[i], a, (b == '+' ? 1 : 0));
    }
    
    return 0;
}
