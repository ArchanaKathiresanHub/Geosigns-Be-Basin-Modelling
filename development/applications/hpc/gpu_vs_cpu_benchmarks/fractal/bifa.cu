/********************************************************************
  Benchmark program that measures the number of GigaFLOPS/sec
  an NVidia CUDA enabled GPU can perform while computing a recursion
  for several input parameters in parallel. 

  The recursion
      x_{n+1} = a * x_n * (1 - x_n)
  shows 'irregular' but bounded behaviour for a \in [1,4) and x_0 \in (0,1).
  Therefore an optimizing compiler cannot take any shortcuts. The resulting
  performance measurements are therefore trustworthy.
**/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <unistd.h>


const int THREADS=32;

double getTime()
{
  struct timeval t;
  gettimeofday(&t, NULL);

  return t.tv_sec + 1.0e-6 * t.tv_usec;
}

void c(cudaError_t errorcode)
{
  if (errorcode != cudaSuccess)
    fprintf(stderr, "CUDA ERROR = %s\n", cudaGetErrorString(errorcode));
}

__global__ void iteration(int N, float amax, float amin, float xmax, float xmin, float * xs)
{
  int i;

  float a = amin + blockIdx.x * (amax - amin) / gridDim.x;
  float x = xmin + threadIdx.x * (xmax - xmin) / blockDim.x;

  for (i = 0; i < N; ++i)
  {
    x = a * x * (1 - x);
  }

  xs[blockIdx.x * blockDim.x + threadIdx.x] = x;
}

int main(int argc, char ** argv)
{
  cudaGetLastError(); // clear cuda errors;

  // parameters
  const float amin = 1.0;
  const float amax = 3.99;
  const float x0min = 0.01; 
  const float x0max = 0.99;

  const int BLOCKS= 1<<13; 
  const int N = (1l << 35) / (THREADS * BLOCKS);

  // allocate output array
  float * h_xs, * d_xs;
  c( cudaMalloc(&d_xs, sizeof(float)*BLOCKS * THREADS));
  c( cudaMemset(d_xs, 0, sizeof(float)*BLOCKS*THREADS));

  h_xs = (float * ) malloc(sizeof(float)*BLOCKS*THREADS);
  memset( h_xs, 0, sizeof(float)*BLOCKS*THREADS);
  
  // check input
  for (int i = 0; i < BLOCKS*THREADS; ++i)
    if (h_xs[i] != 0.0)
    {
      printf("Error in validation. xs[%d] = %f\n", i, h_xs[i]);
      break; 
    }


  // run kernel
  double t0 = getTime();
  iteration<<<BLOCKS, THREADS>>>(N, amax, amin, x0max, x0min, d_xs);
  c( cudaGetLastError() );

  c( cudaMemcpy( h_xs, d_xs, sizeof(float)*BLOCKS*THREADS, cudaMemcpyDeviceToHost) );
  c( cudaDeviceSynchronize() );
  double t1 = getTime();

  // check output
  for (int i = 0; i < BLOCKS*THREADS; ++i)
    if (h_xs[i] <= 0.0 || h_xs[i] > 1.0)
    {
      printf("Error in validation. xs[%d] = %f\n", i, h_xs[i]);
      break; 
    }

  double flops = 3.0 * N * BLOCKS * THREADS / (t1 - t0);

  printf("Time: %f, GFLOPS/s = %f\n", (t1 - t0), flops * 1e-9);

  return 0;
}



// vim: ft=cpp
