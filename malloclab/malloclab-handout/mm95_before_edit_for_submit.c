/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In our approach, a chunk(== block) is classified into two type, malloc chunk and free chunk, and each chunk consists of at most 4 part, as follows.
 * Each chunk has its size info with 1 bit "current_inuse flag" which store the state(MALLOC - 1 / FREE - 0), 
 * and in the case of free chunk, the chunk also has the chunk pointer for fd(forward free chunk) and bk(backward free chunk), which maintains the lists of free chunks.
 *
 * Free list system is maintained by using segregated free lists.
 * We used 10 free lists for maintaining free chunks.
 * Those free lists are stored as a form of "list head", stored in the starting heap spaces about each for 4 bytes, total 40 bytes amount.
 * void ** variable "free_lists" indicates the starting address for the first free_list_head.
 *
 * The size classes of each segregated free lists are based on the chunk size, not a data size, and are divided by (2^n + 8) as follows.
 *
 * data size    list_ index  chunk size(multiples of 8)
 * -------------------------------------
 *  1 ~ 16          0          16, 24
 *  17 ~ 32         1          32, 40
 *  33 ~ 64         2          48 ~ 72
 *  65 ~ 128        3          80 ~ 136
 *  129 ~ 256       4          144 ~ 264
 *  257 ~ 512       5          272 ~ 520
 *  513 ~ 1024      6          528 ~ 1032
 *  1025 ~ 2048     7          1040 ~ 2056
 *  2049 ~ 4096     8          2064 ~ 4104
 *  4096 ~ inf      9          4112 ~ inf
 *
 * Allocating new space is done by two ways,
 * 1. using an appropriate free chunk(with setting it be malloc'd)
 * 2. increasing the available heap space restricted by mem_brk, with calling mem_sbrk with proper incrementing size.
 *
 * Searching an appropriate free chunk from the free list is done by manner of "best-fit" policy. *
 *
 * Finally, there is no specific prologue block or epilogue block. 
 * Boundary managing is done by 4 byte initial padding block and two global pointers.
 * 1. Initial padding block :
 *      Since our malloc system checks the physically previous chunk by checking the current_chunk_address - 4,
 *      we need an initial padding block which works as prev_size for the first chunk.
 *      So we assigned unsigned int 0 right after the heap spaces for free lists, therefore PREVCHK(first_chunk) indicates first_chunk itself. (we'll use isPrevValid() which checks whether PREVCHK(chunk) < chunk, so it works.)
 * 2. void * first_chunk
 *      It indicates the spaces where the first chunk will be located. (== a space right after the initial padding block)
 * 3. char * mm_brk
 *      It works exactly same as mem_brk in memlib.c.
 *
 * chunk structure : 
 * 1. (flagged) size 4byte, fd(forward free chunk pointer) 4byte (in 32bit), bk(backward free chunk pointer) 4byte, prev_size 4byte => minimum chunk size = 16 byte.
 *    a forward free chunk is the free chunk closer to the head of the free list
 *    a backward free chunk is the free chunk farther from the head of the free list
 * 2. chunk size (when unflagged) : multiple of 8.
 *
 *   - 4 byte -
 * --------------
 *|   prev_size  |		<- footer of the previous chunk / this prev_size is the part of the previous chunk.
 * --------------  ---------  start of the current chunk
 *|    size    C |		<- "header" of the current chunk / (flagged) size (lower 1 bits is a flag; C for current_inuse or inuse (is current chunk malloc chunk?)) / both in FREE chunk and MALLOC chunk!
 * --------------
 *|      fd      |		<- forward free chunk pointer in FREE chunk / start of data part in MALLOC chunk
 * --------------
 *|      bk      |		<- backward free chunk pointer in FREE chunk / data part in MALLOC chunk
 * --------------
 *|     ....     |            <- data parts
 * --------------
 *|     size     |		<- "footer" of the current chunk / (not flagged) size / same value with UNFLAG(size) / prev_size for the next chunk / both in FREE chunk and MALLOC chunk!
 * --------------  ---------- end of the current chunk
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "Steam",
    /* First member's full name */
    "Dongpyeong Seo",
    /* First member's email address */
    "bludrimjr@kaist.ac.kr",
    /* Second member's full name (leave blank if none) */
    "Yusung Sim",
    /* Second member's email address (leave blank if none) */
    "sys5867@kaist.ac.kr"
};

/* DEBUG and VERBOSE for debugging, used in several functions to run do_debug() / mm_check() */
#define DEBUG 0
#define VERBOSE 0

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define FREE 0
#define ALLOC 1

/* constant for aligning data size */
#define K 7 / 8 // if  K * (2 ^ n) <= datasize <= (2 ^ n) -> datasize will be aligned to (2 ^ n) for worst case.

/* assigning 4 byte data in the given ptr */
#define PUT4(ptr, valORptr) (*((unsigned int *) (ptr)) = (unsigned int)(valORptr))

/* get the pointer that indicates the data part (or payload part) from the chunk pointer */
#define GET_DATAPTR(chk_ptr) ((void *)((unsigned int)(chk_ptr) + 4))

/* get the pointer that indicates the chunk part from the data pointer */
#define GET_CHUNKPTR(data_ptr) ((void *)((unsigned int)(data_ptr) - 4))

/* get the previous size of the given chunk, not the footer part of the given chunk */
#define GET_PREVSIZE(chk_ptr) (*( (unsigned int *)( (unsigned int)(chk_ptr) - 4 ) ))

/* get the size (with current_inuse flag set) of the given chunk */
#define GET_FSIZE(chk_ptr) (*( (unsigned int *)(chk_ptr))) 

/* get the pure size (without flag information) of the given chunk */
#define GET_SIZE(chk_ptr) (*((unsigned int *)(chk_ptr)) & ~7)

/* get the current_inuse flag from the given chunk */
#define GET_C(chk_ptr) (GET_FSIZE(chk_ptr) & 1)

/* set the current_inuse flag on the given chunk */
#define SET_C(chk_ptr, flag) (PUT4(chk_ptr, GET_FSIZE(chk_ptr) & ~1 | (flag)))

/* calculate the previous chunk pointer for the given chunk. */
#define PREVCHK(chk_ptr) ((void *) ( (unsigned int)(chk_ptr) - GET_PREVSIZE(chk_ptr) ))

/* calculate the next chunk pointer for the given chunk.  */
#define NEXTCHK(chk_ptr) ((void *) ( (unsigned int)(chk_ptr) + GET_SIZE(chk_ptr) ))

/* calculate the pointer that indicates footer part (prev_size part for the next chunk) of the given chunk */
#define FOOTERP(chk_ptr) ((void *)((unsigned int)(chk_ptr) + GET_SIZE(chk_ptr) - 4))

/* set the size of the given chunk / both header size and footer size will be set */
#define SET_SIZE(chk_ptr, fsize) (PUT4(chk_ptr, fsize)); (PUT4(FOOTERP(chk_ptr), fsize & ~7))

/* set the fd (forward free chunk pointer) of the given chunk */
#define SET_FD(chk_ptr, new_fd) (*((unsigned int *)((unsigned int)(chk_ptr) + 4)) = (unsigned int)(new_fd))

/* set the bk (backward free chunk pointer) of the given chunk */
#define SET_BK(chk_ptr, new_bk) (*((unsigned int *)((unsigned int)(chk_ptr) + 8)) = (unsigned int)(new_bk))

/* get the fd chunk pointer of the given chunk */
#define FDFREECHK(chk_ptr) (*((void **)((unsigned int)(chk_ptr) + 4)))

/* get the bk chunk pointer of the given chunk */
#define BKFREECHK(chk_ptr) (*((void **)((unsigned int)(chk_ptr) + 8)))

static char * mm_brk;
static void ** free_lists; // same value with mem_start_brk, but works as the pointer that indicates the several free lists in starting 40 byte(10 free list pointer) area on the heap.
// *(free_listsk + listindex) -> (listindex)th free-list.
static void * first_chunk; // pointer indicates the place where the first chunk will be located.

/* 
- helper functions
int isNextValid(void * chunkptr);
int isPrevValid(void * chunkptr);
int find_listindex(unsigned int chunksize);
void * find_chunk(unsigned int chunksize, int listindex);
void mm_unlink(void * chunkptr);
void insert(void * chunkptr);
void set_malloc_chunk(void * chunkptr, unsigned int alloc_chunksize);
void * expand_heap(int incr);
*/


/* returns whether the next chunk (physically next chunk) of the given chunk is a valid chunk or not. */
static int isNextValid(void * chunkptr)
{
	void * next_chunk = NEXTCHK(chunkptr);
	return first_chunk < next_chunk && next_chunk < (void *)mm_brk;
}

/* returns whether the previous chunk (physically previous chunk) of the given chunk is a valid chunk or not */
static int isPrevValid(void * chunkptr)
{
	void * prev_chunk = PREVCHK(chunkptr);
	return prev_chunk < chunkptr && prev_chunk < mm_brk && prev_chunk >= first_chunk;
}

/* returns the index of the appropriate free list for the given chunksize */
static int find_listindex(unsigned int chunksize)
{
	// just for correctness, we did a hard job in this case.
	if (chunksize < 16){
		fprintf(stderr, "find_list : wrong chunksize %d. chunksize is below 16\n", chunksize);
		exit(1);
	}
	if (chunksize <= 24) { // payload size 1 ~ 16 Byte
		return 0;
	}
	else if (chunksize <= 40) { // payload size 17 ~ 32 Byte
		return 1;
	}
	else if (chunksize <= 72) { // payload size 33 ~ 64 Byte
		return 2;
	}
	else if (chunksize <= 136) { // payload size 65 ~ 128 Byte
		return 3;
	}
	else if (chunksize <= 264) { // payload size 129 ~ 256 Byte
		return 4;
	}
	else if (chunksize <= 520) { // payload size 257 ~ 512 Byte
		return 5;
	}
	else if (chunksize <= 1032) { // paylooad size 513 ~ 1024 Byte
		return 6;
	}
	else if (chunksize <= 2056) { // payload size 1025 ~ 2048 Byte
		return 7;
	}
	else if (chunksize <= 4104) { // payload size 2049 ~ 4096 Byte
		return 8;
	}
	else return 9; // payload size 4097 ~ U_INTMAX
}


/* 
 * find_chunk :
 *
 * best fit policy.
 * it searches the free lists and returns the free chunk which has a chunk size which is closest to the given chunksize of the free chunks in the given free list..
 * if search failes, it recursively continues searching through out free_lists + listindex + 1, free_lists + listindex + 2, ... , free_lists + 9.
 */
static void * find_chunk(unsigned int chunksize, int listindex)
{
    void * cursor;
    void * free_list_head;
    void * bestfit_chunk = NULL; // should be initialized.
    unsigned int minimum = 0xffffffff;
    unsigned int cursor_chunksize;

    if (listindex > 9) return NULL;

    if (listindex < 0)
    {
        fprintf(stderr, "find_chunk : wrong listindex, listindex should not be negative\n");
        exit(1);
    }

    // 0 <= listindex <= 9
    free_list_head = *(free_lists + listindex);

    /* fisrt-fit codes - not used 
    for (cursor = free_list_head; cursor != NULL; cursor = BKFREECHK(cursor))
    {
        if (GET_SIZE(cursor) >= chunksize)
        {
              if (GET_C(cursor) != FREE)
              {
                    fprintf(stderr, "find_chunk : found chunk is not a free chunk\n");
                    exit(1);
              }
              return cursor;
        }
    }
    */
    for(cursor = free_list_head; cursor != NULL; cursor = BKFREECHK(cursor))
    {
        if((cursor_chunksize = GET_SIZE(cursor)) >= chunksize)
        {
            if(GET_C(cursor) != FREE)
            {
                fprintf(stderr, "find_chunk : found chunk is not a free chunk\n");
                exit(1);
            }

            if(cursor_chunksize < minimum)
            {
                // update the bestfit chunk.
                bestfit_chunk = cursor;
                minimum = cursor_chunksize;

                // if the optimally fit chunk was found, return it immediately.
                if(cursor_chunksize == chunksize)
                    return bestfit_chunk;
            }
        }
    }
    if(bestfit_chunk != NULL) return bestfit_chunk;

    // continue searching on the next free lists recursively.
    return find_chunk(chunksize, listindex + 1);
}


/* unlink(remove from the list) the given free chunk from the appropriate free list.*/
static void mm_unlink(void * chunkptr)
{
    void * temp_fd;
    void * temp_bk;
    void * free_list_head;
    void ** free_list_headp;

    //if(DEBUG) fprintf(stdout, "mm_unlink : test chunkptr 0x%x\n", chunkptr);

    int listindex = find_listindex(GET_SIZE(chunkptr));
    free_list_headp = free_lists + listindex;
    free_list_head = *free_list_headp;

    if (GET_C(chunkptr) == ALLOC) {
        fprintf(stderr, "mm_unlink : tried to unlink the malloc chunk.\n");
        exit(1);
    }

    temp_fd = FDFREECHK(chunkptr);
    temp_bk = BKFREECHK(chunkptr);

    if (temp_fd == NULL) {
        if (free_list_head != chunkptr) {
            fprintf(stderr, "mm_unlink : free list is weird\n");
            exit(1);
        }
        *free_list_headp = temp_bk;
    }
    else {
        SET_BK(temp_fd, temp_bk);
    }

    if (temp_bk != NULL) {
        SET_FD(temp_bk, temp_fd);
    }
}

/* insert the given chunk to a head of the free list which has the appropriate list index for the size of the given chunk. */
static void insert(void * chunkptr)
{
    void * free_list_head;
    void ** free_list_headp;

    if(DEBUG) fprintf(stdout, "insert : test chunkptr 0x%x\n", chunkptr);
    int listindex = find_listindex(GET_SIZE(chunkptr));

    // given chunk should be a free chunk.
    if (GET_C(chunkptr) != FREE) {
        fprintf(stderr, "insert : tried to insert malloc chunk in the free list\n");
        exit(1);
    }

    free_list_headp = free_lists + listindex;
    free_list_head = *free_list_headp;

    SET_FD(chunkptr, NULL);
    SET_BK(chunkptr, free_list_head);

    if (free_list_head != NULL)
        SET_FD(free_list_head, chunkptr);

    *free_list_headp = chunkptr;
}


/*
 * assumption :
 * 1. given chunk should have a valid size set(header and footer). flag state does not matter.
 * 2. IF the given chunk was a free chunk, it should have been unlinked in caller side
 * 3. given chunk should have a size >= alloc_chunksize
 *
 * jobs :
 * 1. size of the chunk >= alloc_chunksize + 16 -> 
 *    - devide it into two free chunks
 *    - set the first chunk in malloc state
 *    - coalesce & insert(both will be done by calling mm_free) the rest chunk in the free list.
 * 2. alloc_chunksize <= size of the chunk < alloc_chunksize + 16 -> no devide, set the given chunk in malloc state.
 */
static void set_malloc_chunk(void * chunkptr, unsigned int alloc_chunksize)
{
    unsigned int init_chunksize = GET_SIZE(chunkptr); // initial size of the given chunk
    unsigned int rest_chunksize; // init_chunksize - alloc_chunksize
    void * rest_chunk;

    if (init_chunksize < alloc_chunksize)
    {
        fprintf(stderr, "set_malloc_chunk : assumption error, given chunk 0x%x has a smaller size 0x%x than the alloc_chunksize 0x%x\n", chunkptr, init_chunksize, alloc_chunksize);
        exit(1);
    }

    rest_chunksize = init_chunksize - alloc_chunksize;

    if (rest_chunksize >= 16)
    {
        // If the rest size is big enough, cut the given chunk into two chunk. 
        // The previous chunk will be malloc'd, and the rest chunk will be used as a new free chunk.

        SET_SIZE(chunkptr, alloc_chunksize);
        SET_C(chunkptr, ALLOC);

        if (!isNextValid(chunkptr))
        {
            fprintf(stderr, "set_malloc_chunk : next chunk of the chunkptr 0x%x is not valid. alloc_chunksize = 0x%x, init_chunksize = 0x%x\n", chunkptr, alloc_chunksize, init_chunksize);
            exit(1);
        }

        rest_chunk = NEXTCHK(chunkptr);
        SET_SIZE(rest_chunk, rest_chunksize);

        mm_free(GET_DATAPTR(rest_chunk));
    }
    else
    {
        // If the rest size is not big enough, just set the given chunk in malloc state
        SET_SIZE(chunkptr, init_chunksize); // actually it is not needed
        SET_C(chunkptr, ALLOC);
    }
}

/* 
 * expands the heap space by using mem_sbrk with updating mm_brk properly, 
 * and returns the starting address of the expanded heap space, gotten from mem_sbrk.
 * 
 * If mem_sbrk fails, the return pointer will be (void *)-1
 */
static void * expand_heap(int incr)
{
    void * return_ptr;
    if (incr < 0) {
        fprintf(stderr, "expand_heap : it is shrinking the heap space\n");
        exit(1);
    }
    return_ptr = mem_sbrk(incr);
    mm_brk += incr;
    return return_ptr;
}


/* when each main functions (mm_malloc, mm_free, mm_realloc) return, we will do the debugging step by mm_check() if DEBUG is on */
static void do_debug()
{
    int errno;
    if(DEBUG)
    {
        fprintf(stdout, "mm_check() starts\n");
        if((errno = mm_check()) < 0) // 0 -> check passed
        {
            fprintf(stderr, "result of mm_check = errno : %d\n", errno);
            exit(1);
        }
    }
}

/* 
 * mm_init - initialize the malloc package.
 * if terminated normally, returns 0 / otherwisem returns -1
 */
int mm_init(void)
{
    void * initial_prevsize_padding;
    free_lists = (void **)mem_sbrk(40); // spaces for free_list heads.

    if(free_lists == (void *)-1) return -1;

    for (int i = 0; i < 10; ++i){
        PUT4(free_lists + i, 0); // initialize 10 free list heads to NULL
    }

    if((initial_prevsize_padding = mem_sbrk(4)) == (void *)-1) return -1;

    PUT4(initial_prevsize_padding, 0); // initial prev_size which works as a padding. set it to 0 -> PREVCHK(first_chunk) == first_chunk -> caught by isPrevValid()
    mm_brk = (unsigned int)mem_heap_hi() + 1;
    first_chunk = (void *)mm_brk;

    return 0;
}

/* 
 * mm_malloc : gets a data size(dsize) and returns an appropriate void * for the data part of the new allocated chunk.
 * 1. Align the dsize (dsize -> dsize) if needed.
 * 2-1. find an appropriate free chunk in the several free lists.
 * 2-2. If found, set the found free chunk to the malloc chunk. (cutting, coalescing will be done by set_malloc_chunk())
 * 3. If not found, expand the heap space about the amount we needed. We can use the physically last chunk of the heap if the chunk is free chunk.
 * 
 * Always allocate a block whose size is a multiple of the alignment.
 * The minimum chunk size is 16.
 */
void *mm_malloc(size_t dsize) // dsize = data size
{
    unsigned int chunksize;
    unsigned int restsize;
    int listindex;
    void * found_chunk;
    void * last_chunk;

    if (dsize <= 0) return NULL;
    // for payload size(dsize) 65 ~ 128(MAX), 129 ~ 256(MAX), 257 ~ 512(MAX), 
    // if the given dsize >= K * MAX, 
    // we aligned dsize to the dsize MAX(closest 2 ^ n) so that we can treat the extreme malicious malloc-free pattern with only a few penalty in memory utilization
    // * K is defined by MACRO, now K = 7 / 8
    if(dsize > 64 && dsize <= 128){
      if(dsize >= (128 * K))
          dsize = 128;
    }
    else if(dsize > 128 && dsize <= 256){
      if(dsize >= (256 * K))
          dsize = 256;
    }
    else if(dsize > 256 && dsize <= 512){
      if(dsize >= (512 * K))
          dsize = 512;
    }

    chunksize = ALIGN(dsize) + 8; // 4 for header (size) , 4 for footer (prev_size for the physically next chunk)
    //if(DEBUG) fprintf(stdout, "mm_malloc : starts with dsize %d, chunksize %d\n", dsize, chunksize);
    listindex = find_listindex(chunksize);

    if ((found_chunk = find_chunk(chunksize, listindex)) != NULL) {
        /* if appropriate free chunk is found */
        mm_unlink(found_chunk);
        set_malloc_chunk(found_chunk, chunksize);
        //do_debug();
        return GET_DATAPTR(found_chunk);
    }
    else
    {
        // if there was no appropriate free chunk, we should make an extra heap space by using mem_sbrk()
        // 
        // optimization :
        //	  if the last chunk on the current heap space is free chunk, 
        //    we can reduce the increasing size for mem_sbrk() by using the very last free chunk.
        if (isPrevValid(mm_brk))
        {
            last_chunk = PREVCHK(mm_brk);
            if (GET_C(last_chunk) == FREE)
            {
                //fprintf(stdout, "mm_malloc : expending heap, using free chunk!! last_chunk : 0x%x\n", last_chunk);
                restsize = chunksize - GET_SIZE(last_chunk);
                if (restsize <= 0)
                {
                    fprintf(stderr, "mm_malloc : finding the appropriate chunk may failed. the last chunk is a free chunk 0x%x, and it's big enough.\n", last_chunk);
                    exit(1);
                }
                expand_heap(restsize);

                mm_unlink(last_chunk);
                SET_SIZE(last_chunk, chunksize);
                set_malloc_chunk(last_chunk, chunksize);
                //do_debug();
                return GET_DATAPTR(last_chunk);
              }
        }

        // failed to use the last chunk.
        // just expend the heap space.
        last_chunk = expand_heap(chunksize);
        SET_SIZE(last_chunk, chunksize);
        set_malloc_chunk(last_chunk, chunksize);
        //do_debug();
        return GET_DATAPTR(last_chunk);
    } 
}


/*
 * mm_free - Freeing a block, doing coalescing with prev or next free blocks.
 * coalescing will be done in 4 cases, each handled by insert and mm_unlink. 
 */
void mm_free(void *data_ptr)
{
    // mm_free with 4 cases of coalescing.
    void* ptr = GET_CHUNKPTR(data_ptr);
    void* prev = PREVCHK(ptr);
    void* next = NEXTCHK(ptr);
    int cur_size = GET_SIZE(ptr);
    int prev_c = GET_C(prev);
    int next_c = GET_C(next);
    int is_first = (prev == ptr);
    int is_last = (next == mm_brk);

    //if(DEBUG) fprintf(stdout, "mm_free : given data_ptr = 0x%x, chunkptr = 0x%x, original state = %d\n", data_ptr, ptr, GET_C(ptr)); 
    
    SET_C(ptr, FREE);

    if (prev_c == ALLOC || is_first) {
        if (next_c == ALLOC || is_last) {
            // case 1. PREV : ALLOC / NEXT : ALLOC
            int new_size = cur_size;
            // ptr is freed, so just insert
            insert(ptr);
        }
        else { // next_c == FREE
            // case 2. PREV : ALLOC / NEXT : FREE -> coalesce with NEXT

            // before coalescing, unlink the next chunk(in free)
            mm_unlink(next);

            int next_size = GET_SIZE(next);
            int new_size = cur_size + next_size; // new_size = current + next size

            // coalesce current and next chunk, and insert it into the free list.
            SET_SIZE(ptr, new_size);
            SET_C(ptr, FREE);
            insert(ptr);
        }
    }
    else { // prev_c == FREE
        if (next_c == ALLOC || is_last) {
            // case 3. PREV : FREE / NEXT : ALLOC -> coalesce with PREV

            // before coalescing, unlink the previous chunk.
            mm_unlink(prev);

            int prev_size = GET_SIZE(prev);
            int new_size = cur_size + prev_size; // new_size = current + previous size

            // coalesce current and previous chunk, and insert it into free list.
            SET_SIZE(prev, new_size);
            SET_C(prev, FREE);
            insert(prev);
        }
        else { // next_c == FREE
            // case 4. PREV : FREE / NEXT : FREE -> coalesce with both PREV and NEXT

            // before coalescing, unlink both previous and next chunk.
            mm_unlink(next);
            mm_unlink(prev);

            int prev_size = GET_SIZE(prev);
            int next_size = GET_SIZE(next);
            int new_size = cur_size + prev_size + next_size; // new_size = previous + current + next size

            // coalesce previous, current and next chunk, and insert it into free list.
            SET_SIZE(prev, new_size);
            SET_C(prev, FREE);
            insert(prev);
        }
    }

    //do_debug();
      
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 *
 * Some optimization :
 * 1. If original chunksize(old_chunksize) >= required chunksize(aligned from req_datasize)
 *    -> new allocation and memory copying are not needed.
 * 2. If original chunksize < required chunksize
 *    two way.
 *    1) If physically next chunk is a free chunk
 *       -> Try to MERGE the very free chunk with the original chunk.
 *          If it is big enough, new allocation and memory copying are not needed.
 *    2) If physically previous chunk is a free chunk
 *       -> Try to MERGE the very free chunk with the original chunk.
 *          If it is big enough, new allocation is not needed, but still memocy copying is needed.
 */
