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
  int col_n;
  int row_n;
  int i = 0;
  int j = 0;
  int d = 0;
  int miss_save;
  int m_2;
  int m_3;
  int b_size = 8;
  int b_size_64 = 4;
  int b_size_67 = 16;

  //program arranged by converting pseudeocode/reading an academic paper created by the Georgia Tech CS department
  //URL:http://www.cc.gatech.edu/~bader/COURSES/UNM/ece637-Fall2003/papers/KW03.pdf
  if(M == 32){
    for(row_n = 0; row_n < M; row_n += b_size){
      for(col_n =0; col_n < M; col_n += b_size){
        for(i = row_n; i < row_n + b_size; i++){
          for(j = col_n; j < col_n + b_size; j++){
            if(i == j){
              d = i;
      	      //for the transpose of an even numbered matrix, the elements on the diagonal will remain in place.
      	      //store the value, to resolve the 'miss'
              miss_save = A[i][j];
            }
            else{
	             //swap the appropriate elements rows-> cols and cols -> rows
               B[j][i] = A[i][j];
             }
           }
           if(row_n == col_n){
	            //place 'miss' on the appropriate diagonal index of the output matrix B
              B[d][d] = miss_save;
            }
          }
        }
      }
    }
    else if(M == 64){
      //works identically to the base case, adjusts the block size to 4
      for(row_n = 0; row_n < M; row_n += b_size_64){
        for(col_n =0; col_n < M; col_n += b_size_64){
          for(i = row_n; i < row_n + b_size_64; i++){
            for(j = col_n; j < col_n + b_size_64; j++){
              if(i == j){
                d = i;
                m_2 = A[i][j];
              }
              else{

                B[j][i] = A[i][j];
              }
            }

            if(row_n == col_n){
              B[d][d] = m_2;
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
