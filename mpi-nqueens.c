/* Fully optimized solution to the N-queens problem.
   This implementation removes the flag structure used to time
   the two optimizations (Wirth's O(1) validity check and the
   use of permutation vectors).  It also condenses two heavily
   heavily used into macros for to remove the function call
   overhead:
      Mark  --- modified to do both Mark and Unmark
      Valid --- the check of the diagonal attacks

   Author:    Timothy J. Rolfe
   Language:  C
*/
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <ctype.h>
#include <string.h>
#include "util.c"

// Zero para o mestre MPI
#define MPI_MATER 0

/*
Here are the definitions of the public fields of MPI_Status:
    int MPI_SOURCE;
    int MPI_TAG;
    int MPI_ERROR;
} MPI_Status;
*/

#define INIT 1 // Message to client:  size and [0]
#define DATA 2 // Message from client with results
#define EXIT 4 // Message from client with CPU time \
               // Also to client, giving permission to exit

// Server process:  send jobs to client compute engines and
// receives results back.
void StartQueens(int size, double *clientTime)
{
    int col, k,
        commBuffer[2],          // Communication buffer -- size, [0]
        Count[2],               // Counts back from the clients
        limit = (size + 1) / 2, // Mirror images done automatically
        nProc,                  // size of the communicator
        proc,                   // For loop [1..nProc-1] within initial message
        nActive;                // Number of active processes
    MPI_Status Status;

    if (TRACE)
        puts("Server process has entered StartQueens");

    MPI_Comm_size(MPI_COMM_WORLD, &nProc);

    commBuffer[0] = size;
    // Send initial configurations to all client processes --- or to those
    // needed in case not all are required.
    for (col = 0, proc = 1; proc < nProc && col < limit; proc++, col++)
    {
        printf("\nproc[%d] col=%d\n\n", proc, col);
        commBuffer[1] = col;
        if (TRACE)
            printf("Sending client %d job %d,%d\n", proc,
                   commBuffer[0], commBuffer[1]);
        MPI_Send(commBuffer, 2, MPI_INT, proc, INIT, MPI_COMM_WORLD);
    }
    nActive = proc - 1; // Since rank==0 is not used
    if (proc < nProc)   // More processes than jobs
    {
        int dmy[2] = {0, 0}; // Termination message to unused processes
        while (proc < nProc)
            MPI_Send(dmy, 2, MPI_INT, proc++, INIT, MPI_COMM_WORLD);
    }
    if (TRACE)
        puts("Server beginning to wait on results");
    // Receive back results and send out new problems
    while (col < limit)
    {
        MPI_Recv(Count, 2, MPI_INT, MPI_ANY_SOURCE, DATA,
                 MPI_COMM_WORLD, &Status);
        proc = Status.MPI_SOURCE;
        if (TRACE)
            printf("Received results from client %d (%d, %d)\n",
                   proc, Count[0], Count[1]);
        Nunique += Count[0];
        Ntotal += Count[1];
        commBuffer[1] = col++;
        if (TRACE)
            printf("Sending client %d job %d,%d\n", proc,
                   commBuffer[0], commBuffer[1]);
        MPI_Send(commBuffer, 2, MPI_INT, proc, INIT, MPI_COMM_WORLD);
    }
    // Finally, receive back pending results and send termination
    // indication (message with size of zero).
    commBuffer[0] = 0;
    while (nActive > 0)
    {
        if (TRACE)
            printf("%d pending\n", nActive);
        MPI_Recv(Count, 2, MPI_INT, MPI_ANY_SOURCE, DATA, MPI_COMM_WORLD, &Status);
        --nActive;
        proc = Status.MPI_SOURCE;
        if (TRACE)
            printf("Received results from client %d (%d, %d)\n",
                   proc, Count[0], Count[1]);
        Nunique += Count[0];
        Ntotal += Count[1];
        if (TRACE)
            printf("Sending client %d termination message\n", proc);
        MPI_Send(commBuffer, 2, MPI_INT, proc, INIT, MPI_COMM_WORLD);
    }
    for (proc = 1; proc < nProc; proc++)
    {
        MPI_Send(&proc, 0, MPI_INT, proc, EXIT, MPI_COMM_WORLD);
        if (TRACE)
            printf("Sending EXIT to %d\n", proc);
    }
    if (TRACE)
        puts("Exiting StartQueens.");
}