void *mm_realloc(void *old_dataptr, unsigned int req_datasize)
{
    void * old_chunkptr = GET_CHUNKPTR(old_dataptr);
    void * new_dataptr;
    void * next_chunk;
    void * prev_chunk;
    unsigned int req_chunksize = ALIGN(req_datasize) + 8;
    unsigned int old_chunksize;
    unsigned int next_chunksize;
    unsigned int prev_chunksize;
    unsigned int move_datasize;
    unsigned int merged_chunksize;

    //if(DEBUG) fprintf(stdout, "mm_realloc : old_dataptr = 0x%x, old_chunkptr = 0x%x, req_datasize = %d\n", old_dataptr, old_chunkptr, req_datasize);
    if(req_chunksize < 16)
        req_chunksize = 16;

    if(req_datasize == 0) {
        mm_free(old_dataptr);
        //do_debug();
        return NULL;
    }

    if(old_dataptr == NULL) {
        //do_debug();
        return mm_malloc(req_datasize);
    }

    old_chunksize = GET_SIZE(old_chunkptr);

    move_datasize = req_datasize;
    if(req_datasize > old_chunksize - 8)
        move_datasize = old_chunksize - 8;

    if(req_chunksize <= old_chunksize)
    {
        /* requested realloc size <= maximum possible data size of the old_chunk -> we don't have to newly allocate chunk and copy datas. */
        if(old_chunksize - req_chunksize >= 16)
        {
            /* 
             * we can devide old_chunk into two chunks.
             * 1. malloc chunk whose chunk size is req_chunksize
             * 2. free chunk whose chunk size is old_chunksize - req_chunksize
             */

            //if(DEBUG) fprintf(stdout, "mm_realloc : devide old_chunkptr 0x%x\n", old_chunkptr);

            // malloc'ing the old_chunkptr with req_chunksize, and freeing the rest chunk, coalescing, inserting will all be done by set_malloc_chunk.
            set_malloc_chunk(old_chunkptr, req_chunksize);
        }
        //do_debug();
        // don't have to allocate new malloc chunk or copy datas. just return it.
        return old_dataptr;

    }
    else
    {
        /* 
         * In usual cases, when requested realloc size > maximum possible data size of the old_chunk -> we should make new malloc chunk and call memcpy() to copy datas to the new malloc chunk.
         * 
         * But, IF
         * 1. next chunk of the old_chunk is a free chunk
         * 2. the very next chunk(free) is big enough so that we can satisfy the requested realloc size
         * -> we don't have to make new malloc chunk and don't have to call memcpy by merging old_chunk and the very next free chunk.
         */
        
        // using physically next free chunk.
        if(isNextValid(old_chunkptr))
        {
            next_chunk = NEXTCHK(old_chunkptr);
            if(GET_C(next_chunk) == FREE)
            {
                /* next_chunk of old_chunk is valid free chunk -> try to use it */
                next_chunksize = GET_SIZE(next_chunk);
                merged_chunksize = next_chunksize + old_chunksize;
                if(merged_chunksize >= req_chunksize)
                {
                    /* next_chunk is big enough -> use it, merge it with the old_chunk */
                    mm_unlink(next_chunk);
                    SET_SIZE(old_chunkptr, merged_chunksize);

                    set_malloc_chunk(old_chunkptr, req_chunksize);
                    //do_debug();
                    return GET_DATAPTR(old_chunkptr);
                }
            }
        }

        /* Also , IF
         * 1. previous chunk of the old_chunk is a free chunk.
         * 2. the very previous chunk(free) is big enough so that we can satisfy the requested realloc size by merging the previous chunk and old_chunk
         * -> we don't have to make new malloc chunk, we only have to do the similar jobs that memcpy does by merging old_chunk and previous chunk.
         *  * we cannot call memcpy this time, since the source part and destination part can be overlapped.
         */

        // using physically previous free chunk.
        if(isPrevValid(old_chunkptr))
        {
            prev_chunk = PREVCHK(old_chunkptr);
            if(GET_C(prev_chunk) == FREE)
            {
                /* prev_chunk of old_chunk is valid free chunk -> try to use it */
                prev_chunksize = GET_SIZE(prev_chunk);

                /*
                if(prev_chunksize + old_chunksize >= req_chunksize)
                {
                    // now, we can use free prev_chunk, by merging it with the original old_chunk.
                    mm_unlink(prev_chunk);
                    SET_SIZE(prev_chunk, prev_chunksize + old_chunksize);

                    // before setting the prev_chunk(merged with old_chunk) in malloc state, copy the datas in old_chunk
                    cursor = (char *)GET_DATAPTR(prev_chunk);
                    for(int i = 0; i < req_datasize; ++i)
                    {
                        *cursor = *(cursor + prev_chunksize);
                    }

                    // now, set the merged prev_chunk in malloc state
                    set_malloc_chunk(prev_chunk, req_chunksize);

                    do_debug();
                    return GET_DATAPTR(prev_chunk);
                }
                */
                
                if(prev_chunksize + old_chunksize >= req_chunksize )//&& prev_chunksize >= old_chunksize)
                {
                    /* prev_chunk of old_chunk is big enough -> use it, merge it with the old_chunk */
                    mm_unlink(prev_chunk);
                    SET_SIZE(prev_chunk, prev_chunksize + old_chunksize);

                    memmove(GET_DATAPTR(prev_chunk), old_dataptr, req_datasize);

                    set_malloc_chunk(prev_chunk, req_chunksize);

                    //do_debug();
                    return GET_DATAPTR(prev_chunk);
                }
            }
        }

        /* all optimizations are not available -> make new malloc chunk and copy the data */
        new_dataptr = mm_malloc(req_datasize);
        memcpy(new_dataptr, old_dataptr, req_datasize);
        mm_free(old_dataptr);
        //do_debug();
        return new_dataptr; 
    }
}

