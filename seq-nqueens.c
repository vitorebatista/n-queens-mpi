// http://penguin.ewu.edu/~trolfe/SCCS-95/index.html
// http://penguin.ewu.edu/~trolfe/Queens/OptQueen.html

/* Fully optimized solution to the N-queens problem.
   This implementation removes the flag structure used to time
   the two optimizations (Wirth's O(1) validity check and the
   use of permutation vectors).  It also uses the C++ option of
   declaring a function as inline for the two functions most
   heavily used:
      Mark  --- here modified to do both Mark and Unmark
      Valid --- the check of the diagonal attacks

   Author:    Timothy J. Rolfe
   Language:  C
*/

/*
   N = 5
   solve: 0 2 4 1 3 [1]
   solve: 0 3 1 4 2 [7]
   solve: 1 3 0 2 4 [3]
   solve: 1 4 2 0 3 [8]
   solve: 2 0 3 1 4 [4]
   solve: 2 4 1 3 0 [5]
   solve: 3 0 2 4 1 [9]
   solve: 3 1 4 2 0 [2]
   solve: 4 1 3 0 2 [0]
   solve: 4 2 0 3 1 [6]
   */

  /*
[0]     Board = 4 1 3 0 2  (3;6;14;17;20;)
[1]     Board = 0 2 4 1 3  (0;8;11;19;22;)
[2]     Board = 3 1 4 2 0  (4;6;13;15;22;)
[3]     Board = 1 3 0 2 4  (2;5;13;16;24;)
[4]     Board = 2 0 3 1 4  (1;8;10;17;24;)
[5]     Board = 2 4 1 3 0  (4;7;10;18;21;)
[6]     Board = 4 2 0 3 1  (2;9;11;18;20;)
[7]     Board = 0 3 1 4 2  (0;7;14;16;23;)
[8]     Board = 1 4 2 0 3  (3;5;12;19;21;)
[9]     Board = 3 0 2 4 1  (1;9;12;15;23;)

N=6
Moodle
[3] 1;9;17;18;26;34;
[2] 2;11;13;22;24;33;
[1] 3;6;16;19;29;32;
[0] 4;8;12;23;27;31;

Brute Force
solve: 1 3 5 0 2 4 
solve: 2 5 1 4 0 3 
solve: 3 0 4 1 5 2 
solve: 4 2 0 5 3 1 

Optimized
[0]     Board = 2 5 1 4 0 3  (4;8;12;23;27;31;)
[1]     Board = 1 3 5 0 2 4  (3;6;16;19;29;32;)
[2]     Board = 4 2 0 5 3 1  (2;11;13;22;24;33;)
[3]     Board = 3 0 4 1 5 2  (1;9;17;18;26;34;)
  
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "time.c"

#define FALSE 0
#define TRUE 1

long int Nunique = 0,
         Ntotal = 0;

/* Maintenance procedure:  print a picture of the board. */
void Picture(int Image[], int Size)
{
   int Row, Col, Tst;

   for (Row = 0; Row < Size; Row++)
   {
      putchar('\n');
      Tst = Image[Row];
      for (Col = 0; Col < Size; Col++)
      {
         putchar(' ');
         putchar(Col == Tst ? 'Q' : '.');
      }
   }
   putchar('\n');
   putchar('\n');
}

/* Check two vectors for equality; return first inequality (a la strncmp) */
int intncmp(int L[], int R[], int N)
{
   int Idx;

   for (Idx = 0; Idx < N; Idx++)

      if (L[Idx] - R[Idx])

         return L[Idx] - R[Idx];
      else
      {
         //printf("L: %d R: %d\n",L[Idx] , R[Idx]);
      }

   return 0;
}

