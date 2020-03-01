//Louis Ye 1999922

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

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

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

    //create values for i, j, row, and col
    int i, j, row, col;
    //have value for diagonal
    int diagonal = 0;
    //create a temp value so that you don't have to uselessly store
    int tmp = 0;
    if(N == 32){
         //focus on switching/reversing the column in A
        for(col=0; col < N; col+=8){

            //focus on switching/reversing rows in A
            for(row=0;row < N; row +=8){

                //iterate through these two for loops so that the now diagonal rows and columns can be
                //systematically transposed onto B.
                for(i = row; i < row + 8; i++){
                    for(j = col; j < col + 8; j++){
                        //by adding this if statement we can avoid switching the diagonals which don't
                        //need switching
                        if(i != j){
                            B[j][i] = A[i][j];
                        }
                        //since diagonal we have to treat it differently otherwise will result in
                        //unecessary cache miss since the square-type matrix diagonals don't move
                        //the position can easily be remembered.
                        else{
                            tmp = A[i][j];
                            diagonal = i;
                        }
                    }
                    //using this if statement we can transpose the A values into the B smoothly
                    if(row == col){
                        B[diagonal][diagonal] = tmp;
                    }
                }
            }
        }

    }

    else if(N == 64){
         //focus on switching/reversing the column in A
        for(col=0; col < N; col+=4){
            //focus on switching/reversing rows in A
            for(row=0;row < N; row +=4){
                //iterate through these two for loops so that the now diagonal rows and columns can be
                //systematically transposed onto B.
                for(i = row; i < row + 4; i++){
                    for(j = col; j < col + 4; j++){
                        //by adding this if statement we can avoid switching the diagonals which don't
                        //need switching
                        if(i != j){
                            B[j][i] = A[i][j];
                        }
                        //since diagonal we have to treat it differently otherwise will result in
                        //unecessary cache miss since the square-type matrix diagonals don't move
                        //the position can easily be remembered.
                        else{
                            tmp = A[i][j];
                            diagonal = i;
                        }
                    }
                    //using this if statement we can transpose the A values into the B smoothly
                    if(row == col){
                        B[diagonal][diagonal] = tmp;
                    }
                }
            }
        }
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
