/* 
 * 20160759 Dongpyeong Seo
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
void trans_block_32X32(int A[32][32], int B[32][32]);
void trans_block_64X64(int A[64][64], int B[64][64]);
void trans_block_61X67(int A[67][61], int B[61][67]);
void trans_block(int M, int N, int A[N][M], int B[M][N]);
/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if(M == 32 && N == 32){
        trans_block_32X32(A, B);
    }
    else if(M == 64 && N == 64){
        trans_block_64X64(A, B);
    }
    else if(M == 61 && N == 67){
        trans_block_61X67(A, B);
    }
    else{
        trans_block(M, N, A, B);
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

}

char trans_block_desc[] = "Simple trans using 8X8 blocking";
void trans_block(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, i0, j0, diag;
    int b = 8;

    // Block iteration : (row-iteration first, then column-iteration) to use more cache line datas before they replaced. 
    for (j = 0; j < M; j += b)
    {
        for (i = 0; i < N; i += b)
        {
            // blocking of block size b(= 8)
            for (i0 = i; (i0 < N && i0 < i + b); ++i0)
            {
                for (j0 = j; (j0 < M && j0 < j + b); ++j0)
                {
                    if (j0 == i0) // in the case of diagonal entry
                    {
                        diag = A[i0][j0];
                        continue;
                    }
                    B[j0][i0] = A[i0][j0];
                }
                
                if (i == j) // It will be done only in the case of diagonal bXb block
                {
                    B[i0][i0] = diag; 
                }
            }
        }
    }
}

char trans_block_61X67_desc[] = "Simple trans using 16X16 blocking";
void trans_block_61X67(int A[67][61], int B[61][67])
{
    // It only used blocking strategy of block size 16.
    int i, j, i0, j0;
    int b = 16; // I choosed this block size since it showed the better result in 61X67 case than when I tried ford block size 8.
    for(i = 0; i < 67; i += b)
    {
        for(j = 0; j < 61; j += b)
        {
            for(i0 = i; (i0 < 67 && i0 < i + b); ++i0)
            {
                for(j0 = j; (j0 < 61 && j0 < j + b); ++j0)
                {
                    B[j0][i0] = A[i0][j0];
                }
            }
        }
    }
}

char trans_block_32X32_desc[] = "Simple trans using blocking and several temporal variables";
void trans_block_32X32(int A[32][32], int B[32][32])
{
    int i, j, i0, j0;
    
    // blocking strategy : chosen block size = 8 byte (2*8*8 < C/4 (= 1kB/4 = 256))
    
    // In ahead, we should treat the case 8X8 small blocks located in diagonal entry, let's call them 'diagonal blocks'
    for(i = 0; i < 32; i += 8)
    {
        
        // 8X8 block (1,1), (3,3) : temporary store the transposed result to the block on the left
        if((i / 8) % 2)
        { 
            // store in the temporal block
            for(i0 = i; (i0 < i + 8 && i0 < 32); ++i0)
            {
                for(j0 = i; (j0 < i + 8 && j0 < 32); ++j0)
                {
                    B[j0][i0-8] = A[i0][j0];
                }
            }

            // move to the original target block from temporal block
            for(j0 = i; (j0 < i + 8 && j0 < 32); ++j0)
            {
                for(i0 = i; (i0 < i + 8 && i0 < 32); ++i0)
                {
                    B[j0][i0] = B[j0][i0-8];
                }
            }
        }

        
        // 8X8 block (0,0), (2,2) : temporary store the transposed result to the block on the right.
        else 
        { 
            // store in the temporal block
            for(i0 = i; (i0 < i + 8 && i0 < 32); ++i0)
            {
                for(j0 = i; (j0 < i + 8 && j0 < 32); ++j0)
                {
                    B[j0][i0+8] = A[i0][j0];
                }
            }

            // move to the original target block from temporal block
            for(j0 = i; (j0 < i + 8 && j0 < 32); ++j0)
            {
                for(i0 = i; (i0 < i + 8 && i0 < 32); ++i0)
                {
                    B[j0][i0] = B[j0][i0+8];
                }
            }
        }
    }

    // for rest cases (except diagonal blocks), just do the ordinary transpose method since they do not make a compulsory miss
    for(i = 0; i < 32; i += 8)
    {
        for(j = 0; j < 32; j += 8)
        {
            if(i != j)
            {
                for(i0 = i; (i0 < i + 8 && i0 < 32); ++i0)
                {
                    for(j0 = j; (j0 < j + 8 && j0 < 32); ++j0)
                    {
                        B[j0][i0] = A[i0][j0];
                    }
                }
            }
        }
    }
}

char trans_block_64X64_desc[] = "64X64 trans";
void trans_block_64X64(int A[64][64], int B[64][64])
{
    int i, j, i0, j0, temp1, temp2, temp3, temp4, diag; // I can declare 10 local variables. 9/10

    /* 
     * This method uses blocking strategy of block size 8, 
     * suspending strategy of allocation of diagonal entries, 
     * and using 4 local variables for storage of some data to reduce of waste of cache data.
     *
     * Following notation were used.
     * block(i,j) -> like A[8 * i ~ 8 * i + 7][8 * j ~ 8 * j + 7]
     *
     * When treating one 8x8 block, the very 8x8 block is divided into four 4x4 blocks as follows,
     *
     *      LU|RU (left upper, right upper)
     *      -----
     *      LL|RL (left lower, right lower)
     * 
     * Big contextual flow is as follows.
     * 1. If target 8X8 block is diagonal block(i,i), suspending strategy of allocation of diagonal entries is used.
     * 2. If target 8X8 block is non-diagonal block(i,j) (i != j), the method treats four 4X4 blocks separately and uses a lot of variables.
     */
    
    // 8X8 block iteration in 64X64 original matrix is done by (column iteration first, then row iteration).
    for(i = 0; i < 64; i += 8)
    {
        for(j = 0; j < 64; j += 8)
        {
            // case 1 : non-diagonal 8X8 blocks
            if(i != j)
            {
                // LU
                // left upper (LU) 4X4 block of A : just transpose it and store the result in the LU 4X4 block of B.
                for(i0 = i; i0 < i + 4; ++i0)
                {
                    for(j0 = j; j0 < j + 4; ++j0)
                    {
                        B[j0][i0] = A[i0][j0];
                    }
                }

                // copy 'transpose' of RU block of A(transpose(A-RU)) into RU block of B, in order to use the datas of RU block of A in cache, instead of wasting them.
                for(i0 = i; i0 < i + 4; ++i0)
                {
                    for(j0 = j + 4; j0 < j + 8; ++j0)
                    {
                        B[j0 - 4][i0 + 4] = A[i0][j0];
                    }
                }

                // LL & RR 
                // transpose A-LL to B-RU, and at the same time, move the 'already transposed' A-RU datas, which are stored in the B-RU, 'line by line'!
                //
                // This really reduces the waste of cache datas so that it reduces compulsory misses.
                for(j0 = j; j0 < j + 4; ++j0)
                {
                    temp1 = B[j0][i + 4];
                    temp2 = B[j0][i + 5];
                    temp3 = B[j0][i + 6];
                    temp4 = B[j0][i + 7];
                    for(i0 = i + 4; i0 < i + 8; ++i0)
                    {
                        B[j0][i0] = A[i0][j0];
                    }
                    B[j0 + 4][i] = temp1;
                    B[j0 + 4][i + 1] = temp2;
                    B[j0 + 4][i + 2] = temp3;
                    B[j0 + 4][i + 3] = temp4;
                }
                
                // RL
                // right lower (RL) 4X4 block of A : just transpose it and store the result in the right lower 4X4 block of B.
                for(i0 = i + 4; i0 < i + 8; ++i0)
                {
                    for(j0 = j + 4; j0 < j + 8; ++j0)
                    {
                        B[j0][i0] = A[i0][j0];
                    }
                }
            }

            // case 2 : diagonal 8X8 blocks
            else
            {
                // To reduce the compulsory miss when we transpose x-th row of 8X8 block of A to x-th column of 8X8 block of B, 
                // I will temporary save the (x,x) element of block in 'diag' variable while I iterate the x-th row, 
                // and after iteration finished, then move the saved diagonal entry to the original target location.
                // 
                // Just suspending the allocation! 
                
                for(temp2 = j; temp2 < j + 8; temp2 += 4)
                {
                    for(temp1 = i; temp1 < i + 8; temp1 += 4)
                    {
                        for (i0 = temp1; i0 < temp1 + 4; ++i0)
                        {
                            for (j0 = temp2; j0 < temp2 + 4; ++j0)
                            {
                                if (j0 == i0) // in the case of diagonal entry
                                {
                                    diag = A[i0][j0];
                                    continue;
                                }
                                B[j0][i0] = A[i0][j0];
                            }
                            if(temp1 == temp2) // only the diagonal 4X4 block has diagonal entries.
                            {
                                B[i0][i0] = diag; 
                            }
                        }
                    }
                }
            }
        }
    }


}


