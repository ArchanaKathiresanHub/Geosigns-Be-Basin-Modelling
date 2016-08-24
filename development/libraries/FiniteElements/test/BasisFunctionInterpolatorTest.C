#include "PropertiesToQuadPts.h"
#include "FiniteElementTypes.h"
#include "AlignedDenseMatrix.h"
#include "BasisFunctionInterpolator.h"
#include <gtest/gtest.h>
#include <algorithm>
#include <cmath>

//
// In all of the following tests the number of quadrature points computed by nx^2 * nz
// Where nx = 2, 3, 4. and nz = 3, 4, 5.
// Plus several tests that test the interpolation algorithm for numbers of quadrature points
// that cannot occur.
// Number of properties: 9, 10, 11, 13
//
// The number of degrees of freedom (Dofs) will always be 8, the number of nodes in the element.


const int NumberOfDofs = 8;
const double NumberOfDofsDouble = static_cast<double>( NumberOfDofs );


void doTest ( const int NumberOfQuadPoints,
              const int NumberOfProperties ) {

   Numerics::AlignedDenseMatrix basisFunctionsTranspose ( NumberOfQuadPoints, NumberOfDofs );
   Numerics::AlignedDenseMatrix propertyVectors ( NumberOfDofs, NumberOfProperties );
   Numerics::AlignedDenseMatrix interpolatedProperties ( NumberOfQuadPoints, NumberOfProperties );

   basisFunctionsTranspose.fill ( 1.0 );
   propertyVectors.fill ( 1.0 );
   interpolatedProperties.fill ( 1.0 );

   FiniteElementMethod::BasisFunctionInterpolator::compute ( basisFunctionsTranspose, propertyVectors, interpolatedProperties );

   for ( int i = 0; i < NumberOfQuadPoints; ++i ) {

      for ( int j = 0; j < NumberOfProperties; ++j ) {
         EXPECT_DOUBLE_EQ ( interpolatedProperties ( i, j ), NumberOfDofsDouble );
      }

   }

}


TEST ( BasisFunctionInterpolator, Test_12x10 ) {
   // Number of rows mod 8 = 4
   // Number of rows mod 4 = 0
   // Leading dimension - rows = 0

   const int NumberOfQuadPoints = 12;
   const int NumberOfProperties = 10;
   doTest ( NumberOfQuadPoints, NumberOfProperties );
}


TEST ( BasisFunctionInterpolator, Test_13x10 ) {
   // Number of rows mod 8 = 5
   // Number of rows mod 4 = 1
   // Leading dimension - rows = 1

   const int NumberOfQuadPoints = 13;
   const int NumberOfProperties = 10;
   doTest ( NumberOfQuadPoints, NumberOfProperties );
}


TEST ( BasisFunctionInterpolator, Test_14x10 ) {
   // Number of rows mod 8 = 6
   // Number of rows mod 4 = 2
   // Leading dimension - rows = 2

   const int NumberOfQuadPoints = 14;
   const int NumberOfProperties = 10;
   doTest ( NumberOfQuadPoints, NumberOfProperties );
}


TEST ( BasisFunctionInterpolator, Test_15x10 ) {
   // Number of rows mod 8 = 7
   // Number of rows mod 4 = 3
   // Leading dimension - rows = 3

   const int NumberOfQuadPoints = 15;
   const int NumberOfProperties = 10;
   doTest ( NumberOfQuadPoints, NumberOfProperties );
}


TEST ( BasisFunctionInterpolator, Test_36x11 ) {

   const int NumberOfQuadPoints = 36;
   const int NumberOfProperties = 11;
   doTest ( NumberOfQuadPoints, NumberOfProperties );

}

TEST ( BasisFunctionInterpolator, Test_45x9 ) {

   const int NumberOfQuadPoints = 45;
   const int NumberOfProperties = 9;
   doTest ( NumberOfQuadPoints, NumberOfProperties );

}

TEST ( BasisFunctionInterpolator, Test_45x13 ) {

   const int NumberOfQuadPoints = 45;
   const int NumberOfProperties = 13;
   doTest ( NumberOfQuadPoints, NumberOfProperties );

}

TEST ( BasisFunctionInterpolator, Test_80x13 ) {

   const int NumberOfQuadPoints = 80;
   const int NumberOfProperties = 13;
   doTest ( NumberOfQuadPoints, NumberOfProperties );

}
