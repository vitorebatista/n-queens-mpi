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
#include "util.c"

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

   // printf("row[%d] ", 0);
   // printBoard(Board, Size);
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