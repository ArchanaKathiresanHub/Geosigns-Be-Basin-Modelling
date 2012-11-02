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
#include <limits.h>

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


double measureSaxpy( const size_t N)
{
  size_t i;
  float *x, *y, a;
  double t0, t1;

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
  t0 = getTime();

  // run kernel
#pragma omp parallel
  {
	int j;
    int rank = omp_get_thread_num();
    int procs = omp_get_num_threads();
    const size_t n = N/procs;
    const size_t offset = rank * n;

	assert( N % procs == 0);
	assert( n < INT_MAX );

    for (j = 0; j < ITER2; ++j)
    {
      saxpy( (int) n, a, x + offset, y + offset);
    }
  }

  // synchronize and stop timer
  t1 = getTime();

  // Validate results
  {
	const double epsilon = 0.1;
	const double expected = (0.5 + ITER2 * (0.5 * (ITER1+1) * ITER1) * 0.5 * a);
	double avg=0.0;
	for (i = 0 ; i < N; ++i)
	  avg += y[i] / N;
	
	if (fabs( avg - expected ) / expected > epsilon)
	  fprintf(stderr, "High deviation: avg = %f, expected = %f\n", avg, expected);
  }

  // release memory
  free(x);
  free(y);

  // Return FLOPS
  return (double) ITER1 * ITER2 * N*2 / (t1 - t0);
}

int main(int argc, char ** argv)
{
  size_t i;
  const size_t MINN=1<<7;
  const size_t MAXN=1<<20;
  const size_t MAXTHREADS=1<<9;
  printf("#% 9s  % 10s\n", "N", "GFLOPS/s");
  for(i = MINN; i <= MAXN; i*=2)
  {
#ifdef _MSC_VER
	printf("% 10Iu  % 10f\n", i, measureSaxpy(i) * 1e-09);
#else
    printf("% 10zu  % 10f\n", i, measureSaxpy(i) * 1e-09);
#endif
  }

  return 0;
}


// vim: ft=cpp:ai
