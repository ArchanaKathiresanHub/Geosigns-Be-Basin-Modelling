#include "../src/SimdTraits.h"
#include "../src/SimdInstruction.h"
#include "../src/AlignedDenseMatrix.h"
#include <gtest/gtest.h>
#include <stdlib.h>


#ifdef _WIN32
double drand48 () {
   return static_cast<double>(rand ()) / static_cast<double>(RAND_MAX+1);
}
#endif

using namespace Numerics;

// Fill the matrix with random values.
void randomise ( AlignedDenseMatrix& mat );


TEST ( MatrixTest, ConstructionTest ) {

   static const Numerics::SimdInstructionTechnology SimdUsed = Numerics::CurrentSimdTechnology;

   typedef Numerics::SimdTraits<SimdUsed> SimdTraits;
   typedef Numerics::SimdInstruction<SimdUsed> SimdInstruction;

   const int n1 = 9;
   const int n2 = 10;
   AlignedDenseMatrix mat1 ( n1, n2 );
   AlignedDenseMatrix mat2 ( n2, n1 );

   int i;

   // The columns of each matrix should be aligned on the correct address boundary.
   for ( i = 0; i < n2; ++i ) {
      EXPECT_EQ ((long long)(mat1.getColumn ( i )) % SimdInstruction::Alignment, 0 );
   }

   for ( i = 0; i < n1; ++i ) {
      EXPECT_EQ ((long long)(mat2.getColumn ( i )) % SimdInstruction::Alignment, 0 );
   }

   // The leading dimension should be a multiple of the stride.
   EXPECT_EQ ( mat1.leadingDimension () % SimdTraits::DoubleStride, 0 );
   EXPECT_EQ ( mat2.leadingDimension () % SimdTraits::DoubleStride, 0 );

   EXPECT_EQ ( mat1.rows (), mat2.cols ());
   EXPECT_EQ ( mat1.cols (), mat2.rows ());

   mat2 = mat1;

   EXPECT_EQ ( mat2.leadingDimension () % SimdTraits::DoubleStride, 0 );
   EXPECT_EQ ( mat1.rows (), mat2.rows ());
   EXPECT_EQ ( mat1.rows (), mat2.rows ());
   EXPECT_EQ ( mat1.leadingDimension (), mat2.leadingDimension ());

   // The columns of the matrix should be aligned on the correct address boundary.
   for ( i = 0; i < n2; ++i ) {
      EXPECT_EQ ((long long)(mat2.getColumn ( i )) % SimdInstruction::Alignment, 0 );
   }


}

TEST ( MatrixTest, CopyTest01 ) {

   static const Numerics::SimdInstructionTechnology SimdUsed = Numerics::CurrentSimdTechnology;

   typedef Numerics::SimdTraits<SimdUsed> SimdTraits;
   typedef Numerics::SimdInstruction<SimdUsed> SimdInstruction;

   const int n1 = 8;
   const int n2 = 27;
   AlignedDenseMatrix mat1 ( n1, n2 );
   AlignedDenseMatrix mat2 ( n2, n1 );

   int i;
   int j;

   for ( i = 1; i <= 5; ++i ) drand48 ();

   randomise ( mat1 );
   randomise ( mat2 );

   // Assign mat1 to mat2.
   mat2 = mat1;

   // Now the two matrices should be the same.
   for ( i = 0; i < n1; ++i ) {

      for ( j = 0; j < n2; ++j ) {
         EXPECT_EQ ( mat1 ( i, j ), mat2 ( i, j ));
      }
   }

}

TEST ( MatrixTest, TransposeTest01 ) {

   static const Numerics::SimdInstructionTechnology SimdUsed = Numerics::CurrentSimdTechnology;

   typedef Numerics::SimdTraits<SimdUsed> SimdTraits;
   typedef Numerics::SimdInstruction<SimdUsed> SimdInstruction;

   const int n1 = 8;
   const int n2 = 27;
   AlignedDenseMatrix mat1 ( n1, n2 );
   AlignedDenseMatrix mat2 ( n2, n1 );

   int i;
   int j;

   for ( i = 1; i <= 5; ++i ) drand48 ();

   randomise ( mat1 );
   randomise ( mat2 );

   // mat2 should be overwritten with the transpose of mat1
   Numerics::transpose ( mat1, mat2 );

   for ( i = 0; i < n1; ++i ) {

      for ( j = 0; j < n2; ++j ) {
         EXPECT_EQ ( mat1 ( i, j ), mat2 ( j, i ));
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
