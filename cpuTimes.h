#ifndef CPUTIMES
#define CPUTIMES
void getTimes( double *wallClock, double *cpuClock );

// Return ONLY the CPU time.
double cpuClock();

// Return ONLY the wall clock time.
double wallClock();
#endif