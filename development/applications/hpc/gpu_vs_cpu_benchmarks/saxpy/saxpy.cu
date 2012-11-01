/********************************************************************
  Simple benchmark program that measures the number of GigaFLOPS/sec
  an NVidia CUDA enabled GPU can perform while doing a SAXPY vector
  operation, i.e. it computes: y = alpha * x + y, where alpha is a
  scalar and x and y are vectors.

  To get an optimistic figure the saxpy is repeated ITER1 times. This
  increases the number of FLOPS per read/written bytes from/to memory.

  Note: The 'nans' in the output are for certain combinations of
     of 'threads' and 'blocks' it skipped.
**/


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <unistd.h>


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

const int ITER1=1<<10;
const int ITER2=1<<3;

__global__ void saxpy( int n, float a, float *x, float *y)
{
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  
  for (int j = 1; j <= ITER1; ++j)
    y[i] = y[i] + j * a * x[i];
}


double measureSaxpy( const int N, const int THREADS)
{
  if (N / THREADS >= 1 << 16 || THREADS > N || THREADS > 1 << 9)
    return NAN;

  float *x, *y, a;
  float *d_x, *d_y;

  // allocate memory
  x = (float *) malloc(sizeof(float)*N);
  y = (float *) malloc(sizeof(float)*N);

  c( cudaMalloc(&d_x, sizeof(float)*N));
  c( cudaMalloc(&d_y, sizeof(float)*N));


  // initialize memory
  a = 2.0/3.0;
  for (int i = 0; i < N; ++i)
  {
    x[i] = rand() / (RAND_MAX + 1.0);
    y[i] = rand() / (RAND_MAX + 1.0);
  }

  // start timer
  cudaDeviceSynchronize();
  double t0 = getTime();

  // copy to device
  c( cudaMemcpy(d_x, x, sizeof(float)*N, cudaMemcpyHostToDevice) );
  c( cudaMemcpy(d_y, y, sizeof(float)*N, cudaMemcpyHostToDevice) );

  // run kernel
  for (int i = 0; i < ITER2; ++i)
  {
    saxpy<<< N/THREADS, THREADS>>>(N, a, d_x, d_y);
    cudaError_t error = cudaGetLastError();
    if ( error != cudaSuccess)
    {
      fprintf(stderr, "Kernel lanch failure: %s\n", cudaGetErrorString(error));
      fprintf(stderr, "Kernel arguments were: BLOCKS=%d, THREADS=%d\n", N/THREADS, THREADS);
      break;
    }
  }

  // copy result back
  c( cudaMemcpy(y, d_y, sizeof(float)*N, cudaMemcpyDeviceToHost) );

  // synchronize and stop timer
  cudaDeviceSynchronize();
  double t3 = getTime();

  // print some statistic based on x & y;
  double sum = 0.0;
  for (int i = 0 ; i < N; ++i)
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
  c( cudaFree(d_x) );
  c( cudaFree(d_y) );

  //c( cudaStreamDestroy(stream) );

  return flops;
}

void showDeviceProps(int dev)
{
  cudaDeviceProp props;
  c( cudaGetDeviceProperties(&props, dev) );

  printf("==== DEVICE PROPERTIES: %d ======\n", dev);
  printf(" . asyncEngineCount = %d\n", props.asyncEngineCount);
  printf("=================================\n");
}

int main(int argc, char ** argv)
{
  c( cudaDeviceReset() );
  c( cudaSetDeviceFlags( cudaDeviceScheduleSpin ) );

  cudaGetLastError(); // clear cuda errors;

  const int MINN=1<<7;
  const int MAXN=1<<17;
  const int MAXTHREADS=1<<9;
  printf("% 10s  % 10s  % 10s\n", "N", "Threads", "GFLOPS/s");
  for(int i = MINN; i <= MAXN; i*=2)
  {
    for (int j = 1; j <= MAXTHREADS; j*=2)
      printf("% 10d  % 10d  % 10f\n", i, j, measureSaxpy(i, j) * 1e-09);
  }

  return 0;
}


// vim: ft=cpp:ai
