#ifndef CPUCLOCK_C      // In case this CODE file is included in other code
#define CPUCLOCK_C

#include <sys/times.h>  // Defines the struct tms
#include <unistd.h>     // Pulls in _SC_CLK_TCK
#include <time.h>
#include <stdio.h>
// Possibly restricted to Linux:
// return as floating point seconds both the CPU time
// and the wall-clock time.
void getTimes( double *wallClock, double *cpuClock )
{
   static double cvt = 0.0;       // Retain across function calls
   struct tms cpu;
   clock_t wall = times( &cpu );   // times() return wall clock

   // One time only, capture the conversion factor
   if ( cvt == 0.0 )
   {  cvt = 1.0 / sysconf(_SC_CLK_TCK);
   }
   *wallClock = cvt * wall;
   *cpuClock  = cvt * cpu.tms_utime;
}

// Return ONLY the CPU time.
double cpuClock()
{  double rtnVal, dmy;
   getTimes (&dmy, &rtnVal);
   return rtnVal;
}

// Return ONLY the wall clock time.
double wallClock()
{  double rtnVal, dmy;
   getTimes (&rtnVal, &dmy);
   return rtnVal;
}

#endif