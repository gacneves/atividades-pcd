#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>
#include <sys/time.h>
#define N 2048
#define T 2000
#define COLRED "\x1B[31m"
#define COLGRN "\x1B[32m"
#define COLWHT "\x1B[37m"

int getNeighbors(int ** grid, int i, int j){
    int neighborLin, neighborCol, alive = 0;
    
    for(neighborLin = i - 1; neighborLin <= i + 1; neighborLin++){
        for(neighborCol = j - 1; neighborCol <= j + 1; neighborCol++){
            if(neighborLin != i || neighborCol != j){
                switch(neighborLin){
                    case -1:
                        switch(neighborCol){
                            case -1:
                                alive += grid[N-1][N-1];
                                break;
                            case N:
                                alive += grid[N-1][0];
                                break;
                            default:
                                alive += grid[N-1][neighborCol];
                        }
                        break;
                    case N:
                    switch(neighborCol){
                            case -1:
                                alive += grid[0][N-1];
                                break;
                            case N:
                                alive += grid[0][0];
                                break;
                            default:
                                alive += grid[0][neighborCol];
                        }
                        break;
                    default:
                        switch(neighborCol){
                            case -1:
                                alive += grid[neighborLin][N-1];
                                break;
                            case N:
                                alive += grid[neighborLin][0];
                                break;
                            default:
                                alive += grid[neighborLin][neighborCol];
                        }
                }
            }
        }
    }

    return alive;
}

void printGrid(int ** grid, int size){
    for(int i = 0; i < size; i++){
        for(int j = 0; j < size; j++){
            if(grid[i][j] == 0)
                printf("%s%d ", COLRED, grid[i][j]);
            else
                printf("%s%d ", COLGRN, grid[i][j]);
        }
        printf("\n");
    }
}

int main(){
    int i, j, ** grid, ** newGrid;
    grid = malloc(N * sizeof(int *));
    newGrid = malloc(N * sizeof(int *));
    for(i = 0; i < N; i++){
        grid[i] = calloc(N, sizeof(int));
        newGrid[i] = calloc(N, sizeof(int));
    }

    //GLIDER
    int lin = 1, col = 1;
    grid[lin  ][col+1] = 1;
    grid[lin+1][col+2] = 1;
    grid[lin+2][col  ] = 1;
    grid[lin+2][col+1] = 1;
    grid[lin+2][col+2] = 1;

    //R-pentomino
    lin = 10; col = 30;
    grid[lin  ][col+1] = 1;
    grid[lin  ][col+2] = 1;
    grid[lin+1][col  ] = 1;
    grid[lin+1][col+1] = 1;
    grid[lin+2][col+1] = 1;

    double start, end;

    omp_set_num_threads(8);
    for(int g = 0; g < T; g++){
        int neighborAlive, local;
        #pragma omp parallel for private(i,j,neighborAlive)
        for(i = 0; i < N; i++){
            for(j = 0;  j < N; j++){
                neighborAlive = getNeighbors(grid, i, j);
                if(grid[i][j] == 0){
                    if(neighborAlive == 3){
                        newGrid[i][j] = 1;
                    }
                    else
                        newGrid[i][j] = 0;
                }
                else{
                    if(neighborAlive != 2 && neighborAlive != 3)
                        newGrid[i][j] = 0;
                    else{
                        newGrid[i][j] = 1;
                    }
                }
            }
        }
        for(i = 0; i < N; i++)
            for(j = 0;  j < N; j++){
                grid[i][j] = newGrid[i][j];
            }
        printf("%sGeração %d\n", COLWHT, g + 1);
    }

    int alive = 0;
    omp_set_num_threads(1);
    start = omp_get_wtime();
    #pragma omp parallel for private(i,j) reduction(+:alive)
    for(i = 0; i < N; i++){
        for(j = 0;  j < N; j++){
            alive += grid[i][j];
        }
    }
    end = omp_get_wtime();
    printf("Took %f seconds.\n", end-start);

    for(i = 0; i < N; i++){
        free(grid[i]);
        free(newGrid[i]);
    }
    free(grid);
    free(newGrid);
    return 0;
}