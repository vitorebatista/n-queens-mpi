#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <ctype.h>
#include <string.h>
#include "util.c"

/*
Itens do MPI_Status:
    int MPI_SOURCE;
    int MPI_TAG;
    int MPI_ERROR;
} MPI_Status;
*/

#define MPI_MATER 0 // Zero para o mestre MPI
#define TAG_INIT 1 // Message to client:  size and [0]
#define TAG_DATA_INT 2 // Message from client with results
#define TAG_DATA_CHAR 3 // Message from client with results
#define TAG_EXIT 4 // Message from client with CPU time \
               // Also to client, giving permission to exit

// Server process:  send jobs to client compute engines and
// receives results back.
void StartQueens(int size, double *clientTime)
{
    int col, k,
        commBuffer[2],          // Communication buffer -- size, [0]
        Count[3],               // Counts back from the clients
        limit = (size + 1) / 2, // Mirror images done automatically
        nProc,                  // size of the communicator
        proc,                   // For loop [1..nProc-1] within initial message
        nActive;                // Number of active processes
    int fsize;
    char char_info[5000];
    char file_name[14];   
    FILE *file_result; 
    MPI_Status Status;

    puts("Server process has entered StartQueens");

    MPI_Comm_size(MPI_COMM_WORLD, &nProc);

    commBuffer[0] = size;
    // Send initial configurations to all client processes --- or to those
    // needed in case not all are required.
    for (col = 0, proc = 1; proc < nProc && col < limit; proc++, col++)
    {
        printf("\n\n Processo[%d] col=%d\n", proc, col);
        commBuffer[1] = col;

        printf("Enviado para o escravo %d o trabalho de %d,%d\n", proc, commBuffer[0], commBuffer[1]);
        MPI_Send(commBuffer, 2, MPI_INT, proc, TAG_INIT, MPI_COMM_WORLD);
    }
    nActive = proc - 1; // Since rank==0 is not used
    if (proc < nProc)   // More processes than jobs
    {
        int dmy[2] = {0, 0}; // Termination message to unused processes
        while (proc < nProc)
            MPI_Send(dmy, 2, MPI_INT, proc++, TAG_INIT, MPI_COMM_WORLD);
    }
    puts("Aguardando novos problemas");
    // Receive back results and send out new problems
    while (col < limit)
    {
        MPI_Recv(Count, 3, MPI_INT, MPI_ANY_SOURCE, TAG_DATA_INT, MPI_COMM_WORLD, &Status);
        proc = Status.MPI_SOURCE;
        printf("Resultados recebidos do escravo %d (%d, %d)\n", proc, Count[0], Count[1]);
        total_unique += Count[0];
        total_all += Count[1];
        fsize = Count[2];
        commBuffer[1] = col++;
        printf("\n\n\ntamanho do arquivo1 = %d\n\n\n", fsize);
        MPI_Recv(char_info, fsize+1, MPI_CHAR, MPI_ANY_SOURCE, TAG_DATA_CHAR, MPI_COMM_WORLD, &Status);
        printf("\n\nstring1: %s\n", char_info);
        printf("Enviando para o escravo %d os parâmetros %d,%d\n", proc, commBuffer[0], commBuffer[1]);
        MPI_Send(commBuffer, 2, MPI_INT, proc, TAG_INIT, MPI_COMM_WORLD);
    }
    // Finally, receive back pending results and send termination
    // indication (message with size of zero).
    commBuffer[0] = 0;
    while (nActive > 0)
    {
        printf("%d pending\n", nActive);
        MPI_Recv(Count, 3, MPI_INT, MPI_ANY_SOURCE, TAG_DATA_INT, MPI_COMM_WORLD, &Status);
        --nActive;
        proc = Status.MPI_SOURCE;
        printf("Received results from client %d (%d, %d)\n", proc, Count[0], Count[1]);
        total_unique += Count[0];
        total_all += Count[1];
        fsize = Count[2];
        commBuffer[1] = col++;
        printf("\n\n\ntamanho do arquivo2 = %d\n\n\n", fsize);
        MPI_Recv(char_info, fsize+1 , MPI_CHAR, MPI_ANY_SOURCE, TAG_DATA_CHAR, MPI_COMM_WORLD, &Status);
        printf("\n\nstring2: %s \n", char_info);
        
        printf("Enviando para o escravo %d msg de termino\n", proc);
        MPI_Send(commBuffer, 2, MPI_INT, proc, TAG_INIT, MPI_COMM_WORLD);
        
        
        snprintf(file_name, 24, "solution%d.txt", size);
        file_result = fopen(file_name, "a"); //somente leitura
        fprintf(file_result, "%d;", char_info);
        fclose(file_result);
    }
    for (proc = 1; proc < nProc; proc++)
    {
        MPI_Send(&proc, 0, MPI_INT, proc, TAG_EXIT, MPI_COMM_WORLD);
        printf("Enviando EXIT para %d\n", proc);
    }
    puts("Terminando StartQueens.");
}

// Prototype for forward referencing
void Nqueens(int *, int *, int, int, int);