// please ignore it, it's just for archiving.
/*
void storage(int A[64][64], int B[64][64])
{
    int i, j, temp1, temp2, temp3, temp4, temp5, temp6, temp7, itr;

    // 8X8 block iteration in 64X64 original matrix is done by (column iteration first, then row iteration).
    for(i = 0; i < 64; i += 8)
    {
        for(j = 0; j < 64; j += 8)
        {
            // Left Upper 4X4 block in 8X8 block
            // Just transpose with pending a diagonal entry
            for(itr = 0; itr < 16; ++itr)
            {
                // element iteration in the 4X4 block is done by (column iteration first, then row iteration).
                // row index : i + itr / 4
                // column index : j + itr % 4
                if(i + itr / 4 != j + itr % 4)
                {
                    B[j + itr % 4][i + itr / 4] = A[i + itr / 4][j + itr % 4]; // B[col][row] = A[row][col];
                }
                else
                {
                    // In the case of diagonal entry, pending an allocation is done in order to prevent unnecessary compulsory misses.
                    temp7 = A[i + itr / 4][j + itr % 4]; // temp7 = A[row][col] (row == col)
                }
                
                if(j + itr % 4 == 3) // one column iteration finished & pended allocation of diagonal entry will be done.
                {
                    B[i + itr / 4][i + itr / 4] = temp7;
                }
            }

            // Left Lower 4X4 block
            
            // Before transpose it, copy the Right Upper block of A into Right Upper block of B(A-RU -> B-RU)
            // in order to use the datas about Right Upper block of A in cache, instead of wasting them.
            for(itr = 0; itr < 16; ++itr)
            {

            } 

            // First, among the choosed Left Lower (4X4) part, 
            // transpose the Left Upper 2X2 part,
            

        }
    }

}
*/




/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 
    registerTransFunction(trans_block, trans_block_desc);
}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

