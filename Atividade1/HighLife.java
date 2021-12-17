public class HighLife{
    public static int N = 2048;
    public static int T = 2000;
    public static int noThreads = 4;
    public static int print = 0; // set PRINT to 1 to print in console the first five generations
    public static final String ANSI_RESET = "\u001B[0m";
    public static final String ANSI_RED = "\u001B[31m";
    public static final String ANSI_GREEN = "\u001B[32m";

    public static void printGrid(int [][] grid, int size){
        int i, j;
        for(i = 0; i < N; i++){
            for(j = 0; j < N; j++){
                if(grid[i][j] == 0)
                    System.out.print(ANSI_RED + grid[i][j] + ANSI_RESET + " ");
                else
                    System.out.print(ANSI_GREEN + grid[i][j] + ANSI_RESET + " ");
            }
            System.out.println();
        }
    }

    public static void main(String[] args){
        int i, j;
        int[][] grid = new int[N][N];
        for(i = 0; i < N; i++)
            for(j = 0; j < N; j++)
                grid[i][j] = 0;
        
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
        
        System.out.println("Geração 0: 10 celulas vivas");
        if(print == 1)
            printGrid(grid, 50);
        Thread[] th = new Thread[noThreads];
        RunThreadHighLife[] rt = new RunThreadHighLife[noThreads];
        int totalAlive;
        long startTime = System.currentTimeMillis();
        for(int g = 0; g < T; g++){  
            for(i = 0; i < noThreads; i++){
                rt[i] = new RunThreadHighLife(i, noThreads, grid);
                th[i] = new Thread(rt[i]);
                th[i].start();
            }
            totalAlive = 0;
            try{
                for(i = 0; i < noThreads; i++){
                    th[i].join();
                    totalAlive += rt[i].alive;
                }
            } catch (InterruptedException e){

            }
            for(i = 0; i < noThreads; i++)
                for(j = N / noThreads * i; j < N / noThreads * (i + 1); j++)
                    System.arraycopy(rt[i].newGrid[j], 0, grid[j], 0, N);
            System.out.println("Geração " + (g + 1) + ": " + totalAlive + " celulas vivas");
            if(print == 1 && g < 5)
                printGrid(grid, 50);
        }
        long calcTime = System.currentTimeMillis() - startTime;
        System.out.println("Levou " + calcTime/1000.0 + " segundos.");
    }
}

class RunThreadHighLife implements Runnable{
    private int id;
    private int N;
    private int noThreads;
    private int[][] grid;
    public int[][] newGrid;
    public int alive;

    public RunThreadHighLife(int tId, int tNoThreads, int[][] tGrid){
        id = tId;
        grid = tGrid;
        noThreads = tNoThreads;
        N = tGrid.length;
    }

    public static int getNeighbors(int [][] grid, int i, int j){
        int neighborLin, neighborCol, neighborAlive = 0, N = grid.length;

        for(neighborLin = i - 1; neighborLin <= i + 1; neighborLin++){
            for(neighborCol = j - 1; neighborCol <= j + 1; neighborCol++){
                if(neighborLin != i || neighborCol != j){
                    if(neighborLin == -1){
                        if(neighborCol == -1)
                            neighborAlive += grid[N-1][N-1];
                        else if(neighborCol == N)
                            neighborAlive += grid[N-1][0];
                        else
                            neighborAlive += grid[N-1][neighborCol];
                    }
                    else if(neighborLin == N){
                        if(neighborCol == -1)
                            neighborAlive += grid[0][N-1];
                        else if(neighborCol == N)
                            neighborAlive += grid[0][0];
                        else
                            neighborAlive += grid[0][neighborCol];
                    }
                    else{
                        if(neighborCol == -1)
                            neighborAlive += grid[neighborLin][N-1];
                        else if(neighborCol == N)
                            neighborAlive += grid[neighborLin][0];
                        else
                            neighborAlive += grid[neighborLin][neighborCol];
                    }
                }
            }
        }
        return neighborAlive;
    }

    public void run(){
        newGrid = new int[N][N];
        alive = 0;
        int i, j, neighborAlive;
        for(i = N / noThreads * id; i < N / noThreads * (id + 1); i++){
            for(j = 0;  j < N; j++){
                neighborAlive = getNeighbors(grid, i, j);
                if(grid[i][j] == 0){
                    if(neighborAlive == 3 || neighborAlive == 6)
                        newGrid[i][j] = 1;
                    else
                        newGrid[i][j] = 0;
                }
                else{
                    if(neighborAlive != 2 && neighborAlive != 3)
                        newGrid[i][j] = 0;
                    else
                        newGrid[i][j] = 1;
                }
                alive += newGrid[i][j];
            }
        }
    }
}