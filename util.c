#include "time.c"
// https://en.wikipedia.org/wiki/Eight_queens_puzzle

int CopyVector(int R[], int V[], int N, int nList)
{
    for (int Idx = 0; Idx < N; Idx++)
    {
        R[Idx] = V[Idx];
    }
    return ++nList;
}
int cmpfunc(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}
void printBoard(int Board[], int N)
{   
    printf("\tBoard = ");
    for (int Idx = 0; Idx < N; Idx++)
        printf("%d ", Board[Idx]);

    printf(" (");
}
void writeBoard(int Board[], int N, FILE *file_result)
{
    int Idx;
    int result[N];

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

// Macro to swap datatype x, locations y and z
#define swap(x, y, z) \
    {                 \
        x temp = y;   \
        y = z;        \
        z = temp;     \
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

/* Vertical mirror:  reflect each row across the middle */
void Vmirror(int R[], int N)
{
    int Idx;

    for (Idx = 0; Idx < N; Idx++)
        R[Idx] = (N - 1) - R[Idx];
    return;
}