// Prototype for forward referencing
void Nqueens(int *, int *, int, int);

// Client processes receive problems to process from the
// server and then return their results to the server.
void ProcessQueens(int myPos)
{
    int nCells = 0, size, k, col,
        buffer[2];
    int *board = NULL, *trial = NULL; // Allow for realloc use
    MPI_Status Status;

    if (TRACE)
        printf("Client %d has entered ProcessQueens.\n", myPos);
    MPI_Recv(buffer, 2, MPI_INT, 0, INIT, MPI_COMM_WORLD, &Status);
    if (TRACE)
        printf("Client %d has received problem: %d and %d\n",
               myPos, buffer[0], buffer[1]);
    fflush(stdout);
    size = buffer[0];
    col = buffer[1];
    // As long as a valid problem is in hand, do the processing.
    // The server sends a size of zero as a termination message
    while (size > 0)
    {
        int Count[2];

        // Generate the arrays
        if (size > nCells)
        {
            int idx;

            board = (int *)calloc(size, sizeof *board);
            trial = (int *)calloc(size * 2, sizeof *trial);
            // Allocate the boolean arrays Diag and AntiD
            // Note that calloc automatically fills with FALSE (0)
            Diag = (short *)calloc(2 * (size - 1), sizeof *Diag);
            AntiD = (short *)calloc(2 * (size - 1), sizeof *AntiD);
            // Initial permutation generated
            // Since trial is scratch space, it is filled by Nqueens.
            for (idx = 0; idx < size; idx++)
                board[idx] = idx;

            nCells = size;
        }
        // Zero out the counters for THIS problem start.
        Nunique = 0,
        Ntotal = 0;
        swap(int, board[0], board[col]);
        // CRITICAL:  mark [0] as used, and then as unused
        Mark(0, board[0], size, Diag, AntiD, TRUE);
        if (TRACE)
            printf("%d calling Nqueens\n", myPos);
        fflush(stdout);
        Nqueens(board, trial, size, 1);
        Mark(0, board[0], size, Diag, AntiD, FALSE);
        swap(int, board[0], board[col]); // Undo the swap
                                         // Put the data into the communication vector
        Count[0] = Nunique;
        Count[1] = Ntotal;
        if (TRACE)
            printf("Client %d sending results (%d, %d).\n",
                   myPos, Count[0], Count[1]);
        MPI_Send(Count, 2, MPI_INT, 0, DATA, MPI_COMM_WORLD);
        // Get the next job --- or the termination message.
        if (TRACE)
            printf("Client %d waiting for job,\n", myPos);
        MPI_Recv(buffer, 2, MPI_INT, 0, INIT, MPI_COMM_WORLD, &Status);
        size = buffer[0];
        col = buffer[1];
    }
    // Final hand-shake:  get permission to terminate
    MPI_Recv(buffer, 0, MPI_INT, 0, EXIT, MPI_COMM_WORLD, &Status);
}

/****************************************************************/
/*            And finally, all the Nqueens logic                */
/****************************************************************/

int main(int argc, char *argv[])
{
    int nProc, // Processes in the communicator
        proc;  // loop variable
    //MPI_Status Status; // Return status from MPI
    int rc;    // Status  code from MPI_Xxx() call
    int myPos; // My own position

    rc = MPI_Init(&argc, &argv);
    if (rc != MPI_SUCCESS)
    {
        puts("MPI_Init failed");
        exit(-1);
    }

    rc = MPI_Comm_rank(MPI_COMM_WORLD, &myPos);
    rc = MPI_Comm_size(MPI_COMM_WORLD, &nProc);
    if (TRACE)
    {
        printf("Process %d of %d started.\n", myPos, nProc);
        fflush(stdout);
    }

    if (myPos == MPI_MATER) // I.e., this is the server/master/host
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

        printf("%3d ==> %10ld  %10ld \n",
               size, Nunique, Ntotal);
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