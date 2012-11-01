/********************************************************************
  Simple benchmark program that measures the number of GigaFLOPS/sec
  an NVidia CUDA enabled GPU can perform while doing a SAXPY vector
  operation, i.e. it computes: y = alpha * x + y, where alpha is a
  scalar and x and y are vectors.

  The SAXPY uses cuBLAS: A CUDA accelerated BLAS library.
**/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <cublas_v2.h>

#include "gettime.h"

void c(cublasStatus_t errorcode)
{
  if (errorcode != CUBLAS_STATUS_SUCCESS)
    fprintf(stderr, "A cuBLAS error occurred\n");
}

#define d(errorcode) \
  do \
  {\
    if (errorcode != cudaSuccess) \
      fprintf(stderr, "A CUDA error on line %d: %s\n", __LINE__, cudaGetErrorString(errorcode)); \
  } \
  while(0) 



double measureSaxpy( cublasHandle_t handle, const int N)
{
  const long WORK = 1 << 25;
  const long ITER= WORK / N;
  float *x, *y;
  float *d_x, *d_y, *d_a;

  // allocate memory
  x = (float * ) malloc(sizeof(float)*N);
  y = (float *) malloc(sizeof(float)*N);

  d( cudaMalloc(&d_x, sizeof(float)*N) );
  d( cudaMalloc(&d_y, sizeof(float)*N) );
  d( cudaMalloc(&d_a, sizeof(float)*1) );


  // initialize memory
  const float a = 2.0/3.0;
  for (int i = 0; i < N; ++i)
  {
    x[i] = rand() / (RAND_MAX + 1.0);
    y[i] = rand() / (RAND_MAX + 1.0);
  }

  // create a command stream
  cudaStream_t stream;
  c( cublasGetStream(handle, &stream) );

  // copy to device
  double t0 = getTime();
  c( cublasSetVector(N, sizeof(float), x, 1, d_x, 1) );
  c( cublasSetVector(N, sizeof(float), y, 1, d_y, 1) );
  c( cublasSetVector(1, sizeof(float), &a, 1, d_a, 1) );

  // syncronize and start timer
  double t1 = getTime();

  // run kernel
  for (long i = 0; i < ITER; ++i)
    c( cublasSaxpy(handle, N, d_a, d_x, 1, d_y, 1) );
  
  // copy result back
  c( cublasGetVector(N, sizeof(float), d_y, 1, y, 1) );
  double t3 = getTime();

  // print some statistic based on x & y;
  double sum = 0.0;
  for (long i = 0 ; i < N; ++i)
    sum += y[i];
  double avg= sum / N;

  const double epsilon = 0.1;
  const double expected = (0.5 + ITER * 0.5 * a);
  if (fabs( avg - expected ) / expected > epsilon)
    fprintf(stderr, "High deviation: avg = %f, expected = %f\n", avg, expected);

  // print performance
  double flops= (double) ITER * N*2 / (t3 - t1);

  // force output of a silly statistic

  // release memory
  free(x);
  free(y);
  d( cudaFree(d_x) );
  d( cudaFree(d_y) );

  return flops;
}

int main(int argc, char ** argv)
{
  cublasHandle_t handle;
  c( cublasCreate(&handle) );
  c( cublasSetPointerMode(handle, CUBLAS_POINTER_MODE_DEVICE));

  const int MINN=1<<7;
  const int MAXN=1<<26;
  printf("#% 9s  % 10s\n", "N", "GFLOPS/s");
  for(int i = MINN; i <= MAXN; i*=2)
  {
    printf("% 10d  % 10f\n", i, measureSaxpy(handle, i) * 1e-09);
  }

  c( cublasDestroy(handle) );

  return 0;
}


// vim: ft=cpp:cindent
