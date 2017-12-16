/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h" // should be changed to memlib.h

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
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define PUT4(ptr, valORptr) (*((unsigned int *) (ptr)) = (unsigned int)(valORptr))

#define UNFLAG(size) ((size) & ~7)

#define GET_DATAPTR(chk_ptr) ((void *)((unsigned int)(chk_ptr) + 4))

#define GET_PREVSIZE(chk_ptr) (*( (unsigned int *)( (unsigned int)(chk_ptr) - 4 ) ))

#define GET_FSIZE(chk_ptr) (*( (unsigned int *)(chk_ptr))) // F_ means "flagged"

#define GET_SIZE(chk_ptr) (*((unsigned int *)(chk_ptr)) & ~7)

#define GET_P(chk_ptr) ((GET_FSIZE(chk_ptr) & 2) >> 1)

#define GET_C(chk_ptr) (GET_FSIZE(chk_ptr) & 1)

#define SET_P(chk_ptr, flag) (PUT4(chk_ptr, GET_FSIZE(chk_ptr) & ~2 | (2 * (flag))))

#define SET_C(chk_ptr, flag) (PUT4(chk_ptr, GET_FSIZE(chk_ptr) & ~1 | (flag)))

/* Calculates the previous chunk pointer for the given chk_ptr. * since footer of the previous chunk is not flagged, unflagging is not needed. */
#define PREVCHK(chk_ptr) ((void *) ( (unsigned int)(chk_ptr) - GET_PREVSIZE(chk_ptr) ))

/* Calculates the next chunk pointer for the given chk_ptr. * since header of the current chunk is flagged, unflagging is needed. */
#define NEXTCHK(chk_ptr) ((void *) ( (unsigned int)(chk_ptr) + GET_SIZE(chk_ptr) ))

/* the pointer for the part for forward free chunk pointer in the given chk_ptr */
#define FDPART(chk_ptr) ((void *)((unsigned int)(chk_ptr) + 4))

/* the pointer for the part for backward free chunk pointer in the given chk_ptr */
#define BKPART(chk_ptr) ((void *)((unsigned int)(chk_ptr) + 8))

/* footer part (prev_size part for the next chunk) of the given chk_ptr */
#define FOOTERP(chk_ptr) ((void *)((unsigned int)(chk_ptr) + GET_SIZE(chk_ptr) - 4))

#define SET_SIZE(chk_ptr, fsize) (PUT4(chk_ptr, fsize)); (PUT4(FOOTERP(chk_ptr), fsize & ~7))

#define SET_ONLYSIZE(chk_ptr, fsize) (PUT4(chk_ptr, fsize))

#define SET_FD(chk_ptr, new_fd) (*((unsigned int *)((unsigned int)(chk_ptr) + 4)) = (unsigned int)(new_fd))

#define SET_BK(chk_ptr, new_bk) (*((unsigned int *)((unsigned int)(chk_ptr) + 8)) = (unsigned int)(new_bk))

#define FDFREECHK(chk_ptr) (*((void **)((unsigned int)(chk_ptr) + 4)))

#define BKFREECHK(chk_ptr) (*((void **)((unsigned int)(chk_ptr) + 8)))

/* 
 * chunk structure : 
 * 1. (flagged) size 4byte, fd(forward free chunk pointer) 4byte (in 32bit), bk(backward free chunk pointer) 4byte, prev_size 4byte => minimum chunk size = 16 byte.
 * 2. chunk size (unflagged) : multiple of 8.
 *
 *	  
 *    malloc(8k + 4) -> prev_size part is used as data part in malloc chunk; later covered by prev_size when it is freed.
 *    malloc(8k) -> prev_size part is not used in malloc chunk (just padding); later covered by prev_size when freed.
 *    Both malloc case -> chunk size is 8k + 8  /  In other word, malloc(8k - 3) ~ malloc(8k + 4) -> chunk size 8k + 8	
 *
 *   - 4 byte -
 * --------------
 *|   prev_size  |		this prev_size is the part of the previous chunk.
 * --------------  ---------  start of the current chunk
 *|    size   PC |		(flagged) size (lower 2 bits are flags; P for prev_inuse (is previous chunk malloc chunk?), C for current_inuse or inuse (is current chunk malloc chunk?))
 * --------------
 *|      fd      |		forward free chunk pointer (only in FREE chunk) (in malloc chunk, user data part)
 * --------------
 *|      bk      |		backward free chunk pointer (only in FREE chunk) (in malloc chunk, user data part)
 * --------------
 *|     ....     |
 * --------------
 *|   prev_size  |		(not flagged) prev_size / same value with UNFLAG(size) / previous size for the next chunk / only in FREE chunk (in malloc chunk, user data part or padding)
 * --------------  ---------- end of the current chunk
 *

 */


