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
  int blockSize;
  int blockRow, blockCol;
  int r, c; //to iterate through each block, used in inner loops
  int temp = 0, d = 0;
  int v0,v1,v2,v3,v4;
	if (N == 32)
	{
		blockSize = 8;
		for(blockCol = 0; blockCol < N; blockCol += 8)
		{
			for(blockRow = 0; blockRow < N; blockRow += 8)
			{
				for(r = blockRow; r < blockRow + 8; r++)
				{
					for(c = blockCol; c < blockCol + 8; c++)
					{
						if(r != c)
						{
							B[c][r] = A[r][c];
						}

						else
						{
						temp = A[r][c];
						d = r;
						}
					}
					if (blockRow == blockCol)
					{
						B[d][d] = temp;
					}
				}
			}
		}
	}else if (N == 64){
 		blockSize = 4;
		for(r = 0; r < N; r += blockSize)
		{
			for(c = 0; c < M; c += blockSize)
			{
				v0 = A[r][c];
				v1 = A[r+1][c];
				v2 = A[r+2][c];
				v3 = A[r+2][c+1];
				v4 = A[r+2][c+2];
				B[c+3][r] = A[r][c+3];
				B[c+3][r+1] = A[r+1][c+3];
				B[c+3][r+2] = A[r+2][c+3];
				B[c+2][r] = A[r][c+2];
				B[c+2][r+1] = A[r+1][c+2];
				B[c+2][r+2] = v4;
				v4 = A[r+1][c+1];
				B[c+1][r] = A[r][c+1];
				B[c+1][r+1] = v4;
				B[c+1][r+2] = v3;
				B[c][r] = v0;
				B[c][r+1] = v1;
				B[c][r+2] = v2;
				B[c][r+3] = A[r+3][c];
				B[c+1][r+3] = A[r+3][c+1];
				B[c+2][r+3] = A[r+3][c+2];
				v0 = A[r+3][c+3];
				B[c+3][r+3] = v0;
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
