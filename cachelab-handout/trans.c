/* 
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

#define row_step_32by32 (8)
#define col_step_32by32 (8)

#define row_step_64by64 (4)
#define col_step_64by64 (4)

#define min(a,b) ((a<b)?a:b)

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
void transpose32by32(int M, int N, int A[N][M], int B[M][N]);


char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    
    if((M == 32 && N == 32)||(M == 61 && N == 67)){
        for(int col_offset = 0;col_offset < N;col_offset += 8){
            for(int row_offset = 0;row_offset < M;row_offset += 8){
                int x[8] = {0};
                
                for(int col = 0;col < min(8,N - col_offset);++col){
                    int row;
                    for(row = 0;row < min(8,M - row_offset);++row){
                        x[row] = A[col_offset + col][row_offset + row];
                    }
                    for(row = 0;row < min(8,M - row_offset);++row){
                        B[row_offset + row][col_offset + col] = x[row];
                    }
                }  
            }
        }
    }
    else if(M == 64 && N == 64){
        for(int col_offset = 0;col_offset < N;col_offset += 8){
            for(int row_offset = 0;row_offset < M;row_offset += 8){
                int row,col;
                int x[8] = {0};
                for(int col = 0;col < 4;++col){
                    for(int row = 0;row < 8;++row){
                        x[row] = A[col_offset+col][row_offset+row];
                    }
                    for(int row = 0;row < 8;++row){
                        B[row_offset+row+4*(row < 4)][col_offset+col+4*(row < 4)] = x[row];
                    }
                }
                
                for(row = 4;row < 8; ++row){
                    for(col = 4;col < 8; ++col){
                        x[col-4] = B[row_offset+row][col_offset+col];
                        B[row_offset + row][col_offset+col] = A[col_offset+col][row_offset+row];
                    }
                    for(col = 4;col < 8; ++col){
                        B[row_offset + row - 4][col_offset+col - 4] = x[col - 4];
                    }
                }
                for(int col = 4;col < 8;++col){
                    for(int row = 0;row < 4;++row){
                        x[row] = A[col_offset+col][row_offset+row];
                    }
                    for(int row = 0;row < 4;++row){
                        B[row_offset+row][col_offset+col] = x[row];
                    }
                }
                
            }
        }
    }

}


// void transpose32by32(int M, int N, int (*A)[M], int (*B)[N]){
//     int row,col;
//     for(int col_offset = 0;col_offset < M;col_offset += col_step_32by32){
//         for(int row_offset = 0;row_offset < N;row_offset += row_step_32by32){
//             if(row_offset != col_offset){
//                 for(col = 0;col< col_step_32by32;++col){
//                     for(row = 0;row< row_step_32by32;++row){
//                         B[row_offset + row][col_offset + col] = A[col_offset + col][row_offset + row];            
//                     }
//                 }
//             }
//             else{
//                 int x[8] = {0};
//                 for(col = 0;col < col_step_32by32;++col){
//                     for(row = 0;row < row_step_32by32;++row){
//                         x[row] = A[col_offset + col][row_offset + row];
//                     }
//                     for(row = 0;row < row_step_32by32;++row){
//                         B[row_offset + row][col_offset + col] = x[row];
//                     }
//                 }
//             }
//             //printf("%d %d",A[0][1],B[1][0]);
//         }
//     }
// }

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