static void * free_list_head;
static char * mm_brk;
static char * mm_start_brk;
static void * last_chunk;

/* set the P flag of the next chunk to state. If no valid next chunk, for example, when the given chunk is the last chunk in the heap, do nothing.*/
void adjust_next(void * chunkptr, int state)
{
	void * next_chunk;
	if (GET_C(chunkptr) != state)
	{
		fprintf(stderr, "adjust_next : adjusting info and the actual state are different\n");
		exit(1);
	}
	
	next_chunk = NEXTCHK(chunkptr);
	
	if(next_chunk < mm_brk) 
		SET_P(next_chunk, state);
}
void * search(unsigned int newsize)
{
	void * cursor;

	for (cursor = free_list_head; cursor != NULL; cursor = BKFREECHK(cursor))
	{
		if (GET_SIZE(cursor) >= newsize)
			return cursor;
	}

	return NULL;
}

/* unlink(remove from the list) the given free chunk from free list.*/
void mm_unlink(void * chunkptr)
{
	void * temp_fd;
	void * temp_bk;

	if (GET_C(chunkptr) == 1)
	{
		fprintf(stderr, "mm_unlink : tried to unlink the malloc chunk.\n");
		exit(1);
	}

	temp_fd = FDFREECHK(chunkptr);
	temp_bk = BKFREECHK(chunkptr);

	if (temp_fd == NULL)
	{
		if (free_list_head != chunkptr)
		{
			fprintf(stderr, "mm_unlink : free list is weird\n");
			exit(1);
		}

		free_list_head = temp_bk;
	}
	else
	{
		SET_BK(temp_fd, temp_bk);
	}

	if (temp_bk != NULL)
	{
		SET_FD(temp_bk, temp_fd);
	}	
}

/* insert the given chunk in the head of the free list. the given chunk should be a free chunk. */
void insert(void * chunkptr)
{
	if (GET_C(chunkptr) == 1)
	{
		fprintf(stdout, "insert : tried to insert malloc chunk in the free list\n");
		exit(1);
	}
      //fprintf(stdout, "test4 passed\n");
	SET_FD(chunkptr, NULL);
	SET_BK(chunkptr, free_list_head);
      //fprintf(stdout, "test5 passed\n");
	if (free_list_head != NULL)
		SET_FD(free_list_head, chunkptr);
	free_list_head = chunkptr;
}


/* assumption : 
 * 1. given chunk has a valid size set, has a correct prev_inuse flag, has a inuse flag 0.
 * 2. given chunk should not a malloc chunk. Only free chunk / (free chunk + newly made space) / newly made space are allowed.
 * 3. IF the given chunk was a free chunk, it should have been unlinked in caller side.
 * 4. given chunk should have a size >= newsize
 * 
 * 
 * jobs :
 * 1. when given chunk is unlinked free chunk
 *     1.1. size of the chunk >= newsize + 16 -> devide it into two free chunks, set the first chunk in malloc state, and insert the rest chunk in the free list.
 *     1.2. newsize <= size of the chunk < newsize + 16 -> no devide, set the given chunk in malloc state. 
 * 2. when given chunk contains newly made heap space (made by mem_sbrk)
 *     unflagged size == newsize, just set the given chunk in malloc state.
 * 
 */
