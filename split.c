/* Fully optimized solution to the N-queens problem.
   This implementation removes the flag structure used to time
   the two optimizations (Wirth's O(1) validity check and the
   use of permutation vectors).  It also condenses two heavily
   heavily used into macros for to remove the function call
   overhead:
      Mark  --- modified to do both Mark and Unmark
      Valid --- the check of the diagonal attacks

   In addition, this modifies the single call
   Nqueens (Board, Trial, Size, 0);
   into the call to a procedure which explicitly generates each
   of the [0] permutations --- Deal (Size)  The Deal procedure,
   then, invokes Start(k, Size) --- generate the base permutation,
   swap [k] into [0], mark [0] as in use, then make the call
   Nqueens (Board, Trial, Size, 1);
   Afterwards mark [0] as NOT in use.

   This is preliminary work to running the program in parallel
   under MPI:  The "Deal" procedure mimics the work server
   program distributing the tasks to the work-engine clients.
   The "Start" procedure then mimics the work-engine client
   program that receives a message from the server and computes
   part of the solution.

   Author:    Timothy J. Rolfe
   Language:  C
*/
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <ctype.h>
#include <string.h>
#include "cpuTimes.h"

#define FALSE 0
#define TRUE  1

long int Nunique = 0,
         Ntotal  = 0;

// These need to be modified outside of Nqueens
short *Diag, *AntiD = NULL;

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
#define Mark(R,C,Size,Diag,AntiD,Flag) \
   {  Diag[R-C+Size-1]=Flag; AntiD[R+C]=Flag;  }

void dump(int *, int);
void Nqueens (int*, int*, int, int);

// x is the data type:  swap items y and z.
#define swap(x, y, z) {x tmp = y; y = z; z = tmp;}

void Start(int k, int Size)
{
   static int *Board = NULL, *Trial = NULL;

   if (Board == NULL)
   {  int Idx;
      Board = (int *) calloc (Size, sizeof *Board);
      Trial = (int *) calloc (Size*2, sizeof *Board);
   /* Initial permutation generated. */
      for (Idx = 0; Idx < Size; Idx++)
         Board[Idx] = Idx;

   /* Allocate the boolean arrays Diag and AntiD */
   /* NOTE:  calloc initializes to all zero == FALSE  */
      Diag  = (short *) calloc ( 2*Size-1, sizeof *Diag );
      AntiD = (short *) calloc ( 2*Size-1, sizeof *AntiD );
      if ( !AntiD )
      {
         puts ("calloc failed --- out of memory!");
         exit (6);
      }
   }
   swap (int, Board[0], Board[k]);
// CRITICAL:  mark [0]
   Mark (0, Board[0], Size, Diag, AntiD, TRUE);
   Nqueens (Board, Trial, Size, 1);
// Undo the first two operations --- in appropriate order
   Mark (0, Board[0], Size, Diag, AntiD, FALSE);
   swap (int, Board[0], Board[k]);
}

void Deal (int Size)
{
   int k, lim = (Size+1) / 2;
   for ( k = 0; k < lim; k++ )
      Start(k, Size);
}

/* Check two vectors for equality; return first inequality (a la strncmp) */
int intncmp (int L[], int R[], int N)
{
   int Idx;

   for (Idx = 0; Idx < N; Idx++)
      if ( L[Idx] - R[Idx] )
         return L[Idx]-R[Idx];
   return 0;
}

/* Rotate +90 or -90: */
void Rotate(int R[], int C[], int N, int Neg)
{
   int Idx, Jdx;

   Jdx = Neg ? 0 : N-1;
   for (Idx = 0; Idx < N; Neg ? Jdx++ : Jdx--)
      C[Idx++] = R[Jdx];
   Jdx = Neg ? N-1 : 0;
   for (Idx = 0; Idx < N; Neg ? Jdx-- : Jdx++)
      R[C[Idx++]] = Jdx;
}

/* Vertical mirror:  reflect each row across the middle */
void Vmirror(int R[], int N)
{
   int Idx;

   for (Idx = 0; Idx < N; Idx++)
      R[Idx] = (N-1) - R[Idx];
   return;
}

