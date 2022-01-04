#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define N 2048 // Matrix size
#define T 2000 // Number of generations

int getNeighbors(int ** grid, int i, int j){
    int neighborLin, neighborCol, alive = 0;
    
    for(neighborLin = i - 1; neighborLin <= i + 1; neighborLin++){
        for(neighborCol = j - 1; neighborCol <= j + 1; neighborCol++){
            if(neighborLin != i || neighborCol != j){
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

    return alive;
}

int main(int argc, char * argv[]){
    int procId;
    int noProcs;
    int ** grid, ** newGrid;
    int i, j;
    int alive;
    int prev, next;
    MPI_Status status;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &noProcs);
    MPI_Comm_rank(MPI_COMM_WORLD, &procId);
    if(noProcs > 8){
        if(procId == 0)
            printf("Try less number of processes (limit is 8).\n");
        return 0;
    }
    else{
        if((noProcs % 2 != 0 && noProcs != 1) || noProcs == 6){
            if(procId == 0)
                printf("The program only works with number of processes that are power of 2 (limit is 8).\n");
            return 0;
        }
    }

    grid = (int **) malloc((N/noProcs+2) * sizeof(int *));
    newGrid = (int **) malloc(N/noProcs * sizeof(int *));
    for(i = 0; i < N/noProcs + 2; i++){
        grid[i] = (int *) calloc(N, sizeof(int));
        if(i < N/noProcs)
            newGrid[i] = (int *) calloc(N, sizeof(int));
    }

    if(procId == 0){
        //GLIDER
        int lin = 2, col = 1;
        grid[lin  ][col+1] = 1;
        grid[lin+1][col+2] = 1;
        grid[lin+2][col  ] = 1;
        grid[lin+2][col+1] = 1;
        grid[lin+2][col+2] = 1;

        //R-pentomino
        lin = 11; col = 30;
        grid[lin  ][col+1] = 1;
        grid[lin  ][col+2] = 1;
        grid[lin+1][col  ] = 1;
        grid[lin+1][col+1] = 1;
        grid[lin+2][col+1] = 1;
    }

    prev = (procId + noProcs - 1) % noProcs;
    next = (procId + 1) % noProcs;
    double time = 0.0;
    if(procId == 0)
        time -= MPI_Wtime();
    for(int g = 0; g < T; g++){
        MPI_Sendrecv(grid[1], N, MPI_INT, prev, 10,
            grid[N/noProcs+1], N, MPI_INT, next, 10, MPI_COMM_WORLD, &status);
        MPI_Sendrecv(grid[N/noProcs], N, MPI_INT, next, 20,
            grid[0], N, MPI_INT, prev, 20, MPI_COMM_WORLD, &status);
        int neighborAlive;
        for(i = 1; i <= N/noProcs; i++){
            for(j = 0; j < N; j++){
                neighborAlive = getNeighbors(grid, i, j);
                if(grid[i][j] == 0){
                    if(neighborAlive == 3)
                        newGrid[i-1][j] = 1;
                    else
                        newGrid[i-1][j] = 0;
                }
                else{
                    if(neighborAlive != 2 && neighborAlive != 3)
                        newGrid[i-1][j] = 0;
                    else
                        newGrid[i-1][j] = 1;
                }
            }
        }
        alive = 0;
        for(i = 0; i < N/noProcs; i++){
            for(j = 0; j < N; j++){
                grid[i+1][j] = newGrid[i][j];
                alive += newGrid[i][j];
            }
        }
        int totalAlive;
        MPI_Reduce(&alive, &totalAlive, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        if(procId == 0)
            printf("Geração %d: %d celulas vivas\n", g + 1, totalAlive);
    }
    if(procId == 0){
        time += MPI_Wtime();
        printf("Sessão paralela levou %lfs", time);
    }
    
    for(i = 0; i < N/noProcs + 2; i++){
        free(grid[i]);
        if(i < N/noProcs)
            free(newGrid[i]);
    }
    free(grid);
    free(newGrid);

    MPI_Finalize();

    return 0;
}