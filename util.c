#include "time.c"
// https://en.wikipedia.org/wiki/Eight_queens_puzzle


long int total_unique = 0, // Accumulate results here
    total_all = 0;

// These need to be modified outside of Nqueens
short *Diag = NULL, *AntiD = NULL;

enum
{
    FALSE,
    TRUE
} TRACE = TRUE; // Enable/disable tracing execution


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


/* Copy the vector to another vector */
int CopyVector(int R[], int V[], int N, int nList)
{
    for (int Idx = 0; Idx < N; Idx++)
    {
        R[Idx] = V[Idx];
    }
    return ++nList;
}
/* Function to compare to order a vector using by qsort func */
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

/* Write the board result to a text file (solution<n>.txt */
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
/*
Original                      90 degree rotation
  1  .  .  .  .                 .  .  .  .  1
  .  .  2  .  .                 .  4  .  .  .
  .  .  .  .  3                 .  .  .  2  .
  .  4  .  .  .                 5  .  .  .  .
  .  .  .  5  .                 .  .  3  .  .
*/
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
/*
Original                       Vertical mirror
  1  .  .  .  .                  .  .  .  .  1
  .  .  2  .  .                  .  .  2  .  .
  .  .  .  .  3                  3  .  .  .  .
  .  4  .  .  .                  .  .  .  4  .
  .  .  .  5  .                  .  5  .  .  .
*/
void Vmirror(int R[], int N)
{
    int Idx;

    for (Idx = 0; Idx < N; Idx++)
        R[Idx] = (N - 1) - R[Idx];
    return;
}


/* Verifique as simetrias. Retorne 0 se este não for o primeiro */
/* solução no conjunto de soluções equivalentes; de outra forma */
/* retornar o número de soluções equivalentes.                  */
int SymmetryOps(
    int Board[], /* O painel totalmente preenchido                 */
    int Trial[], /* Usado para verificações de simetria            */
                 /* Possui seu próprio espaço de trabalho também!  */
    int Size,    /* Número de células em uma linha / coluna        */
    int Process) /* */
{
    int Idx;                     
    int Nequiv;                  /* Número de equivalentes          */
    int *Scratch = &Trial[Size]; 
    int **result = (int **)calloc(8, sizeof(int *));
    int nList = 0;

    FILE *file_result;
    /* Copy; Trial will be subjected to the transformations   */
    for (Idx = 0; Idx < Size; Idx++)
        Trial[Idx] = Board[Idx];

    for (Idx = 0; Idx < 8; Idx++)
    {
        result[Idx] = (int *)calloc(Size, sizeof(int));
    }
    /* 90 degrees --- clockwise (4th parameter of Rotate is FALSE)*/
    char file_name[24];
    
    snprintf(file_name, 24, "solution%d_%d.txt", Size, Process);
    file_result = fopen(file_name, "a");
    if (file_result == NULL)
    {
        printf("\n\n****Error ao abrir o arquivo %s para escrever \n\n***", file_name);
    }
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
        nList = CopyVector(result[nList], Trial, Size, nList);   //0
        nList = CopyVector(result[nList], Board, Size, nList);   //1
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

            nList = CopyVector(result[nList], Trial, Size, nList);   //3
            nList = CopyVector(result[nList], Scratch, Size, nList); //4
            Rotate(Trial, Scratch, Size, 0);

            Idx = intncmp(Board, Trial, Size);
            if (Idx > 0)
            {
                free(result);
                fclose(file_result);
                return 0;
            }
            nList = CopyVector(result[nList], Trial, Size, nList);   //5
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
            writeBoard(result[n], Size, file_result);
        }
    }
    // printf("Quantidade=%d\n", Nequiv * 2);

    free(result);
    fclose(file_result);
    return Nequiv * 2;
}

/* Process the partial (or complete) board for the indicated Row */
void Nqueens(int Board[], int Trial[], int Size, int Row, int Process)
{
    int Idx, Lim, Vtemp;
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
            Nqueens(Board, Trial, Size, Row + 1, Process);
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
                Nqueens(Board, Trial, Size, Row + 1, Process);
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
        Idx = SymmetryOps(Board, Trial, Size, Process);
        if (Idx)
        {
            total_unique++;
            total_all += Idx;
        }
    }
    return;
}