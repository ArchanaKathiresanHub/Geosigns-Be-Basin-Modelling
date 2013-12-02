#include "MklElementAssembly.h"

#include <mkl.h>

#include <cassert>
#include <string.h>

#include <xmmintrin.h>

#include <iostream>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
///
// Forward decalration of BLAS fortran subroutines:
//
extern void dgemm_( char *transa, char *transb, int *m, int *n, int *k, double *alpha, double *a, int *lda, double *b, int *ldb, double *beta, double *c, int *ldc );
extern void dgemv_( char *transa, int *m, int *n, double *alpha, double *a, int *lda, double *x, int *incx, double *beta, double *y, int *incy );
extern void dcopy_( int * n, double * x, int * incx, double * y, int * incy );
extern void dscal_( int * n, double * a, double * x, int * incx );
extern double ddot_( int * n, double * x, int * incx, double * y, int * incy );

#ifdef __cplusplus
}
#endif /* __cplusplus */

// Some wrappers to simplify parameters definition for fortran BLAS routines
inline void dgemm(char transa, char transb, int m, int n, int k, double alpha, double *a, int lda, double *b, int ldb, double beta, double *c, int ldc)
{
  dgemm_ ( &transa, &transb, &m, &n, &k, &alpha, a, &lda, b, &ldb, &beta, c, &ldc);
}

inline void dgemv( char transa, int m, int n, double alpha, double *a, int lda, double *x, int incx, double beta, double *y, int incy )
{
   dgemv_( &transa, &m, &n, &alpha, a, &lda, x, &incx, &beta, y, &incy );
}

inline void   dcopy( int n, double * x, int incx, double * y, int incy ) { dcopy_( &n, x, &incx, y, &incy ); }
inline void   dscal( int n, double a, double * x, int incx ) { dscal_( &n, &a, x, &incx ); }
inline double ddot( int n, double * x, int incx, double * y, int incy ) { return ddot_( &n, x, &incx, y, &incy ); }

Mkl::MklNewElementAssembly::MklNewElementAssembly()
{
   // allocate memory for maximum size - 4x4x4 - 64
   N = 4 * 4 * 4;

   // result matrix
   K  = (double *)mkl_malloc( 8 * 8 * sizeof( double ), 64 );  // 8 x 8

   // zero order term
   P  = (double *)mkl_malloc( 8 * N * sizeof( double ), 64 ); // 8 x n
   C  = (double *)mkl_malloc( N *     sizeof( double ), 64 ); // n x n diagonal
   PC = (double *)mkl_malloc( 8 * N * sizeof( double ), 64 ); // 8 x n scaled P by diagondal C

   // first order term
   G  = (double *)mkl_malloc( 8 * 3 * N * sizeof( double ), 64 ); // 8 x 3n
   B  = (double *)mkl_malloc( 3 * N     * sizeof( double ), 64 ); // 3n x n diagonal
   GB = (double *)mkl_malloc( 8 * 3 * N * sizeof( double ), 64 ); // scaled G by diagonal B

   // second order term
   A  = (double *)mkl_malloc( 3 * 3 * N * sizeof( double ), 64 ); // 3n x 3n diagonal
   GA = (double *)mkl_malloc( 8 * 3 * N * sizeof( double ), 64 ); // scaled G by diagonal A 

   assert( P );
   assert( C );
   assert( PC );
   assert( K );
   assert( G );
   assert( B );
   assert( GB );
   assert( A );
   assert( GA );
}

void Mkl::MklNewElementAssembly:: freeMatrices()
{
   if ( !P  ) { mkl_free( P  ); P  = 0; }

   if ( !C  ) { mkl_free( C  ); C  = 0; }
   if ( !PC ) { mkl_free( PC ); PC = 0; }  

   if ( !K  ) { mkl_free( K  ); K  = 0; }

   if ( !G  ) { mkl_free( G  ); G  = 0; }
   if ( !B  ) { mkl_free( B  ); B  = 0; }
   if ( !GB ) { mkl_free( GB ); GB = 0; }

   if ( !A  ) { mkl_free( A  ); A  = 0; }
   N = -1;
}


void Mkl::MklNewElementAssembly::InitAssembly( int xyQuadPts, int zQuadPts, const std::vector<double> & randData )
{
   assert( xyQuadPts < 5 && zQuadPts < 5 );

   N = xyQuadPts * xyQuadPts * zQuadPts;

   size_t ri = 0;

   // column wise order!
   for (    int i = 0; i < N; ++i )
   {
      C[i] = randData[ri++];

      for ( int j = 0; j < 8; ++j ) 
         P[i + j * 8] = randData[ri++];
   }

   for (    int i = 0; i < 8; ++i ) 
      for ( int j = 0; j < 3 * N; ++j )
         G[i * 3 * N + j] = randData[ri++]; // G row wise

   for ( int i = 0; i < 3 * N; ++i ) B[i] = randData[ri++];

   for (       int i = 0; i < 9 * N; i += 9 )
      for (    int k = 0; k < 3; ++k )
         for ( int l = 0; l < 3; ++l )
            A[i + k * 3 + l] = randData[ri++]; // vector of column wise 3x3 matrices
}


void Mkl::MklNewElementAssembly::AssembleElement()
{
   memset( K,  8 * 8 * sizeof( double ), 0 );

   // Calc PC = P * C
   for ( int i = 0; i < 8; ++i )
   {
      dcopy( 8, P + i * 8, 1, PC + i * 8, 1 );
      dscal( 8, C[i], PC + i * 8, 1 );
   }
   
   // Calc K += PC * P' 
   dgemm( 'N', 'T', 8, 8, N, 1.0, PC, 8, P, 8, 0.0, K, 8);

   // first order term
   // calc GB = G * B
   for ( int i = 0; i < 8; ++i )
   {
      for ( int j = 0; j < N; ++j  )
      {
         GB[ i + j * 8 ] = ddot( 3, G + i * 3 * N + j * 3, 1, B + j * 3, 1 );
      }
   }

   // Calc K += GB * P'
   dgemm( 'N', 'T', 8, 8, N, 1.0, GB, 8, P, 8, 1.0, K, 8 );
   
   // Calc GA = G * A; GA column order
   for ( int i = 0; i < 8; ++i )
   {
      for ( int j = 0; j < N; ++j  )
      {
         double resV[3];
         dgemv( 'N', 3, 3, 1.0, A + j * 9, 3, G + i * 3 * N + j * 3, 1, 0.0, resV, 1 );

         for( int k = 0; k < 3; ++k )
            GA[ i + (j*3 + k) * 8] = resV[k];

      }
   }

   // Calc K +=  GA * G'
   dgemm( 'N', 'N', 8, 8, 3 * N, 1.0, GA, 8, G, 3 * N, 1.0, K, 8 );
}


