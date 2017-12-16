#include <stdio.h>
#include <stdlib.h>
#include "cachelab.h"

/*
 * 20160759 Dongpyeong Seo
 */

typedef struct {
    char valid;
    unsigned int age;
    unsigned long tag;
    unsigned long block; // actually, it is not used. 
} line;

char help_string[] = "Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>";
int hit_count, miss_count, evict_count = 0;
unsigned int current_age = 0;

void print_help(){
    printf("%s\n", help_string);
}

// initialize all field of all lines to 0.
void initialize(line ** cache, unsigned int  S, unsigned int E){
    /*
    line * templp = *cache;
    for (int i = 0; i < S; ++i){
        for (int j = 0; j < E; ++j){
            templp->valid = 0;
            templp->age = 0;
            templp->tag = 0;
            templp->block = 0;
            templp++;
        }
    }
    */ // this code caused the heap error (it covered the head part of chunk(prev_size, size)).... be careful!
    line * templp;
    for(unsigned int i = 0; i < S; ++i){
        templp = *cache;
        for(unsigned int j = 0; j < E; ++j){
            templp->valid = 0;
            templp->age = 0;
            templp->tag = 0;
            templp->block = 0;
            templp++;
        }
        cache++;
    }
}



// check_set : get a setp which we want to check, calculated tag, and the number of lines (E) 
// -> returns a number which represents hit(0) / miss(1) / miss eviction(2) state
int check_set(line ** setp, unsigned long tag, unsigned int E){
    line * current_line = *setp;
    line * LRU_line = current_line;
    unsigned int min_age = LRU_line->age;
    char hit_flag = 0;
    char miss_flag = 0;
    // set an initial (line *) iterator current_line, some flags, and LRU_line, min_age for LRU policy

    for(unsigned int i = 0; i < E; ++i){
        //-- for debug --
        //printf("line %u\n", i);
        //printf("valid : %d, age : %u\n", current_line->valid, current_line->age);
        //printf("line_tag : %lx, given tag : %lx\n", current_line->tag, tag);
        //-- end debug --

        if(!current_line->valid){ // miss
            miss_flag = 1;
            miss_count++;
            break;
        } 
        if(current_line->tag == tag){ // hit
            hit_flag = 1;
            hit_count++;
            break;
        }
        if(current_line->age < min_age){ // updating the least recently used line and min_age
            LRU_line = current_line;
            min_age = LRU_line->age;
        }
        current_line++;
    }
    if(hit_flag){
        current_line->age = ++current_age; // I missed this part. updata the age.
        return 0;
    }
    else if(miss_flag){
        // insert data into cache
        current_line->valid = 1;
        current_line->tag = tag;
        current_line->age = ++current_age;
        return 1;
    }
    else { // all valid, no tag match -> miss evict

        // replace the least recently used line with new tag and new age.
        LRU_line->tag = tag;
        LRU_line->age = ++current_age;
        miss_count++;
        evict_count++;
        return 2;
    }
}



