#include "../src/SimdTraits.h"
#include "../src/SimdInstruction.h"
#include "../src/AlignedDenseMatrix.h"
#include <gtest/gtest.h>
#include <stdlib.h>
#include <iostream>

#ifdef _WIN32
double drand48 () {
   return static_cast<double>(rand ()) / static_cast<double>(RAND_MAX+1);
}
#endif

using namespace Numerics;

// Fill the matrix with random values.
void randomise ( AlignedDenseMatrix& mat );

TEST ( MatrixTest, MultTest01 ) {

   const int n1 = 8;
   const int n2 = 27;

   // IMPORTANT: For this test to complete this number should be a double precision
   // model number and have not too many significant figures.
   double value = 2.5;
   double expectedValue = value * value * static_cast<double>(n2);

   AlignedDenseMatrix mat1 ( n1, n2 );
   AlignedDenseMatrix mat2 ( n2, n1 );
   AlignedDenseMatrix mat3 ( n1, n1 );

   int i;
   int j;

   mat1.fill ( value );
   mat2.fill ( value );

   Numerics::matmult ( Numerics::NO_TRANSPOSE, Numerics::NO_TRANSPOSE, 1.0, mat1, mat2, 0.0, mat3 );

   // Compare the result of the product with the expected value.
   for ( i = 0; i < n1; ++i ) {

      for ( j = 0; j < n1; ++j ) {
         EXPECT_EQ ( mat3 ( i, j ), expectedValue );
      }

   }

   // Now add the product to mat3 again.
   Numerics::matmult ( Numerics::NO_TRANSPOSE, Numerics::NO_TRANSPOSE, 1.0, mat1, mat2, 1.0, mat3 );

   // Compare the result of the product with the expected value.
   for ( i = 0; i < n1; ++i ) {

      for ( j = 0; j < n1; ++j ) {
         EXPECT_EQ ( mat3 ( i, j ), 2.0 * expectedValue );
      }

   }

}


TEST ( MatrixTest, MultTest02 ) {

   const int n1 = 8;
   const int n2 = 27;

   // IMPORTANT: For this test to complete this number should be a double precision
   // model number and have not too many significant figures.
   double value = 2.5;
   double expectedValue = value * value * static_cast<double>(n2);

   AlignedDenseMatrix mat1 ( n1, n2 );
   AlignedDenseMatrix mat2 ( n1, n2 );
   AlignedDenseMatrix mat3 ( n1, n1 );

   int i;
   int j;

   mat1.fill ( value );
   mat2.fill ( value );

   Numerics::matmult ( Numerics::NO_TRANSPOSE, Numerics::TRANSPOSE, 1.0, mat1, mat2, 0.0, mat3 );

   // Compare the result of the product with the expected value.
   for ( i = 0; i < n1; ++i ) {

      for ( j = 0; j < n1; ++j ) {
         EXPECT_EQ ( mat3 ( i, j ), expectedValue );
      }

   }

   // Now add the product to mat3 again.
   Numerics::matmult ( Numerics::NO_TRANSPOSE, Numerics::TRANSPOSE, 1.0, mat1, mat2, 1.0, mat3 );

   // Compare the result of the product with the expected value.
   for ( i = 0; i < n1; ++i ) {

      for ( j = 0; j < n1; ++j ) {
         EXPECT_EQ ( mat3 ( i, j ), 2.0 * expectedValue );
      }

   }

}


TEST ( MatrixTest, MultTest03 ) {

   const int n1 = 8;
   const int n2 = 27;

   // IMPORTANT: For this test to complete this number should be a double precision
   // model number and have not too many significant figures.
   double value = 2.5;
   double expectedValue = value * value * static_cast<double>(n2);

   AlignedDenseMatrix mat1 ( n2, n1 );
   AlignedDenseMatrix mat2 ( n2, n1 );
   AlignedDenseMatrix mat3 ( n1, n1 );

   int i;
   int j;

   mat1.fill ( value );
   mat2.fill ( value );

   Numerics::matmult ( Numerics::TRANSPOSE, Numerics::NO_TRANSPOSE, 1.0, mat1, mat2, 0.0, mat3 );

   // Compare the result of the product with the expected value.
   for ( i = 0; i < n1; ++i ) {

      for ( j = 0; j < n1; ++j ) {
         EXPECT_EQ ( mat3 ( i, j ), expectedValue );
      }

   }

   // Now add the product to mat3 again.
   Numerics::matmult ( Numerics::TRANSPOSE, Numerics::NO_TRANSPOSE, 1.0, mat1, mat2, 1.0, mat3 );

   // Compare the result of the product with the expected value.
   for ( i = 0; i < n1; ++i ) {

      for ( j = 0; j < n1; ++j ) {
         EXPECT_EQ ( mat3 ( i, j ), 2.0 * expectedValue );
      }

   }

}

TEST ( MatrixTest, MultTest04 ) {

   const int n1 = 8;
   const int n2 = 27;

   // IMPORTANT: For this test to complete this number should be a double precision
   // model number and have not too many significant figures.
   double value = 2.5;
   double expectedValue = value * value * static_cast<double>(n2);

   AlignedDenseMatrix mat1 ( n2, n1 );
   AlignedDenseMatrix mat2 ( n1, n2 );
   AlignedDenseMatrix mat3 ( n1, n1 );

   int i;
   int j;

   mat1.fill ( value );
   mat2.fill ( value );

   Numerics::matmult ( Numerics::TRANSPOSE, Numerics::TRANSPOSE, 1.0, mat1, mat2, 0.0, mat3 );

   // Compare the result of the product with the expected value.
   for ( i = 0; i < n1; ++i ) {

      for ( j = 0; j < n1; ++j ) {
         EXPECT_EQ ( mat3 ( i, j ), expectedValue );
      }

   }

   // Now add the product to mat3 again.
   Numerics::matmult ( Numerics::TRANSPOSE, Numerics::TRANSPOSE, 1.0, mat1, mat2, 1.0, mat3 );

   // Compare the result of the product with the expected value.
   for ( i = 0; i < n1; ++i ) {

      for ( j = 0; j < n1; ++j ) {
         EXPECT_EQ ( mat3 ( i, j ), 2.0 * expectedValue );
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
