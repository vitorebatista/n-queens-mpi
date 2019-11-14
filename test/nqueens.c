#include <stdio.h>
#include <stdlib.h>

#define N 7

typedef enum boolean_ {
  false = 0,
  true = 1
} boolean;

int rev[N] = {N};
int count = 0;

void solve(int ord) {
  int i, j;
  boolean field[N] = {false};
  boolean ret;

#ifdef DEBUG
  for (j = 0; j < N; j++) {
    printf("%d ", rev[j]);
  }
  printf("\n");
#endif

  if (ord >= N) {
    printf("solve: ");
    for (j = 0; j < N; j++) {
      printf("%d ", rev[j]);
      if (rev[j] >= N) {
	printf("fail\n");
	exit(1);
      }
    }
    printf("\n");
    count++;
    return;
  }

  for (i = 0; i < ord; i++) {
    if (rev[i] >= 0 && rev[i] < N) {
      field[rev[i]] = true;
    }
    if (rev[i] + (ord-i) >= 0 && rev[i] + (ord-i) < N) {
      field[rev[i] + (ord-i)] = true;
    }
    if (rev[i] - (ord-i)>= 0 && rev[i] - (ord-i)< N) {
      field[rev[i] - (ord-i)] = true;
    }
  }


  for (j = 0; j < N; j++) {
    if (field[j] == false) {
      rev[ord] = j;
      solve(ord+1);
      rev[ord] = N;
    }
  }
  return;
}

int main(void) {
  boolean ret;

  solve(0);
  printf("count: %d\n", count);

  return 0;
}