void set_malloc_chunk(void * chunkptr, unsigned int newsize)
{
	unsigned int initsize;
	unsigned int restsize;
	void * rest_chunk;
	void * temp_fd;
	void * temp_bk;
	int temp_Pflag;

	if (GET_C(chunkptr) == 1)
	{
		fprintf(stderr, "set_malloc_chunk : assumption error, the given chunk has inuse flag 1\n");
		exit(1);
	}

	if (newsize < 16)
	{
		fprintf(stderr, "set_malloc_chunk : given size is not valid, the size should be bigger than or equal to 16, and be 8-byte aligned\n");
		exit(1);
	}

	initsize = GET_SIZE(chunkptr);

	if (initsize < newsize)
	{
		fprintf(stderr, "set_malloc_chunk : assumption error, the given chunk has smaller size 0x%x than newsize 0x%x\n", initsize, newsize);
		exit(1);
	}


	if (initsize < newsize + 16)
	{
		// do nothing
	}
	else // initsize >= newsize + 16
	{
		// given chunk can be devided into two free chunk (newsize / initsize - newsize)
		restsize = initsize - newsize;
		temp_Pflag = GET_P(chunkptr);
		SET_SIZE(chunkptr, newsize);
		SET_P(chunkptr, temp_Pflag);
		// chunkptr : C flag = 0, P flag = temp_Pflag;

		rest_chunk = NEXTCHK(chunkptr);
		SET_SIZE(rest_chunk, restsize);
		// rest_chunk : C flag = 0, P flag = 0;

		insert(rest_chunk);

            // update last_chunk if needed
            if(last_chunk == chunkptr)
                last_chunk = rest_chunk;
	}

	// set the given chunk in malloc state.
      //fprintf(stdout, "set_malloc_chunk : set chunkptr 0x%x!\n", chunkptr);
	SET_C(chunkptr, 1);
	adjust_next(chunkptr, 1);
}




/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
	free_list_head = NULL;
	mm_start_brk = (char *)mem_sbrk(4);
	mm_brk = mm_start_brk + 4;
	PUT4(mm_start_brk, 0);
      last_chunk = NULL;
      return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    void * new_chk;
    //void * last_chk;
    unsigned int newsize = ALIGN(size + 4);
    int incr;
   
    /* 
    if(mm_check() < 0)
    {
        fprintf(stderr, "mm_check : heap is not consistence\n");
        exit(1);
    }
    */
    if (newsize < 16) newsize = 16;

    //fprintf(stdout, "mm_malloc : malloc requested size = 0x%x / aligned chunk size = 0x%x / current brk : 0x%x\n", size, newsize, mm_brk);
    /* search an appropriate free chunk in the list */
    if ((new_chk = search(newsize)) != NULL)
    {
        /* If found, set it to the malloc chunk. During the setting process, the new_chk will be devided if needed. */
        mm_unlink(new_chk);
        set_malloc_chunk(new_chk, newsize);
	  return GET_DATAPTR(new_chk);
    }
    else
    {
        /* if free chunk is not found, make additional heap memory by mem_sbrk() and use the new memory space as a new malloc chunk */
	  /* if last chunk of the heap were a free chunk, we can reduce the needed additional heap memory by reusing the last free chunk with new heap memory space */
	  if (last_chunk != NULL && GET_C(last_chunk) == 0)
	  {
	  	// if the last chunk is free chunk, use it. / no need to update last_chunk.
            //fprintf(stdout, "mm_malloc_test : current last chunk = 0x%x, C flag = %d, mm_brk : 0x%x\n", last_chunk, GET_C(last_chunk), mm_brk);
		incr = newsize - GET_SIZE(last_chunk);
		if (incr <= 0)
		{
                fprintf(stdout, "mm_malloc : skipped using free chunk even though it has enough free chunk\n");
		    exit(1);
		}
	  	mm_unlink(last_chunk);
    		mem_sbrk(incr);
		mm_brk += incr;
		SET_SIZE(last_chunk, newsize);
		SET_P(last_chunk, 1);
		set_malloc_chunk(last_chunk, newsize);
    	  }
        else
        {
            // if the last chunk is malloc chunk (or if the last chunk does not exist; initial allocation), we cannot use it.
            // just get whole memory space by mem_sbrk() and use the space as a new malloc chunk.

		new_chk = mem_sbrk(newsize);
		mm_brk += newsize;
		SET_SIZE(new_chk, newsize);
		SET_P(new_chk, 1);

            // update last_chunk
            last_chunk = new_chk;

		set_malloc_chunk(last_chunk, newsize);
	  }

        return GET_DATAPTR(last_chunk);
    }
}


/* coalesces the current chunk(chunkptr) with the adjacent free chunks, and returns the pointer for the coalesced free chunk. 
 * Points :
 * - current chunk is not inserted in the free list yet. -> unlink(chunkptr) will not be done in this function.
 * - resulting free chunk should not be inserted in the free list. Inserting will be done by the rest part of mm_free() 
 */
