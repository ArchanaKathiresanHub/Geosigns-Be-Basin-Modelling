#include "../src/SimdTraits.h"
#include "../src/SimdInstruction.h"
#include "../src/AlignedDenseMatrix.h"
#include <gtest/gtest.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>

#ifdef _WIN32
double drand48 () {
   return static_cast<double>(rand ()) / static_cast<double>(RAND_MAX+1);
}

static const double M_SQRT2 = 1.41421356237309504880;
static const double M_E     = 2.7182818284590452354;
static const double M_PI    = 3.14159265358979323846;

#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern void dgemm_(char *transa, char *transb, int *m, int *n, int *k, double *alpha, const double *a, int *lda, const double *b, int *ldb, double *beta, double *c, int *ldc);

#ifdef __cplusplus
}
#endif /* __cplusplus */


inline void dgemm(char transa, char transb, int m, int n, int k, double alpha, const double *a, int lda, const double *b, int ldb, double beta, double *c, int ldc) {
  dgemm_ ( &transa, &transb, &m, &n, &k, &alpha, a, &lda, b, &ldb, &beta, c, &ldc);
}

// Character indicating whether or not the transpose is required.
static const char transChar [ 2 ] = {'N', 'T'};

using namespace Numerics;

// Fill the matrix with random values.
void randomise ( AlignedDenseMatrix& mat );


void blasMatMult ( const MatrixTransposeType transposeA,
                   const MatrixTransposeType transposeB,
                   const double              alpha,
                   const AlignedDenseMatrix& a,
                   const AlignedDenseMatrix& b,
                   const double              beta,
                         AlignedDenseMatrix& c );

//
// Compare results of the matrix-matrix product with that
// obtained from the blas library.
//
TEST ( GeneralMatrixMultiply, NoTransNoTrans ) {

   const int n1 = 8;
   const int n2 = 27;

   AlignedDenseMatrix mat1 ( n1, n2 );
   AlignedDenseMatrix mat2 ( n2, n1 );
   AlignedDenseMatrix mat3 ( n1, n1 );
   AlignedDenseMatrix res  ( n1, n1 );
   double alpha1 = M_SQRT2;
   double alpha2 = M_E;
   double beta  = M_PI;

   int i;
   int j;

   randomise ( mat1 );
   randomise ( mat2 );
   randomise ( mat3 );
   randomise ( res );

   Numerics::matmult ( Numerics::NO_TRANSPOSE, Numerics::NO_TRANSPOSE, alpha1, mat1, mat2, 0.0, mat3 );
   blasMatMult       ( Numerics::NO_TRANSPOSE, Numerics::NO_TRANSPOSE, alpha1, mat1, mat2, 0.0, res );

   // Compare the result of the product with the expected value.
   for ( i = 0; i < n1; ++i ) {

      for ( j = 0; j < n1; ++j ) {
         EXPECT_NEAR ( res ( i, j ), mat3 ( i, j ), res ( i, j ) * 1.0e-12 );
      }

   }

   // Now add the product to mat3 again.
   Numerics::matmult ( Numerics::NO_TRANSPOSE, Numerics::NO_TRANSPOSE, alpha2, mat1, mat2, beta, mat3 );
   blasMatMult       ( Numerics::NO_TRANSPOSE, Numerics::NO_TRANSPOSE, alpha2, mat1, mat2, beta, res );

   // Compare the result of the product with the expected value.
   for ( i = 0; i < n1; ++i ) {

      for ( j = 0; j < n1; ++j ) {
         EXPECT_NEAR( res ( i, j ), mat3 ( i, j ), res ( i, j ) * 1.0e-12 );
      }

   }

}


//
// Compare results of the matrix-transpose(matrix) product with that
// obtained from the blas library.
//
TEST ( GeneralMatrixMultiply, NoTransTrans ) {

   const int n1 = 8;
   const int n2 = 27;

   // IMPORTANT: For this test to complete this number should be a double precision
   // model number and have not too many significant figures.
   double value = 2.5;
   double expectedValue = value * value * static_cast<double>(n2);

   AlignedDenseMatrix mat1 ( n1, n2 );
   AlignedDenseMatrix mat2 ( n1, n2 );
   AlignedDenseMatrix mat3 ( n1, n1 );
   AlignedDenseMatrix res  ( n1, n1 );
   double alpha1 = M_SQRT2;
   double alpha2 = M_E;
   double beta  = M_PI;

   int i;
   int j;

   mat1.fill ( value );
   mat2.fill ( value );
   // Fill mat3 with random values in order to test that the assignment in matmult works (beta = 0).
   randomise ( mat3 );

   Numerics::matmult ( Numerics::NO_TRANSPOSE, Numerics::TRANSPOSE, alpha1, mat1, mat2, 0.0, mat3 );
   blasMatMult       ( Numerics::NO_TRANSPOSE, Numerics::TRANSPOSE, alpha1, mat1, mat2, 0.0, res );

   // Compare the result of the product with the expected value.
   for ( i = 0; i < n1; ++i ) {

      for ( j = 0; j < n1; ++j ) {
         EXPECT_NEAR ( res ( i, j ), mat3 ( i, j ), res ( i, j ) * 1.0e-12 );
      }

   }

   // Now add the product to mat3 again.
   Numerics::matmult ( Numerics::NO_TRANSPOSE, Numerics::TRANSPOSE, alpha2, mat1, mat2, beta, mat3 );
   blasMatMult       ( Numerics::NO_TRANSPOSE, Numerics::TRANSPOSE, alpha2, mat1, mat2, beta, res );

   // Compare the result of the product with the expected value.
   for ( i = 0; i < n1; ++i ) {

      for ( j = 0; j < n1; ++j ) {
         EXPECT_NEAR( res ( i, j ), mat3 ( i, j ), res ( i, j ) * 1.0e-12 );
      }

   }

}


