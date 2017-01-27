#include <iostream>
using namespace std;
#include "JacobianStorage.h"
#include "BasisFunctionCache.h"
#include "FiniteElementTypes.h"
#include <gtest/gtest.h>

using namespace FiniteElementMethod;

TEST ( JacobianStorage, SimpleTest )
{

   const int NumberOfXPoints = 2;
   const int NumberOfYPoints = 2;
   const int NumberOfZPoints = 3;

   const double DeltaX = 100.0;
   const double DeltaY = DeltaX;
   const double DeltaZ = DeltaX;

   BasisFunctionCache bfc ( NumberOfXPoints, NumberOfYPoints, NumberOfZPoints );
   JacobianStorage js ( bfc.getNumberOfQuadraturePoints ());
   ElementVector depths;

   depths ( 1 ) = 0.0;
   depths ( 2 ) = 0.0;
   depths ( 3 ) = 0.0;
   depths ( 4 ) = 0.0;

   depths ( 5 ) = DeltaZ;
   depths ( 6 ) = DeltaZ;
   depths ( 7 ) = DeltaZ;
   depths ( 8 ) = DeltaZ;

   js.set ( DeltaX, DeltaY, depths, bfc.getGradBasisFunctions ());

   EXPECT_NEAR ( js.getJacobian11 (), 0.5 * DeltaX, 1.0e-12 );
   EXPECT_NEAR ( js.getJacobian22 (), 0.5 * DeltaY, 1.0e-12 );

   for ( int i = 0; i < bfc.getNumberOfQuadraturePoints (); ++i ) {
      EXPECT_NEAR ( js.getJacobian3 ( i )( 1 ), 0.0, 1.0e-12 );
      EXPECT_NEAR ( js.getJacobian3 ( i )( 2 ), 0.0, 1.0e-12 );
      EXPECT_NEAR ( js.getJacobian3 ( i )( 3 ), 0.5 * DeltaZ, 1.0e-12 );
   }


   EXPECT_NEAR ( js.getInverseJacobian11 (), 2.0 / DeltaX, 1.0e-12 );
   EXPECT_NEAR ( js.getInverseJacobian22 (), 2.0 / DeltaY, 1.0e-12 );

   for ( int i = 0; i < bfc.getNumberOfQuadraturePoints (); ++i ) {
      EXPECT_NEAR ( js.getInverseJacobian3 ( i )( 1 ), 0.0, 1.0e-12 );
      EXPECT_NEAR ( js.getInverseJacobian3 ( i )( 2 ), 0.0, 1.0e-12 );
      EXPECT_NEAR ( js.getInverseJacobian3 ( i )( 3 ), 2.0 / DeltaZ, 1.0e-12 );
   }

   for ( int i = 0; i < bfc.getNumberOfQuadraturePoints (); ++i ) {
      EXPECT_NEAR ( js.getDeterminant ( i ), 0.125 * DeltaX * DeltaY * DeltaZ, 1.0e-9 );
   }


}


TEST ( JacobianStorage, RectangleTest )
{

   const int NumberOfXPoints = 2;
   const int NumberOfYPoints = 2;
   const int NumberOfZPoints = 3;

   const double DeltaX = 100.0;
   const double DeltaY = 200;
   const double DeltaZ = DeltaX;

   BasisFunctionCache bfc ( NumberOfXPoints, NumberOfYPoints, NumberOfZPoints );
   JacobianStorage js ( bfc.getNumberOfQuadraturePoints ());
   ElementVector depths;

   depths ( 1 ) = 0.0;
   depths ( 2 ) = 0.0;
   depths ( 3 ) = 0.0;
   depths ( 4 ) = 0.0;

   depths ( 5 ) = DeltaZ;
   depths ( 6 ) = DeltaZ;
   depths ( 7 ) = DeltaZ;
   depths ( 8 ) = DeltaZ;

   js.set ( DeltaX, DeltaY, depths, bfc.getGradBasisFunctions ());

   EXPECT_NEAR ( js.getJacobian11 (), 0.5 * DeltaX, 1.0e-12 );
   EXPECT_NEAR ( js.getJacobian22 (), 0.5 * DeltaY, 1.0e-12 );

   for ( int i = 0; i < bfc.getNumberOfQuadraturePoints (); ++i ) {
      EXPECT_NEAR ( js.getJacobian3 ( i )( 1 ), 0.0, 1.0e-12 );
      EXPECT_NEAR ( js.getJacobian3 ( i )( 2 ), 0.0, 1.0e-12 );
      EXPECT_NEAR ( js.getJacobian3 ( i )( 3 ), 0.5 * DeltaZ, 1.0e-12 );
   }


   EXPECT_NEAR ( js.getInverseJacobian11 (), 2.0 / DeltaX, 1.0e-12 );
   EXPECT_NEAR ( js.getInverseJacobian22 (), 2.0 / DeltaY, 1.0e-12 );

   for ( int i = 0; i < bfc.getNumberOfQuadraturePoints (); ++i ) {
      EXPECT_NEAR ( js.getInverseJacobian3 ( i )( 1 ), 0.0, 1.0e-12 );
      EXPECT_NEAR ( js.getInverseJacobian3 ( i )( 2 ), 0.0, 1.0e-12 );
      EXPECT_NEAR ( js.getInverseJacobian3 ( i )( 3 ), 2.0 / DeltaZ, 1.0e-12 );
   }

   for ( int i = 0; i < bfc.getNumberOfQuadraturePoints (); ++i ) {
      EXPECT_NEAR ( js.getDeterminant ( i ), 0.125 * DeltaX * DeltaY * DeltaZ, 1.0e-9 );
   }


}