void * mm_coalesce(void * chunkptr)
{
    unsigned int size = GET_SIZE(chunkptr);
    unsigned int prev_size;
    unsigned int next_size;
    void * prev_chunk;
    void * next_chunk;
    int merge_next;
    int temp_Pflag;
    
    //fprintf(stdout, "coalesce : start coalescing free chunk 0x%x / size = 0x%x / prev_size = 0x%x\n", chunkptr, size, GET_PREVSIZE(chunkptr));
    if (GET_C(chunkptr) != 0)
    {
        fprintf(stderr, "coalesce : tried to coalesce malloc chunk\n");
	  exit(1);
    }

    next_chunk = NEXTCHK(chunkptr);
    merge_next = (next_chunk < mm_brk && GET_C(next_chunk) == 0);
    //merge_next = (chunkptr < last_chunk  && GET_C(next_chunk) == 0);
    //fprintf(stdout, "test1 passed\n");

    if (GET_P(chunkptr) == 1 && !merge_next) // prev_chunk : malloc / next_chunk : malloc
    {
        //fprintf(stdout, "coalesce : No coalescing\n");
	  return chunkptr;
    }

    if (GET_P(chunkptr) == 1 && merge_next) // prev_chunk : malloc / next_chunk : free
    {
        //fprintf(stdout, "coalesce : with next chunk\n");
	  next_size = GET_SIZE(next_chunk);

	  mm_unlink(next_chunk);

	  temp_Pflag = GET_P(chunkptr);
	  SET_SIZE(chunkptr, size + next_size); 
	  SET_P(chunkptr, temp_Pflag); // be careful! prev_inuse flag should be preserved in chunkptr.
	  // P flag : temp_Pflag, C flag : 0
        
        // update last_chunk if needed
        if(last_chunk == next_chunk)
            last_chunk = chunkptr;

	  return chunkptr;
    }

    prev_chunk = PREVCHK(chunkptr); // since GET_P(chunkptr) != 1, there is always a valid prev_chunk >= mm_start_brk.
    //fprintf(stdout, "test2 passed\n");

    if (GET_P(chunkptr) == 0 && !merge_next) // prev_chunk : free / next_chunk : malloc
    {
    	  //fprintf(stdout, "coalesce : with prev chunk\n");
    	  prev_size = GET_SIZE(prev_chunk);

	  // since we did not insert the chunkptr in the free list yet, do not unlink(chunkptr).
	  mm_unlink(prev_chunk); // since we'll insert the result free chunk in the free list in mm_free(), first unlink the prev_chunk. .. seems unneeded.

	  temp_Pflag = GET_P(prev_chunk);
	  SET_SIZE(prev_chunk, prev_size + size);
	  SET_P(prev_chunk, temp_Pflag);
	  // P flag : temp_Pflag, C flag : 0

        // update last_chunk if needed
        if(last_chunk == chunkptr)
            last_chunk = prev_chunk;

	  return prev_chunk;
    }

    if (GET_P(chunkptr) == 0 && merge_next) // prev_chunk : free / next_chunk : free
    {
        //fprintf(stdout, "coalesce : with both prev chunk and next chunk / 0x%x and 0x%x\n", prev_chunk, next_chunk);
	  prev_size = GET_SIZE(prev_chunk);
	  next_size = GET_SIZE(next_chunk);

        //fprintf(stdout, "test1 : got prev_size and next_size = 0x%x, 0x%x\n", prev_size, next_size);
	  mm_unlink(prev_chunk);
	  mm_unlink(next_chunk);
        //fprintf(stdout, "test2 : unlinked prev_chunk and next_chunk\n");
	  temp_Pflag = GET_P(prev_chunk);
        SET_SIZE(prev_chunk, prev_size + size + next_size);
	  SET_P(prev_chunk, temp_Pflag);
	  // P flag : temp_Pflag, C flag : 0

        // update last_chunk if needed
        if(last_chunk == next_chunk)
            last_chunk = prev_chunk;

	  return prev_chunk;
    }
    //fprintf(stdout, "coalesce : what happened?! chunkptr : 0x%x, prev_inuse : %d, merge_next : %d\n", chunkptr, GET_P(chunkptr), merge_next);
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void * dataptr)
{
    
    void * chunkptr;
    void * resultptr;

    chunkptr = (void *)((unsigned int)dataptr - 4);
    //fprintf(stdout, "free : tried to free dataptr 0x%x / chunkptr 0x%x / current brk : 0x%x\n", dataptr, chunkptr, mm_brk);

    PUT4(FOOTERP(chunkptr), GET_SIZE(chunkptr)); // I missed this part. We should recover the prev_size of the free chunk since it was used as a data part in malloc chunk.
    SET_C(chunkptr, 0);
    adjust_next(chunkptr, 0);

    resultptr = mm_coalesce(chunkptr);
    //insert(mm_coalesce(chunkptr));
    //fprintf(stdout, "test3 passed\n");

    insert(resultptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *old_dataptr, unsigned int req_datasize)
{
    /*
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
    */

    void * old_chunkptr = (void *)((unsigned int)old_dataptr - 4);
    void * rest_freechk;
    void * new_dataptr;
    void * prev_chunk;
    void * next_chunk;
    void * rest_chunk;
    char * cursor; // cursor for copying the datas into new chunk when we cannot use memcpy() (when areas overlap)
    unsigned int req_chunksize = ALIGN(req_datasize + 4);
    unsigned int old_chunksize;
    unsigned int max_datasize;
    unsigned int rest_chunksize;
    unsigned int prev_chunksize;
    unsigned int next_chunksize;
    int temp_Pflag;
    int free_rest = 0;

    if(req_chunksize < 16)
        req_chunksize = 16;

    if(req_datasize == 0)
    {
        mm_free(old_dataptr);
        return NULL;
    }


    if(old_dataptr == NULL)
        return mm_malloc(req_datasize);


    old_chunksize = GET_SIZE(old_chunkptr);
    max_datasize = old_chunksize - 4;


    //if(req_datasize <= max_datasize)
    if(req_chunksize <= old_chunksize)
    {
        /* requested realloc size <= maximum possible data size of the old_chunk -> we don't have to newly allocate chunk and copy datas. */
        //rest_chunksize = max_datasize - req_datasize;
        rest_chunksize = old_chunksize - req_chunksize;
        if(rest_chunksize >= 16)
        {
            /* 
             * we can devide old_chunk into two chunks.
             * 1. malloc chunk whose chunk size is req_chunksize
             * 2. free chunk whose chunk size is old_chunksize - req_chunksize
             */

            /* 1. set the malloc chunk */
            temp_Pflag = GET_P(old_chunkptr);
            SET_SIZE(old_chunkptr, req_chunksize);
            SET_P(old_chunkptr, temp_Pflag);
            SET_C(old_chunkptr, 1);

            /* 2. set the free chunk */
            rest_freechk = NEXTCHK(old_chunkptr);
            SET_SIZE(rest_freechk, rest_chunksize);
            SET_P(rest_freechk, 1);
            SET_C(rest_freechk, 0);
            adjust_next(rest_freechk, 0);

            /* insert the free chunk in the free list */
            insert(rest_freechk);


            // update last_chunk if needed
            if(last_chunk == old_chunkptr)
                last_chunk = rest_freechk;
        }

        return old_dataptr;

    }
    else // old_chunk_size < req_chunk_size
    {
        /* 
         * In usual cases, when requested realloc size > maximum possible data size of the old_chunk -> we should make new malloc chunk and call memcpy() to copy datas to the new malloc chunk.
         * 
         * But, IF
         * 1. next chunk of the old_chunk is a free chunk
         * 2. the very next chunk(free) is big enough so that we can satisfy the requested realloc size
         * -> we don't have to make new malloc chunk and don't have to call memcpy by merging old_chunk and the very next free chunk.
         */
  
        /* deleted */

        /*
         * Also, IF
         * 1. previous chunk of the old_chunk is a free chunk
         * 2. the very previous chunk(free) is big enough so that we can satisfy the requested realloc size by merging the previous chunk and old_chunk
         * -> we don't have to make new malloc chunk, we only have to do the similar jobs that memcpy does by merging old_chunk and previous chunk.
         *  * we cannot call memcpy this time, since the source part and destination part can be overlapped.
         */
        
        /* 
        if(GET_P(old_chunkptr) == 0) // when prev is free
        {
            fprintf(stdout, "1 : prev is free, let's go.\n");
            prev_chunk = PREVCHK(old_chunkptr);
            prev_chunksize = GET_SIZE(prev_chunk);
            fprintf(stdout, "1 : old_chunkptr : 0x%x, prev_chunk : 0x%x, req_chunksize : 0x%x, req_datasize : 0x%x\n", old_chunkptr, prev_chunk, req_chunksize, req_datasize);
            if(prev_chunksize + old_chunksize >= req_chunksize) // when we can use previous free chunk since it is big enough.
            {
                fprintf(stdout, "2 : free chunk is big enough! let's unlink it\n");
                mm_unlink(prev_chunk);
                
                // change the size of prev_chunk, and turn it into malloc chunk
                temp_Pflag = GET_P(prev_chunk);
                
                // If prev_chunk + old_chunk is too big, we'll turn the rest unnecessary into free chunk.
                if((rest_chunksize = prev_chunksize + old_chunksize - req_chunksize) >= 16)
                {
                    fprintf(stdout, "3 : after copying, I'll use the rest as a free chunk!\n");
                    SET_ONLYSIZE(prev_chunk, req_chunksize);
                    free_rest = 1;
                }
                else
                {
                    fprintf(stdout, "3 : after copying, I'll not use the rest\n");
                    SET_ONLYSIZE(prev_chunk, prev_chunksize + old_chunksize);
                    free_rest = 0;
                }
                SET_P(prev_chunk, temp_Pflag);
                SET_C(prev_chunk, 1);

                // copy datas from old_chunkptr to prev_chunk.
                cursor = (char *)GET_DATAPTR(prev_chunk);
                fprintf(stdout, "4 : I'll copy from 0x%x to 0x%x\n", old_dataptr, cursor);
                for(int i = 0; i < req_datasize; ++i)
                {
                    *cursor = *(cursor + prev_chunksize);
                }
                fprintf(stdout, "4-1 : copying finished\n");

                // footer part of the prev_chunk, size(header) of the old_chunkptr can be changed. But I already stored them in the variables.

                // If free_rest is TRUE(1), turn the rest part into a free chunk.
                if(free_rest)
                {
                    rest_chunk = NEXTCHK(prev_chunk); // nextchk is updated
                    SET_SIZE(rest_chunk, rest_chunksize);
                    SET_C(rest_chunk, 0);
                    // P flag will set later
                    insert(rest_chunk);

                    adjust_next(rest_chunk, 0);
                    fprintf(stdout, "5 : use rest free chunk : 0x%x\n", rest_chunk);

                }

                adjust_next(prev_chunk, 1);
                fprintf(stdout, "6 : adjusted the next of prev_chunk, and finished the realloc\n");
                return GET_DATAPTR(prev_chunk);
            }


        }
        */

        /* we should make new malloc chunk */
        new_dataptr = mm_malloc(req_datasize);
        memcpy(new_dataptr, old_dataptr, max_datasize);
        mm_free(old_dataptr);
        return new_dataptr;
        
    }

}



/*
 * mm_check - Heap Consistency Checker
 */
int mm_check(){
    int check_first_free = 1;
    int check_last_free = 0;
    int implicit_free = 0;
    int explicit_free = 0;

    int size, P, C, fd, bk, is_start, is_end;
    void * prev;
    void * next;
    void * chk_ptr = mm_start_brk;
    // iterate memory like implicit free list,
    // checking each chunks for consistency with prev & next
    fprintf(stdout, "mm_check : let's check!\n");
    while(chk_ptr < mm_brk){
        size = GET_SIZE(chk_ptr);
        P = GET_P(chk_ptr);
        C = GET_C(chk_ptr);
        prev = PREVCHK(chk_ptr);
        next = NEXTCHK(chk_ptr);
        is_start = (mm_start_brk + 4 == chk_ptr);
        is_end = (next == mm_brk);
        // 1. check if prev's next is current
        if( (!is_start) && NEXTCHK(prev) != chk_ptr) return -1;
        // 2. check if next's prev is current
        if( (!is_end) && PREVCHK(next) != chk_ptr) return -2;
        // 3. check consistency of flag P
        if( P && !GET_C(prev)) return -6;

        // 4. if current is free chunk
        if(!C){
            implicit_free += 1;
            // 1) check if prev or next is not free
            if( (!is_start && !GET_C(prev)) || (!is_end && !GET_C(next)) ) return -3;
            
            // 2) check size == prevsize
            if( size != GET_PREVSIZE(next)) return -5;
        }

        // F. update chk_ptr
        chk_ptr = next;
    }

    // iterate memory like explicit free list
    // check each chunk for consistency of free chunk
    chk_ptr = free_list_head;
    while(chk_ptr != NULL){
        size = GET_SIZE(chk_ptr);
        P = GET_P(chk_ptr);
        C = GET_C(chk_ptr);
        fd = FDPART(chk_ptr);
        bk = FDPART(chk_ptr);
        // 1. check consistency of flag C
        if(!C) return -7;
        //  2. check first free, last free
        if(check_first_free){
            if(chk_ptr != free_list_head || fd != NULL) return -8;
            else check_first_free = 1;
        }
        if(bk == NULL){
            if(check_last_free) return -9;
            else check_last_free = 1;
        }
        // F. update chk_ptr
        chk_ptr = bk;
    }
    return 1;
    fprintf(stdout, "mm_check : finished normally\n");
}










