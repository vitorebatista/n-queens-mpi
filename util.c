
double wtime() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec + (double) t.tv_usec / 1000000;
}

/* Mark a particular [R][C] board cell as in-use or available     */
/* Massively used; inline to eliminate the function call overhead */
/*
inline void Mark (
     int R, int C, int size,
     short Diag[], short AntiD[], int Flag)
{
   int Idx;

// Diagonal:  Row-Col == constant
   Idx = R - C + size-1;
   Diag[Idx] = Flag;
// AntiDiagonal:  Row+Col == constant
   Idx = R + C;
   AntiD[Idx] = Flag;
}
--- for straight C, make a #define macro   */
#define Mark(R, C, size, Diag, AntiD, Flag) \
    {                                       \
        Diag[R - C + size - 1] = Flag;      \
        AntiD[R + C] = Flag;                \
    }

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