// mm_check : heap consistency checker. return negative when inconsistency found.
int mm_check() {
    void *ptr = first_chunk;
    int implicit_free_chunks = 0;
    // iterate memory like implicit list
    // then check each chunk's consistency
    int k = 0;
    if(VERBOSE)
        printf("******** Start Checking by Implicit Listing ********\n");
    while (ptr < mm_brk) {
        if(VERBOSE)
            printf("=> %dth chunk : %p, state : %d\n", k, ptr, GET_C(ptr));
        int cur_size = GET_SIZE(ptr);
        int cur_c = GET_C(ptr);
        void* prev = PREVCHK(ptr);
        void* next = NEXTCHK(ptr);
        int is_first = (prev == ptr);
        int is_last = (next == mm_brk);
        int prev_c = GET_C(prev);
        int next_c = GET_C(next);
        int prev_size = GET_SIZE(prev);

        // 1. check consistency with prev, next chunks
        // 1) is prev's next is current?
        if (!(is_first || NEXTCHK(prev) == ptr)) {
            printf("NEXTCHK(prev) != ptr\n");
            return -1;
        }

        // 2) is next's prev is current?
        if (!(is_last || PREVCHK(next) == ptr)) {
            printf("PREVCHK(next) != ptr\n");
            return -2;
        }
        // 2. check coalescing
	  if (cur_c == FREE) {
            // 0) update total number of free chunks for later
            implicit_free_chunks += 1;
            // 1) current and prev is FREE
            if (!is_first && prev_c == FREE) {
                printf("prev == FREE\n");
                return -3;
            }
            // 2) current and next if FREE
            if (!is_last && next_c == FREE) {
                printf("next == FREE\n");
                return -4;
            }
        }

        // 3. check footer of self
        if (cur_size != *(unsigned int *)FOOTERP(ptr)) {
            printf("ptr's size != ptr's prev_size (footer value)\n");
            return -5;
        }

        // 4. check consistency of prev_size
        if (!(is_first || prev_size == GET_PREVSIZE(ptr))) {
            printf("prev's size != ptr's prev_size\n");
            return -6;
        }

        if (!(is_last || cur_size == GET_PREVSIZE(next))) {
            printf("ptr's size != next's prev_size");
            return -7;
        }

        // F. update ptr and k
        ptr = next;
        k++;
    }

    // iterate each free list of size classes
    if(VERBOSE)
        printf("******** Start Iterating each Free Lists ********\n");
    int index;
    void** free_list_headp;
    int explicit_free_chunks = 0;

    // for each size classes
    for (index = 0; index < 10; ++index) {
        if(VERBOSE)
            printf("Free List Index : %d\n", index);

        // get head of the free list
        free_list_headp = free_lists + index;
        ptr = *free_list_headp;
        int j = 0;
        while (1) {
            if (ptr == NULL) break;
            if(VERBOSE)
                printf("%dth free chunk : %p\n", j, ptr);

            // 0. is ptr in the heap range?
            if (ptr < first_chunk || ptr >= mm_brk) {
                printf("ptr out of heap range\n");
                return -8;
            }
            int cur_size = GET_SIZE(ptr);
            int cur_c = GET_C(ptr);
            // 1. is current flagged free?
            if (cur_c != FREE) {
                printf("ptr != FREE\n");
                return -9;
            }
            else {
                explicit_free_chunks += 1;
            }
            // F. update ptr and j
            ptr = BKFREECHK(ptr);
            j++;
        }
    }
    
    // check total numbers of free chunks match
    if (implicit_free_chunks != explicit_free_chunks) {
        printf("Number of free chunks do not match\n");
        return -10;
    }
    return 0;
}












