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


/* Função que irá fazer a gestão dos jobs do MPI com a divisão dos parâmetros  */
/* Quando os escravos terminarem, eles enviarão o conteúdo para o mestre armazenar */
/* em um arquivo solution<n>.txt */
void MasterQueens(int size, double *clientTime)
{
    int col, k,
        commBuffer[2],          // Communication buffer -- size, [0]
        Count[3],               // Counts back from the clients
        limit = (size + 1) / 2, // Mirror images done automatically
        nProc,                  // size of the communicator
        proc,                   // For loop [1..nProc-1] within initial message
        nActive;                // Number of active processes
    int fsize;
    char file_name[24];   
    FILE *file_result; 
    MPI_Status Status;

    puts("Início da função MasterQueens");

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

    // Recebe os resultados e envia novos problemas
    while (col < limit)
    {
        MPI_Recv(Count, 3, MPI_INT, MPI_ANY_SOURCE, TAG_DATA_INT, MPI_COMM_WORLD, &Status);
        proc = Status.MPI_SOURCE;
        printf("Resultados recebidos do escravo %d (%d, %d)\n", proc, Count[0], Count[1]);
        total_unique += Count[0];
        total_all += Count[1];
        fsize = Count[2];
        commBuffer[1] = col++;
        char char_info[fsize + 1];
        printf("\n\n\n Tamanho do arquivo1 = %d\n\n\n", fsize);
        MPI_Recv(char_info, fsize+1, MPI_CHAR, MPI_ANY_SOURCE, TAG_DATA_CHAR, MPI_COMM_WORLD, &Status);
        //printf("\n\nstring1: %s\n", char_info);
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
        char char_info[fsize + 1];
        printf("\n\n\nTamanho do arquivo2 = %d\n\n\n", fsize);
        MPI_Recv(char_info, fsize+1 , MPI_CHAR, MPI_ANY_SOURCE, TAG_DATA_CHAR, MPI_COMM_WORLD, &Status);
        //printf("\n\n String recebida: %s \n", char_info);
        
        printf("Enviando para o escravo %d msg de termino\n", proc);
        MPI_Send(commBuffer, 2, MPI_INT, proc, TAG_INIT, MPI_COMM_WORLD);
        
        
        snprintf(file_name, 24, "solution%d.txt", size);
        printf("\nVai adicionar conteúdo no arquivo %s\n", file_name);
        file_result = fopen(file_name, "w"); //somente leitura
        fprintf(file_result, "%s", char_info);
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
    char file_name[24];
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
        
        printf("Escravo %d vai executar o Nqueens\n", myPos);
        //fflush(stdout);
        Nqueens(board, trial, size, 1, myPos);
        printf("Escravo %d vai terminar o Nqueens\n", myPos);
        Mark(0, board[0], size, Diag, AntiD, FALSE);
        swap(int, board[0], board[col]); // Undo the swap
                                         // Put the data into the communication vector
        
        printf("Escravo %d realizou o swap\n", myPos);
        
        snprintf(file_name, 24, "solution%d_%d.txt", size, myPos);
        printf("Escravo %d vai abrir arquivo solution%d_%d.txt", myPos, size, myPos);
        file_result = fopen(file_name, "r"); //somente leitura
        
        //https://stackoverflow.com/questions/14002954/c-programming-how-to-read-the-whole-file-contents-into-a-buffer
        fseek(file_result, 0, SEEK_END);
        int fsize = ftell(file_result);
        fseek(file_result, 0, SEEK_SET);
        char char_info[fsize + 1];
        //char char_info[20];
        fread(char_info, 1, fsize, file_result);
        fclose(file_result);
        char_info[fsize] = (char)0;
        //printf("\n\n\nchar_info = %c\n\n\n", char_info);

        int_info[0] = total_unique;
        int_info[1] = total_all;
        int_info[2] = fsize;

        printf("Escravo %d enviando resultado (%d, %d) com tamanho=%d.\n",
                   myPos, int_info[0], int_info[1],int_info[2]);
        MPI_Send(int_info, 3, MPI_INT, 0, TAG_DATA_INT, MPI_COMM_WORLD);
        int countPart = fsize / 10000000;
        printf("\n\nPreparar para o for %d\n\n", countPart);
        MPI_Send(char_info, fsize + 1, MPI_CHAR, 0, TAG_DATA_CHAR, MPI_COMM_WORLD);
        //printf("\nenviou!!\n");

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

        puts("Vai iniciar a chamada da função MasterQueens.");

        MasterQueens(size, clientTime);

        puts("Término da função MasterQueens.");

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