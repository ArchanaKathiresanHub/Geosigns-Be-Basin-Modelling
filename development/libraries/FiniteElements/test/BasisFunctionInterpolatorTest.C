#include "PropertiesToQuadPts.h"
#include "FiniteElementTypes.h"
#include "AlignedDenseMatrix.h"
#include "BasisFunctionInterpolator.h"
#include "CpuInfo.h"
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



void doSimpleTest ( const int NumberOfQuadPoints,
                    const int NumberOfProperties,
                    const double value ) {

   const int NumberOfDofs = 8;
   const double NumberOfDofsDouble = static_cast<double>( NumberOfDofs );

   Numerics::AlignedDenseMatrix basisFunctionsTranspose ( NumberOfQuadPoints, NumberOfDofs );
   Numerics::AlignedDenseMatrix propertyVectors ( NumberOfDofs, NumberOfProperties );
   Numerics::AlignedDenseMatrix interpolatedProperties ( NumberOfQuadPoints, NumberOfProperties );
   FiniteElementMethod::BasisFunctionInterpolator interpolator;
   cpuInfo cpuinfo;
   

   basisFunctionsTranspose.fill ( value );
   propertyVectors.fill ( 1.0 / value );
   interpolatedProperties.fill ( 1.0 );

   interpolator.compute( basisFunctionsTranspose, propertyVectors, interpolatedProperties, cpuinfo );

   for ( int i = 0; i < NumberOfQuadPoints; ++i ) {

      for ( int j = 0; j < NumberOfProperties; ++j ) {
         EXPECT_NEAR ( interpolatedProperties ( i, j ), NumberOfDofsDouble, 1.0e-12 );
      }

   }

}


TEST ( BasisFunctionInterpolator, Test_12x10 ) {
   // Number of rows mod 8 = 4
   // Number of rows mod 4 = 0
   // Leading dimension - rows = 0

   const int NumberOfQuadPoints = 12;
   const int NumberOfProperties = 10;
   doSimpleTest ( NumberOfQuadPoints, NumberOfProperties, 2.0 );
}


TEST ( BasisFunctionInterpolator, Test_13x10 ) {
   // Number of rows mod 8 = 5
   // Number of rows mod 4 = 1
   // Leading dimension - rows = 1

   const int NumberOfQuadPoints = 13;
   const int NumberOfProperties = 10;
   doSimpleTest ( NumberOfQuadPoints, NumberOfProperties, 4.0 );
}


TEST ( BasisFunctionInterpolator, Test_14x10 ) {
   // Number of rows mod 8 = 6
   // Number of rows mod 4 = 2
   // Leading dimension - rows = 2

   const int NumberOfQuadPoints = 14;
   const int NumberOfProperties = 10;
   doSimpleTest ( NumberOfQuadPoints, NumberOfProperties, 0.125 );
}


TEST ( BasisFunctionInterpolator, Test_15x10 ) {
   // Number of rows mod 8 = 7
   // Number of rows mod 4 = 3
   // Leading dimension - rows = 3

   const int NumberOfQuadPoints = 15;
   const int NumberOfProperties = 10;
   doSimpleTest ( NumberOfQuadPoints, NumberOfProperties, 7.0 );
}


TEST ( BasisFunctionInterpolator, Test_36x11 ) {

   const int NumberOfQuadPoints = 36;
   const int NumberOfProperties = 11;
   doSimpleTest ( NumberOfQuadPoints, NumberOfProperties, 11.234 );

}

TEST ( BasisFunctionInterpolator, Test_45x9 ) {

   const int NumberOfQuadPoints = 45;
   const int NumberOfProperties = 9;
   doSimpleTest ( NumberOfQuadPoints, NumberOfProperties, 0.2334 );

}

TEST ( BasisFunctionInterpolator, Test_45x13 ) {

   const int NumberOfQuadPoints = 45;
   const int NumberOfProperties = 13;
   doSimpleTest ( NumberOfQuadPoints, NumberOfProperties, 1.0 );

}

TEST ( BasisFunctionInterpolator, Test_80x13 ) {

   const int NumberOfQuadPoints = 80;
   const int NumberOfProperties = 13;
   doSimpleTest ( NumberOfQuadPoints, NumberOfProperties, 123.0987 );

}