// Client processes receive problems to process from the
// server and then return their results to the server.
void ProcessQueens(int myPos)
{
    int nCells = 0, size, k, col, buffer[2];
    FILE *file_result;
    int *board = NULL, *trial = NULL; // Allow for realloc use
    MPI_Status Status;

    printf("Escravo %d começou .\n", myPos);
    MPI_Recv(buffer, 2, MPI_INT, 0, TAG_INIT, MPI_COMM_WORLD, &Status);
    printf("Escravo %d recebeu os parâmetros: %d and %d\n", myPos, buffer[0], buffer[1]);
    //fflush(stdout);
    size = buffer[0];
    col = buffer[1];
    // As long as a valid problem is in hand, do the processing.
    // The server sends a size of zero as a termination message
    while (size > 0)
    {
        int int_info[3];

        // Generate the arrays
        if (size > nCells)
        {
            int idx;

            board = (int *)calloc(size, sizeof *board);
            trial = (int *)calloc(size * 2, sizeof *trial);
            // Aloca vetor bool para avaliar a diagonal e anti diagonal
            // Calloc irá já inicializar com FALSE (0)
            Diag = (short *)calloc(2 * (size - 1), sizeof *Diag);
            AntiD = (short *)calloc(2 * (size - 1), sizeof *AntiD);
            for (idx = 0; idx < size; idx++)
                board[idx] = idx;

            nCells = size;
        }
        // Zera os contadores
        total_unique = 0,
        total_all = 0;
        swap(int, board[0], board[col]);
        // CRITICAL:  mark [0] as used, and then as unused
        Mark(0, board[0], size, Diag, AntiD, TRUE);
        
        printf("%d vai executar o Nqueens\n", myPos);
        //fflush(stdout);
        Nqueens(board, trial, size, 1, myPos);
        Mark(0, board[0], size, Diag, AntiD, FALSE);
        swap(int, board[0], board[col]); // Undo the swap
                                         // Put the data into the communication vector
        
        char file_name[24];    
        snprintf(file_name, 24, "solution%d_%d.txt", size, myPos);
        file_result = fopen(file_name, "rb"); //somente leitura
        //https://stackoverflow.com/questions/14002954/c-programming-how-to-read-the-whole-file-contents-into-a-buffer
        fseek(file_result, 0, SEEK_END);
        int fsize = ftell(file_result);
        fseek(file_result, 0, SEEK_SET);
        char char_info[400];
        //char char_info[20];
        fread(char_info, 1, fsize, file_result);
        fclose(file_result);
        printf("\n\n\nchar_info = %s\n\n\n", char_info);

        int_info[0] = total_unique;
        int_info[1] = total_all;

        int_info[2] = fsize;
        printf("Escravo %d enviando resultado (%d, %d) com tamanho=%d.\n",
                   myPos, int_info[0], int_info[1],int_info[2]);
        MPI_Send(int_info, 3, MPI_INT, 0, TAG_DATA_INT, MPI_COMM_WORLD);
        MPI_Send(char_info, fsize, MPI_CHAR, 0, TAG_DATA_CHAR, MPI_COMM_WORLD);
        printf("\nenviou!!\n");

        printf("Escravo %d esperando por um trabalho,\n", myPos);

        MPI_Recv(buffer, 2, MPI_INT, 0, TAG_INIT, MPI_COMM_WORLD, &Status);
        size = buffer[0];
        col = buffer[1];
    }
    // Final hand-shake:  get permission to terminate
    MPI_Recv(buffer, 0, MPI_INT, 0, TAG_EXIT, MPI_COMM_WORLD, &Status);
}



int main(int argc, char *argv[])
{
    int nProc; // Número de processos a serem executados
    //MPI_Status Status; // Return status from MPI
    int myPos; // Código do processo atual

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myPos);
    MPI_Comm_size(MPI_COMM_WORLD, &nProc);
    
    printf("Processo %d de %d iniciado.\n", myPos, nProc);
    //fflush(stdout);
    

    if (myPos == MPI_MATER) //Mestre
    {
        double start_time = wtime();
        double end_time;
        int size = 12;
        int k;
        FILE *fptr;
        double ClockT, CPU[2], Clock[2], Lapsed,
            *clientTime = (double *)calloc(nProc, sizeof *clientTime);

        puts("Server has entered its part of main");

        if (argc == 2)
        {
            size = atoi(argv[1]);
            printf("\nsize: %d\n", size);
        }

        puts("Server is calling StartQueens.");

        StartQueens(size, clientTime);

        puts("Server is back from StartQueens.");

        printf("%3d ==> %10ld  %10ld \n", size, total_unique, total_all);
        // for (k = 1; k < nProc; k++)
        //     printf("%15.7lg", clientTime[k]);
        putchar('\n');
        end_time = wtime();

        printf("\n\tTempo de execução:\t%.6f sec \n", end_time - start_time);
    }
    else // I.e., this is the client/slave/node
        ProcessQueens(myPos);

    printf("Process %d is back\n", myPos);

    MPI_Finalize();
    exit(0);
}