/* Check the symmetries.  Return 0 if this is not the 1st */
/* solution in the set of equivalent solutions; otherwise */
/* return the number of equivalent solutions.             */
int SymmetryOps(
    int Board[],     /* The fully-populated board         */
    int Trial[],     /* Used for symmetry checks          */
                     /* Holds its own scratch space too!  */
    int Size)        /* Number of cells in a row/column   */
{  int  Idx;         /* Loop variable; intncmp result     */
   int  Nequiv;      /* Number equivalent boards          */
   int *Scratch=&Trial[Size];   /* Scratch space          */

/* Copy; Trial will be subjected to the transformations   */
   for (Idx = 0; Idx < Size; Idx++)
      Trial[Idx] = Board[Idx];

/* 90 degrees --- clockwise (4th parameter of Rotate is FALSE)*/
   Rotate (Trial, Scratch, Size, 0);
   Idx = intncmp (Board, Trial, Size);
   if (Idx > 0) return 0;
   if ( Idx == 0 )  /* No change on 90 degree rotation        */
      Nequiv = 1;
   else                                       /*  180 degrees */
   {  Rotate (Trial, Scratch, Size, 0);
      Idx = intncmp (Board, Trial, Size);
      if (Idx > 0) return 0;
      if ( Idx == 0 ) /* No change on 180 degree rotation     */
         Nequiv = 2;
      else                                    /* 270 degrees  */
      {  Rotate (Trial, Scratch, Size, 0);
         Idx = intncmp (Board, Trial, Size);
         if (Idx > 0) return 0;
         Nequiv = 4;
      }
   }
/* Copy the board into Trial for rotational checks */
   for (Idx = 0; Idx < Size; Idx++)
      Trial[Idx] = Board[Idx];
/* Reflect -- vertical mirror */
   Vmirror (Trial, Size);
   Idx = intncmp (Board, Trial, Size);
   if (Idx > 0) return 0;
   if ( Nequiv > 1 )    // I.e., no four-fold rotational symmetry
   {
/* -90 degrees --- equiv. to diagonal mirror */
      Rotate (Trial, Scratch, Size, -1);
      Idx = intncmp (Board, Trial, Size);
      if (Idx > 0) return 0;
      if ( Nequiv > 2 ) // I.e., no two-fold rotational symmetry
      {
/* -180 degrees --- equiv. to horizontal mirror */
         Rotate (Trial, Scratch, Size, -1);
         Idx = intncmp (Board, Trial, Size);
         if (Idx > 0) return 0;
/* -270 degrees --- equiv. to anti-diagonal mirror */
         Rotate (Trial, Scratch, Size, -1);
         Idx = intncmp (Board, Trial, Size);
         if (Idx > 0) return 0;
      }
   }
/* WE HAVE A GOOD ONE! */
   return Nequiv * 2;
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
!( Diag[Row-Board[Row]+Size-1] || AntiD[Row+Board[Row]] )

/* Process the partial (or complete) board for the indicated Row */
void Nqueens (int Board[], int Trial[], int Size, int Row)
{
   int Idx, Lim, Vtemp;

/* Check for a partial board. */
   if (Row < Size-1)
   {
      if (Valid (Board, Size, Row, Diag, AntiD))
      {
         Mark (Row, Board[Row], Size, Diag, AntiD, TRUE);
         Nqueens (Board, Trial, Size, Row+1);
         Mark (Row, Board[Row], Size, Diag, AntiD, FALSE);
      }
/*    Rejection of vertical mirror images means that row zero */
/*    only needs to be processed to the middle.               */
      Lim = Row ? Size : (Size+1)/2 ;
      for (Idx = Row+1; Idx < Lim; Idx++)
      {
         Vtemp = Board[Idx];
         Board[Idx] = Board[Row];
         Board[Row] = Vtemp;
         if (Valid (Board, Size, Row, Diag, AntiD))
         {
            Mark (Row, Board[Row], Size, Diag, AntiD, TRUE);
            Nqueens (Board, Trial, Size, Row+1);
            Mark (Row, Board[Row], Size, Diag, AntiD, FALSE);
         }
      }
/*    Regenerate original vector from Row to Size-1:  */
      Vtemp = Board[Row];
      for (Idx = Row+1; Idx < Size; Idx++)
         Board[Idx-1] = Board[Idx];
      Board[Idx-1] = Vtemp;
   }
/* This is a complete board.  Do the symmetry checks */
   else
   {
      if ( !Valid (Board, Size, Row, Diag, AntiD) )
         return;
      Idx = SymmetryOps (Board, Trial, Size);
      if (Idx)
      {
         Nunique++;
         Ntotal += Idx;
      }
   }
   return;
}

void dump(int *x, int n)     // Debugging display
{  int k;
   for (k = 0; k < n; k++ )
      printf("%3d", x[k]);
   printf (" ==> %ld and %ld\n", Nunique, Ntotal);
}

main(int argc, char *argv[])
{
   int   Size;
   FILE  *fptr;
   double Clock, ClockStart, CPUstart, Lapsed;

   if (argc < 2)
   {
      fputs ("Size:  ", stdout);
      scanf ("%d", &Size);
   }
   else
   {
      Size = atoi(argv[1]);
   }

   getTimes(&ClockStart, &CPUstart);
// Nqueens (Board, Trial, Size, 0);
   Deal (Size);
   getTimes(&Clock, &Lapsed);
   Lapsed = Lapsed - CPUstart;
   Clock  = Clock  - ClockStart;

   printf ("%3d ==> %10ld  %10ld  %10.4f  %10.4f\n",
           Size, Nunique, Ntotal, Lapsed, Clock);

   fptr = fopen("Nqueens.csv", "a");
   fprintf (fptr, "%3d,%10ld,%10ld,%10.4f,%10.4f",
            Size, Nunique, Ntotal, Lapsed, Clock);

   fprintf (fptr, ",%5d\n", 4);
   fclose (fptr);

   return 0;
}