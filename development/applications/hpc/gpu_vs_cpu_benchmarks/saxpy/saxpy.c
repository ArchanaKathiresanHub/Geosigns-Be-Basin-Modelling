/********************************************************************
  Simple benchmark program that measures the number of GigaFLOPS/sec all CPUs
  in a comptuter can perform. Therefore it does several computations in
  parallel, for which it uses OpenMP.

  The operation it test is a SAXPY: y = alpha * x + y, where alpha is a scalar
  and x and y are vectors.  To get an optimistic figure the saxpy is repeated
  ITER1 times. This increases the number of FLOPS per read/written bytes
  from/to memory.

  Note: The 'nans' in the output are for certain combinations of
     of 'threads' and 'blocks' it skipped.
**/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <omp.h>

#include "gettime.h"

const int ITER1=1<<10;
const int ITER2=1<<6;

#ifndef restrict
#define restrict
#endif

void saxpy( int n, float a, float * restrict x, float * restrict y)
{
  int i, j;
  for (j = 1; j <= ITER1; ++j)
    for (i = 0; i < n; ++i)
      y[i] = y[i] + j * a * x[i];
}


double measureSaxpy( const int N)
{
  int i;
  float *x, *y, a;

  // allocate memory
  x = (float *) malloc(sizeof(float)*N);
  y = (float *) malloc(sizeof(float)*N);

  // initialize memory
  a = 2.0/3.0;
  for (i = 0; i < N; ++i)
  {
    x[i] = rand() / (RAND_MAX + 1.0);
    y[i] = rand() / (RAND_MAX + 1.0);
  }

  // start timer
  double t0 = getTime();

  // run kernel
#pragma omp parallel
  {

    int rank = omp_get_thread_num();
    int procs = omp_get_num_threads();
    assert( N % procs == 0);
    const int n = N/procs;
    const int offset = rank * n;
    for (i = 0; i < ITER2; ++i)
    {
      saxpy(n, a, x + offset, y + offset);
    }
  }

  // synchronize and stop timer
  double t3 = getTime();

  // print some statistic based on x & y;
  double sum = 0.0;
  for (i = 0 ; i < N; ++i)
    sum += y[i];
  double avg= sum / N;

  const double epsilon = 0.1;
  const double expected = (0.5 + ITER2 * (0.5 * (ITER1+1) * ITER1) * 0.5 * a);
  if (fabs( avg - expected ) / expected > epsilon)
    fprintf(stderr, "High deviation: avg = %f, expected = %f\n", avg, expected);


  // print performance
  double flops= (double) ITER1 * ITER2 * N*2 / (t3 - t0);

   // release memory
  free(x);
  free(y);

  return flops;
}

int main(int argc, char ** argv)
{
  int i, j;
  const int MINN=1<<7;
  const int MAXN=1<<20;
  const int MAXTHREADS=1<<9;
  printf("#% 9s  % 10s\n", "N", "GFLOPS/s");
  for(i = MINN; i <= MAXN; i*=2)
  {
    printf("% 10d  % 10f\n", i, measureSaxpy(i) * 1e-09);
  }

  return 0;
}


// vim: ft=cpp:ai
