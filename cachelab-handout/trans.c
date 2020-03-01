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
  int rowCount;
	int colCount;
	int rowSector;
	int columnSector;
	//This is to prevent cache misses along the diagonal
	int diagonal = 0;
	//Variable that takes advantage of temporal locality (i.e. keeping something inside a cahce to prevent misses
	int temporal = 0;
	//For the 64x64 matrix case
	//We use the same looping structure for 64x64 or 32x32 matrices, only this that changes is size of the sector (based on blocks' proximity to diagonal line)
	//Also, thought there seems to be duplication between the loops, it is due to the general avoidance of local variables
	if(N == 32 && M == 32){
			//We can block 8 at a time, optimal size since its a cache at a time
			for(columnSector = 0; columnSector < N; columnSector += 8){
				for(rowSector = 0; rowSector < N; rowSector += 8){
					//Now we cycle within one block, so we loop sector at a time
					for(rowCount = rowSector; rowCount < rowSector + 8; rowCount++){
						for(colCount = columnSector; colCount < columnSector + 8; colCount++){
							//As long as we are not on the diagonal, we put the value
							//from A[row][column] into B[column][row], the heart of transposing
							if(rowCount != colCount){
								B[colCount][rowCount] = A[rowCount][colCount];

							}
							else{
								//The digonal variable keeps to data in a cache utilizing spatial locality
								diagonal = rowCount;
								//Temporal uses...temporal locality
								temporal = A[rowCount][colCount];
							}
						}
						//In a square matrix, if the sector we are blocking off contains
						//the diagonal, then we keep those elements the same.
						//Doint it this way reduces the amount of misses
						if(columnSector == rowSector){
							B[diagonal][diagonal] = temporal;
						}//end if statement
					}//end rowCount for loop
				}//end rowSector for loop
			}//end columnSector for loop
		}//end N == 32 If statement
        else if(N == 64){
		//We can block 8 at a time, optimal size since its a byte squared at a time
			for(columnSector = 0; columnSector < M; columnSector += 4){
				for(rowSector = 0; rowSector < N; rowSector += 4){
					for(rowCount = rowSector; rowCount < rowSector + 4 ; rowCount++){
						for(colCount = columnSector; colCount < columnSector + 4 ; colCount++){
							//As long as we are not on the diagonal, we put the value
							//from A[row][column] into B[column][row], the heart of transposing
							if(rowCount != colCount){
								B[colCount][rowCount] = A[rowCount][colCount];
							}
							else{
								//The digonal variable keeps to data in a cache utilizing spatial locality
								diagonal = rowCount;
								//Temporal uses...temporal locality
								temporal = A[rowCount][colCount];
							}
						}
						//In a square matrix, if the sector we are blocking off contains
						//the diagonal, then we keep those elements the same.
						//Doint it this way reduces the amount of misses
						if(columnSector == rowSector){
							B[diagonal][diagonal] = temporal;
						}//end if statement
					}//end rowCount for loop
			}//end rowSector for loop
		}//end columnSector for loop
	}//end N == 64 If statement
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
