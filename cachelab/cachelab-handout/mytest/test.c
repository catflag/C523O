#include "cachelab.c"

int main(int argc, char * argv[]){
    FILE * fp;
    char op;
    unsigned long address;
    int size;
    int ret;
    if (argc > 1){
        for(int i = 0; i < argc; ++i){
            printf("%s\n", argv[i]);   
        }
        fp = fopen(argv[1], "r");
    }
    else
        fp = fopen("traces/yi.trace", "r");
/*
    while(ret = fscanf(fp, "*%c %lx,%d\n", &op, &address, &size) != 0){
        printf("op : %c, address : %lx, size : %d\n", op, address, size);
        if (ret != 3){
            printf("passed\n");
            continue;
        }
        printf("not passed\n");
    }
    printf("ret : %d\n", ret);
    printf("op : %c, addr : %lx, size : %d\n", op, address, size);
*/
    while(fscanf(fp, " %c %lx,%d\n", &op, &address, &size) != EOF){
        printf("op : %c, address : %lx, size : %d\n", op, address, size);
    }
    fclose(fp);
    return 0;
}