/* Rotate +90 or -90: */
void Rotate(int R[], int C[], int N, int Neg)
{
   int Idx, Jdx;

   Jdx = Neg ? 0 : N - 1;
   for (Idx = 0; Idx < N; Neg ? Jdx++ : Jdx--)
      C[Idx++] = R[Jdx];
   Jdx = Neg ? N - 1 : 0;
   for (Idx = 0; Idx < N; Neg ? Jdx-- : Jdx++)
      R[C[Idx++]] = Jdx;
}

int cmpfunc(const void *a, const void *b)
{
   return (*(int *)a - *(int *)b);
}

void printBoard(int Board[], int N, FILE *file_result)
{
   int Idx;
   int result[N];
   #ifdef DEBUG
      printf("\tBoard = ");
      for (Idx = 0; Idx < N; Idx++)
         printf("%d ", Board[Idx]);

      printf(" (");
   #endif

   for (Idx = 0; Idx < N; Idx++)
      result[Idx] = Board[Idx] * N + Idx;

   qsort(result, sizeof(result) / sizeof(*result), sizeof(*result), cmpfunc);
   for (Idx = 0; Idx < N; Idx++)
   {
      fprintf(file_result, "%d;", result[Idx]);
      #ifdef DEBUG
         printf("%d;", result[Idx]);
      #endif
   }

   fprintf(file_result, "\n");
   #ifdef DEBUG
      printf(")");
      printf("\n");
   #endif
}

/* Vertical mirror:  reflect each row across the middle */
void Vmirror(int R[], int N)
{
   int Idx;

   for (Idx = 0; Idx < N; Idx++)
      R[Idx] = (N - 1) - R[Idx];
   return;
}
int CopyVector(int R[], int V[], int N, int nList){
   for (int Idx = 0; Idx < N; Idx++) {
      R[Idx] = V[Idx];
   }
   return ++nList;
}
/* Check the symmetries.  Return 0 if this is not the 1st */
/* solution in the set of equivalent solutions; otherwise */
/* return the number of equivalent solutions.             */
int SymmetryOps(
    int Board[], /* The fully-populated board         */
    int Trial[], /* Used for symmetry checks          */
                 /* Holds its own scratch space too!  */
    int Size)    /* Number of cells in a row/column   */
{
   int Idx;                     /* Loop variable; intncmp result     */
   int Nequiv;                  /* Number equivalent boards          */
   int *Scratch = &Trial[Size]; /* Scratch space          */
   int **result = (int **)calloc(8, sizeof(int *));
   int nList = 0;

   FILE *file_result;
   /* Copy; Trial will be subjected to the transformations   */
   for (Idx = 0; Idx < Size; Idx++)
      Trial[Idx] = Board[Idx];

   for (Idx = 0; Idx < 8; Idx++) {
      result[Idx] = (int *)calloc(Size, sizeof(int));
   }
   /* 90 degrees --- clockwise (4th parameter of Rotate is FALSE)*/
   char file_name[14];
   snprintf(file_name, 14, "solution%d.txt", Size);
   file_result = fopen(file_name, "a");
   
   Rotate(Trial, Scratch, Size, 0);
   Idx = intncmp(Board, Trial, Size);
   if (Idx > 0)
   {
      fclose(file_result);
      return 0;
   }
   if (Idx == 0)
   { /* No change on 90 degree rotation        */
      Nequiv = 1;
      nList = CopyVector(result[nList], Board, Size, nList);
      nList = CopyVector(result[nList], Scratch, Size, nList);
   }
   else /*  180 degrees */
   {
      nList = CopyVector(result[nList], Trial, Size, nList); //0
      nList = CopyVector(result[nList], Board, Size, nList); //1
      nList = CopyVector(result[nList], Scratch, Size, nList); //2
      Rotate(Trial, Scratch, Size, 0);
      Idx = intncmp(Board, Trial, Size);
      if (Idx > 0)
      {
         free(result);
         fclose(file_result);
         return 0;
      }

      if (Idx == 0)
      { /* No change on 180 degree rotation     */
         Nequiv = 2;
         nList = CopyVector(result[nList], Scratch, Size, nList); //0
      }
      else /* 270 degrees  */
      {
         
         nList = CopyVector(result[nList], Trial, Size, nList); //3
         nList = CopyVector(result[nList], Scratch, Size, nList); //4
         Rotate(Trial, Scratch, Size, 0);

         Idx = intncmp(Board, Trial, Size);
         if (Idx > 0)
         {
            free(result);
            fclose(file_result);
            return 0;
         }
         nList = CopyVector(result[nList], Trial, Size, nList); //5
         nList = CopyVector(result[nList], Scratch, Size, nList); //6

         Rotate(Trial, Scratch, Size, 0);
         nList = CopyVector(result[nList], Scratch, Size, nList); //7
         Nequiv = 4;
      }
   }
   

   /* Copy the board into Trial for rotational checks */
   for (Idx = 0; Idx < Size; Idx++)
      Trial[Idx] = Board[Idx];
   /* Reflect -- vertical mirror */
   
   Vmirror(Trial, Size);
   Idx = intncmp(Board, Trial, Size);
   if (Idx > 0)
   {
      free(result);
      fclose(file_result);
      return 0;
   }
   if (Nequiv > 1) // I.e., no four-fold rotational symmetry
   {
      /* -90 degrees --- equiv. to diagonal mirror */
      Rotate(Trial, Scratch, Size, -1);
      Idx = intncmp(Board, Trial, Size);
      if (Idx > 0)
      {
         free(result);
         fclose(file_result);
         return 0;
      }
      if (Nequiv > 2) // I.e., no two-fold rotational symmetry
      {
         /* -180 degrees --- equiv. to horizontal mirror */
         Rotate(Trial, Scratch, Size, -1);
         Idx = intncmp(Board, Trial, Size);
         if (Idx > 0)
         {
            free(result);
            fclose(file_result);
            return 0;
         }
         /* -270 degrees --- equiv. to anti-diagonal mirror */
         Rotate(Trial, Scratch, Size, -1);
         Idx = intncmp(Board, Trial, Size);
         if (Idx > 0)
         {
            free(result);
            fclose(file_result);
            return 0;
         }
         
         
      }
   }

   // printf("\nresultado final:\n");
   for (int n = 0; n < 8; n++)
   {
      if (result[n] && result[n][0] != result[n][1])
      {
         //printf("[%d] ",n);
         printBoard(result[n], Size, file_result);
      }
   }
   // printf("Quantidade=%d\n", Nequiv * 2);

   free(result);
   fclose(file_result);
   return Nequiv * 2;
}