int main(int argc, char * argv[]) // ./test asdf qwer -> argv = {"./test", "asdf". "qwer"}
{
    unsigned int s, S, E, b, v_flag = 0; // basic informations given from argument.
    char * argtemp; int skip =  0; // used in argument parsing.
    char * trace_file = NULL;
    FILE * fp;
    line ** cache = NULL;
    if (argc < 2 || argv[1][1] == 'h'){
        goto argument_error;
    }
    // argc >= 3 -> arg num >= 2
    //
    // get an argument, and assign s, E, b, trace_file properly.
    for(int c1 = 1; c1 < argc; c1+=1+skip){
        argtemp = argv[c1]; skip = 0;
        for(int c2 = 0; argtemp[c2] != 0; ++c2){
            if(c2 == 0 && argtemp[c2] != '-'){
                goto argument_error;
            }
            switch(argtemp[c2]){
                case '-': break;
                case 'v': v_flag = 1; break;
                case 's': if (c1 + 1 >= argc || atoi(argv[c1 + 1]) == 0){ // if atoi failed it returns 0
                              goto argument_error;
                          }
                          skip = 1; s = atoi(argv[c1 + 1]); break;
                case 'E': if (c1 + 1 >= argc || atoi(argv[c1 + 1]) == 0){
                              goto argument_error;
                          }
                          skip = 1; E = atoi(argv[c1 + 1]); break;
                case 'b': if (c1 + 1 >= argc || atoi(argv[c1 + 1]) == 0){
                              goto argument_error;
                          }
                          skip = 1; b = atoi(argv[c1 + 1]); break;
                case 't': if (c1 + 1 >= argc){
                              goto argument_error;
                          }
                          skip = 1; trace_file = argv[c1 + 1]; break;
                default : goto argument_error;
            }
        }
    }
    if(s == 0 || E == 0 || b == 0 || trace_file == NULL){
argument_error:
        print_help();
        return 0;
    } 
    //printf("s : %u, E : %u, b : %u, trace : %s\n", s, E, b, trace_file);
    //
    // finished parsing the argument strings.
    
    /*
     * Cache info :
     * s, b (bits), E(line per set): given as an argument.
     * address : 8byte = 64bit
     * tag bit = 64 bit - b bit - s bit(- 1(valid bit))(? -> nope)
     *
     * structure of one line : valid bit(1bit) + age bit(4bit) + tag bit(64 - b - s bit) + block bit(b bit) 
     * (actually, block bit is useless since the purpose of this part is just measuring the (hit, miss, eviction) counts)
     *
     * -> For convenience, line structure consists of four fields, {char valid, unsigned int age, unsigned long tag, unsigned long block} -> sizeof(line) == 24
     *
     * structure of one set -> E line -> line * set = malloc(E * 24(sizeof line structure))
     * * I can use priority queue(heap) for LRU(least recently used) policy, but for coding convenience I'll just do linear search.
     *
     * structure of cache ->  2^s set address -> line ** cache = malloc(2^s * 8(address byte))
     *
     */
    // line ** cache : pointer of set (set = pointer of line) -> cache + 2 = pointer of 3rd set, pointer of pointer of 1st line in 3rd set
    // *cache (line *) : set & pointer for lines  -> *(cache + 2) + 2 = pointer of 3rd line of 3rd set

    S = 2 << s;
    cache = (line **) malloc(S * sizeof(line *));
    line ** tempc = cache;
    for (int i = 0; i < S; ++i){
        *tempc = (line *) malloc(E * sizeof(line));
        tempc++;
    }
    
    // initialize the valid, age, tag, block
    initialize(cache, S, E);

    // open a given trace file.
    fp = fopen(trace_file, "r");
    
    char op;
    unsigned long address;
    int size;
    int ret, check_res; 
    unsigned long tag, setindex, temp_addr;
    line ** setp;
    
    while((ret = fscanf(fp, " %c %lx,%d\n", &op, &address, &size)) != EOF){
        if (ret != 3 || op == 'I'){
            continue;
        }
        //printf("given : %c, %d, %d\n", op, address, size);
        
        temp_addr = address >> b; // since address is unsigned long, '>>' is a logical right shift 'shr'!

        tag = temp_addr >> s;
        setindex = (tag << s) ^ temp_addr;
        setp = cache + setindex;
        // got a target set, and also calculated the tag of the very set.
        
        if(op == 'L' || op == 'S'){
            check_res = check_set(setp, tag, E);
            if(!v_flag){
                continue;
            }
            if(check_res == 0){ // hit
                printf("%c %lx,%d hit\n", op, address, size);
            }
            else if(check_res == 1){ // miss
                printf("%c %lx,%d miss\n", op, address, size);
            }
            else if(check_res == 2){ // miss eviction
                printf("%c %lx,%d miss eviction\n", op, address, size);
            }
            else{ //error
                printf("something goes wrong in check_set.\n");
            }
        }
        else if(op == 'M'){
            check_res = check_set(setp, tag, E);
            if(check_set(setp, tag, E) != 0){ // M = (L,S), second check should return 'hit' state.
                printf("error! something goes wrong in check_set.\n");
            }
            if(!v_flag){
                continue;
            }
            if(check_res == 0){ // hit hit
                printf("%c %lx,%d hit hit\n", op, address, size);
            }
            else if(check_res == 1){ // miss hit
                printf("%c %lx,%d miss hit\n", op, address, size);
            }
            else if(check_res == 2){ // eviction hit
                printf("%c %lx,%d miss eviction hit\n", op, address, size);
            }
            else{ // error
                printf("something goes wrong in check_set.\n");
            }
        }
    } 

    tempc = cache;
   
    // Make all malloced chunks free! 
    for (int i = 0; i < S; ++i){
        free(*tempc);
        tempc++;
    }
    free(cache);
    
    printSummary(hit_count, miss_count, evict_count);
    return 0;
}