//
// Compare results of the transpose(matrix)-matrix product with that
// obtained from the blas library.
//
TEST ( GeneralMatrixMultiply, TransNoTrans ) {

   const int n1 = 8;
   const int n2 = 27;

   // IMPORTANT: For this test to complete this number should be a double precision
   // model number and have not too many significant figures.
   double value = 2.5;
   double expectedValue = value * value * static_cast<double>(n2);

   AlignedDenseMatrix mat1 ( n2, n1 );
   AlignedDenseMatrix mat2 ( n2, n1 );
   AlignedDenseMatrix mat3 ( n1, n1 );
   AlignedDenseMatrix res  ( n1, n1 );
   double alpha1 = M_SQRT2;
   double alpha2 = M_E;
   double beta  = M_PI;

   int i;
   int j;

   mat1.fill ( value );
   mat2.fill ( value );
   // Fill mat3 with random values in order to test that the assignment in matmult works (beta = 0).
   randomise ( mat3 );

   Numerics::matmult ( Numerics::TRANSPOSE, Numerics::NO_TRANSPOSE, alpha1, mat1, mat2, 0.0, mat3 );
   blasMatMult       ( Numerics::TRANSPOSE, Numerics::NO_TRANSPOSE, alpha1, mat1, mat2, 0.0, res );

   // Compare the result of the product with the expected value.
   for ( i = 0; i < n1; ++i ) {

      for ( j = 0; j < n1; ++j ) {
         EXPECT_NEAR ( res ( i, j ), mat3 ( i, j ), res ( i, j ) * 1.0e-12 );
      }

   }

   // Now add the product to mat3 again.
   Numerics::matmult ( Numerics::TRANSPOSE, Numerics::NO_TRANSPOSE, alpha2, mat1, mat2, beta, mat3 );
   blasMatMult       ( Numerics::TRANSPOSE, Numerics::NO_TRANSPOSE, alpha2, mat1, mat2, beta, res );

   // Compare the result of the product with the expected value.
   for ( i = 0; i < n1; ++i ) {

      for ( j = 0; j < n1; ++j ) {
         EXPECT_NEAR( res ( i, j ), mat3 ( i, j ), res ( i, j ) * 1.0e-12 );
      }

   }

}

//
// Compare results of the transpose(matrix)-transpose(matrix) product with that
// obtained from the blas library.
//
TEST ( GeneralMatrixMultiply, TransTrans ) {

   const int n1 = 8;
   const int n2 = 27;

   // IMPORTANT: For this test to complete this number should be a double precision
   // model number and have not too many significant figures.
   double value = 2.5;
   double expectedValue = value * value * static_cast<double>(n2);

   AlignedDenseMatrix mat1 ( n2, n1 );
   AlignedDenseMatrix mat2 ( n1, n2 );
   AlignedDenseMatrix mat3 ( n1, n1 );
   AlignedDenseMatrix res  ( n1, n1 );
   double alpha1 = M_SQRT2;
   double alpha2 = M_E;
   double beta  = M_PI;

   int i;
   int j;

   mat1.fill ( value );
   mat2.fill ( value );
   // Fill mat3 with random values in order to test that the assignment in matmult works (beta = 0).
   randomise ( mat3 );

   Numerics::matmult ( Numerics::TRANSPOSE, Numerics::TRANSPOSE, alpha1, mat1, mat2, 0.0, mat3 );
   blasMatMult       ( Numerics::TRANSPOSE, Numerics::TRANSPOSE, alpha1, mat1, mat2, 0.0, res );

   // Compare the result of the product with the expected value.
   for ( i = 0; i < n1; ++i ) {

      for ( j = 0; j < n1; ++j ) {
         EXPECT_NEAR ( res ( i, j ), mat3 ( i, j ), res ( i, j ) * 1.0e-12 );
      }

   }

   // Now add the product to mat3 again.
   Numerics::matmult ( Numerics::TRANSPOSE, Numerics::TRANSPOSE, alpha2, mat1, mat2, beta, mat3 );
   blasMatMult       ( Numerics::TRANSPOSE, Numerics::TRANSPOSE, alpha2, mat1, mat2, beta, res );

   // Compare the result of the product with the expected value.
   for ( i = 0; i < n1; ++i ) {

      for ( j = 0; j < n1; ++j ) {
         EXPECT_NEAR( res ( i, j ), mat3 ( i, j ), res ( i, j ) * 1.0e-12 );
      }

   }

}


void randomise ( AlignedDenseMatrix& mat ) {

   int i;
   int j;

   for ( j = 0; j < mat.cols (); ++j ) {

      for ( i = 0; i < mat.rows (); ++i ) {
         mat ( i, j ) = drand48 ();
      }

   }

}

void blasMatMult ( const MatrixTransposeType transposeA,
                   const MatrixTransposeType transposeB,
                   const double              alpha,
                   const AlignedDenseMatrix& a,
                   const AlignedDenseMatrix& b,
                   const double              beta,
                         AlignedDenseMatrix& c ) {

   static const char transChar [ 2 ] = {'N', 'T'};

   int m;
   int n;
   int k;

   if ( transposeA == NO_TRANSPOSE ) {
      m = a.rows ();
      k = a.cols ();
   } else {
      m = a.cols ();
      k = a.rows ();
   }

   if ( transposeB == NO_TRANSPOSE ) {
      n = b.cols ();
   } else {
      n = b.rows ();
   }

   dgemm ( transChar [ transposeA ], transChar [ transposeB ], 
           m, n, k, alpha,
           a.data (), a.leadingDimension (),
           b.data (), b.leadingDimension (),
           beta, c.data (), c.leadingDimension ());
}