/* Mark a particular [R][C] board cell as in-use or available     */
/* Massively used; inline to eliminate the function call overhead */
/*
inline void Mark (
     int R, int C, int Size,
     short Diag[], short AntiD[], int Flag)
{
   int Idx;

// Diagonal:  Row-Col == constant
   Idx = R - C + Size-1;
   Diag[Idx] = Flag;
// AntiDiagonal:  Row+Col == constant
   Idx = R + C;
   AntiD[Idx] = Flag;
}
 --- for straight C, make a #define macro   */
#define Mark(R, C, Size, Diag, AntiD, Flag) \
   {                                        \
      Diag[R - C + Size - 1] = Flag;        \
      AntiD[R + C] = Flag;                  \
   }

/* Test the validity of this particular partial board.            */
/* Massively used; inline to eliminate the function call overhead */
/*
inline int Valid (int Board[], int Size, int Row,
                  short Diag[], short AntiD[] )
{
   int Idx;        // Index into Diag[] / AntiD[]
   int Chk;        // Occupied flag

// Diagonal:  Row-Col == constant
   Idx = Row - Board[Row] + Size-1;
   Chk = Diag[Idx];
// AntiDiagonal:  Row+Col == constant
   Idx = Row + Board[Row];
   Chk = Chk || AntiD[Idx];
   return !Chk;    // Valid if NOT any occupied
}
 --- for straight C, make a #define macro   */
