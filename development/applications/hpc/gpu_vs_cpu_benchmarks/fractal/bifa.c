/********************************************************************
  Benchmark program that measures the number of GigaFLOPS/sec an SMP machine
  can perform while computing a recursion for several input parameters in
  parallel. 

  The recursion 
      x_{n+1} = a * x_n * (1 - x_n)
  shows 'irregular' but bounded behaviour for a \in [1,4) and x_0 \in (0,1).
  Therefore an optimizing compiler cannot take any shortcuts. The resulting
  performance measurements are therefore trustworthy.

  The normal way of computing the recursion is when the macro 'ORDER_JI' is
defined.  However, even the Intel compiler (12.0) does not recognize that it
might be beneficial to reorder and unroll the loops. The following macro
combinations are available:
   
    ORDER_JI  -> The recursion written in the straightforward way.
    ORDER_IJ  -> The loops are swapped.
    ORDER_IJ & UNROLL -> The loops are swapped and the inner loop is unrollde
                 for maximum instructure level parallelism.
**/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <omp.h>
#include <string.h>
#include "gettime.h"

#ifdef _MSC_VER
#define THREADS 32
#else
const int THREADS =32;
#endif

void iteration(size_t N, float amax, float amin, float xmax, float xmin, float * xs)
{
  size_t i;
  int j;

#ifdef _OPENMP
  int rank = omp_get_thread_num();
  int procs = omp_get_num_threads();
#else
  int rank = 1;
  int procs = 1;
#endif

#ifdef ORDER_JI
  float a = amin + rank * (amax - amin) / procs;

  for (j = 0; j < THREADS; ++j)
  {
    float x = xmin + j*(xmax - xmin)/THREADS;

    for (i = 0; i < N; ++i)
      x = a * x * (1 - x);

    xs[rank * THREADS + j] = x;
  }

#else // ORDER = IJ

  float x[THREADS];
  float a = amin + rank * (amax - amin) / procs;

  for (j = 0; j < THREADS; ++j)
    x[j] = xmin + j*(xmax - xmin)/THREADS;

  for (i = 0; i < N; ++i)
  {

#ifndef UNROLL
    for (j = 0; j < THREADS; ++j)
      x[j] = a * x[j] * (1.0 - x[j]);

#else 

    for (j = 0; j < THREADS; j+=8)
    {
      // load
      float x_1 = x[j+0];
      float x_2 = x[j+1];
      float x_3 = x[j+2];
      float x_4 = x[j+3];
      float x_5 = x[j+4];
      float x_6 = x[j+5];
      float x_7 = x[j+6];
      float x_8 = x[j+7];

      // compute
      x_1 = a * x_1 * (1 - x_1);
      x_2 = a * x_2 * (1 - x_2);
      x_3 = a * x_3 * (1 - x_3);
      x_4 = a * x_4 * (1 - x_4);
      x_5 = a * x_5 * (1 - x_5);
      x_6 = a * x_6 * (1 - x_6);
      x_7 = a * x_7 * (1 - x_7);
      x_8 = a * x_8 * (1 - x_8);

      // store
      x[j+0] = x_1;
      x[j+1] = x_2;
      x[j+2] = x_3;
      x[j+3] = x_4;
      x[j+4] = x_5;
      x[j+5] = x_6;
      x[j+6] = x_7;
      x[j+7] = x_8;
    }
#endif // UNROLL
  }

  for (j = 0; j < THREADS; ++j)
    xs[rank * THREADS + j] = x[j];
#endif // ORDER = IJ
}

int main(int argc, char ** argv)
{
  size_t i;

  // parameters
  const float amin = 1.0;
  const float amax = 3.99;
  const float x0min = 0.01; 
  const float x0max = 0.99;

  const size_t N = (1ll << 31) / THREADS;

  // allocate output array
  float * xs;
 
 
  #pragma omp parallel
  {
	double t0, t1;

#ifdef _OPENMP
    const int rank = omp_get_thread_num();
    const int procs = omp_get_num_threads();
#else
    const int rank = 1;
    const int procs = 1;
#endif

    #pragma omp master
    {
      xs = (float * ) malloc(sizeof(float)*procs*THREADS);
      memset( xs, 0, sizeof(float)*procs*THREADS);
    
      // check input
      for (i = 0; i < procs*THREADS; ++i)
		if (xs[i] != 0.0)
		{
#ifdef _MSC_VER
		  printf("Error in validation. xs[%Iu] = %f\n", i, xs[i]);
#else
		  printf("Error in validation. xs[%zu] = %f\n", i, xs[i]);
#endif
		  break; 
		}
    }
    #pragma omp barrier

    // run kernel
    t0 = getTime();
    iteration(N, amax, amin, x0max, x0min, xs);
    #pragma omp barrier
    t1 = getTime();

    #pragma omp master
    {
	  double flops = 3.0 * N * procs * THREADS / (t1 - t0);

      // check output
      for (i = 0; i < procs*THREADS; ++i)
		if (xs[i] <= 0.0 || xs[i] > 1.0)
		{
#ifdef _MSC_VER
		  printf("Error in validation. xs[%Iu] = %f\n", i, xs[i]);
#else
		  printf("Error in validation. xs[%zu] = %f\n", i, xs[i]);
#endif
		  break; 
		}

      // print result
      printf("Cores: %d\n", procs);
      printf("Time: %f, GFLOPS/s = %f\n", (t1 - t0), flops * 1e-9);
    }

  }

  return 0;
}