#define Valid(Board, Size, Row, Diag, AntiD) \
   !(Diag[Row - Board[Row] + Size - 1] || AntiD[Row + Board[Row]])

/* Process the partial (or complete) board for the indicated Row */
void Nqueens(int Board[], int Trial[], int Size, int Row)
{
   int Idx, Lim, Vtemp;
   static int *Diag, *AntiD = NULL;

   /* If the first call, allocate the boolean arrays Diag and AntiD */
   if (AntiD == NULL)
   {
      /*    NOTE:  calloc initializes to all zero == FALSE  */
      Diag = (int *)calloc(2 * Size - 1, sizeof *Diag);
      AntiD = (int *)calloc(2 * Size - 1, sizeof *AntiD);
      if (!AntiD)
      {
         puts("calloc failed --- out of memory!");
         exit(6);
      }
   }
   /* Check for a partial board. */
   if (Row < Size - 1)
   {
      if (Valid(Board, Size, Row, Diag, AntiD))
      {
         Mark(Row, Board[Row], Size, Diag, AntiD, TRUE);
         Nqueens(Board, Trial, Size, Row + 1);
         Mark(Row, Board[Row], Size, Diag, AntiD, FALSE);
      }
      /*    Rejection of vertical mirror images means that row zero */
      /*    only needs to be processed to the middle.               */
      Lim = Row ? Size : (Size + 1) / 2;
      for (Idx = Row + 1; Idx < Lim; Idx++)
      {
         Vtemp = Board[Idx];
         Board[Idx] = Board[Row];
         Board[Row] = Vtemp;
         if (Valid(Board, Size, Row, Diag, AntiD))
         {
            Mark(Row, Board[Row], Size, Diag, AntiD, TRUE);
            Nqueens(Board, Trial, Size, Row + 1);
            Mark(Row, Board[Row], Size, Diag, AntiD, FALSE);
         }
      }
      /*    Regenerate original vector from Row to Size-1:  */
      Vtemp = Board[Row];
      for (Idx = Row + 1; Idx < Size; Idx++)
         Board[Idx - 1] = Board[Idx];
      Board[Idx - 1] = Vtemp;
   }
   /* This is a complete board.  Do the symmetry checks */
   else
   {
      if (!Valid(Board, Size, Row, Diag, AntiD))
         return;
      Idx = SymmetryOps(Board, Trial, Size);
      if (Idx)
      {
         Nunique++;
         Ntotal += Idx;
      }
   }
   return;
}

int main(int argc, char *argv[])
{
   double start_time = wtime();
   double end_time, total_time;
   int *Board, *Trial, Idx, Size;
   FILE *fptr;
   double Clock, CPUstart, ClockStart, Lapsed;

   if (argc < 2)
   {
      fputs("Size:  ", stdout);
      scanf("%d", &Size);
   }
   else
   {
      Size = atoi(argv[1]);
   }
   char file_name[14];
   snprintf(file_name, 14, "solution%d.txt", Size);
   int ret = remove(file_name);

   if (ret == 0)
      printf("File deleted successfully\n");

   Board = (int *)calloc(Size, sizeof *Board);
   Trial = (int *)calloc(Size * 2, sizeof *Board);
   /* Initial permutation generated. */
   for (Idx = 0; Idx < Size; Idx++)
      Board[Idx] = Idx;

   //printBoard(Board, Size);
   Nqueens(Board, Trial, Size, 0);

   printf("%3d ==> %10ld  %10ld \n",
          Size, Nunique, Ntotal);
   end_time = wtime();
   total_time = end_time - start_time;
   printf("\nTempo de execução:\t%.6f sec \n", total_time);

   fptr = fopen("time-seq-nqueens.csv", "a");

   fprintf(fptr, "%3d,%10ld,%10ld,%10.4f\n",
           Size, Nunique, Ntotal, total_time);

   fclose(fptr);

   return 0;
}