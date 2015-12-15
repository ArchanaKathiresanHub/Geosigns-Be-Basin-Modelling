#include "BasisFunctionCache.h"
#include <gtest/gtest.h>

#include "BasisFunction.h"
#include "FiniteElementTypes.h"
#include "Quadrature.h"

using namespace FiniteElementMethod;


TEST( BasisCache, DefaultTest )
{

   int xPointCount = 2;
   int yPointCount = 2;
   int zPointCount = 3;
   int total = xPointCount * yPointCount * zPointCount;
   int quadCount;


   // The weights are not used here.
   NumericFunctions::Quadrature::QuadratureArray xPoints;
   NumericFunctions::Quadrature::QuadratureArray xWeights;

   NumericFunctions::Quadrature::QuadratureArray yPoints;
   NumericFunctions::Quadrature::QuadratureArray yWeights;

   NumericFunctions::Quadrature::QuadratureArray zPoints;
   NumericFunctions::Quadrature::QuadratureArray zWeights;

   NumericFunctions::Quadrature::getInstance ().getGaussLegendreQuadrature ( xPointCount, xPoints, xWeights );
   NumericFunctions::Quadrature::getInstance ().getGaussLegendreQuadrature ( yPointCount, yPoints, yWeights );
   NumericFunctions::Quadrature::getInstance ().getGaussLegendreQuadrature ( zPointCount, zPoints, zWeights );


   GradElementVector gradPhi;
   ElementVector phi;
   BasisFunctionCache bases ( xPointCount, yPointCount, zPointCount );
   BasisFunction evaluateBasisFunctions;

   // Check that the sizes are the same.
   EXPECT_EQ ( xPointCount, bases.getNumberOfPointsX ());
   EXPECT_EQ ( yPointCount, bases.getNumberOfPointsY ());
   EXPECT_EQ ( zPointCount, bases.getNumberOfPointsZ ());
   EXPECT_EQ ( total, bases.getNumberOfQuadraturePoints ());

   quadCount = 0;

   // Check that the values are the same.
   for ( int i = 0; i < xPointCount; ++i ) {

      for ( int j = 0; j < yPointCount; ++j ) {

         for ( int k = 0; k < zPointCount; ++k ) {

            evaluateBasisFunctions ( xPoints [ i ], yPoints [ j ], zPoints [ k ], phi, gradPhi );

            for ( int l = 0; l < 8; ++l ) {

               EXPECT_DOUBLE_EQ ( bases.getBasisFunctions ()( l, quadCount ), phi ( l + 1 ));

               EXPECT_DOUBLE_EQ ( bases.getGradBasisFunctions ()( l, 3 * quadCount     ), gradPhi ( l + 1, 1 ));
               EXPECT_DOUBLE_EQ ( bases.getGradBasisFunctions ()( l, 3 * quadCount + 1 ), gradPhi ( l + 1, 2 ));
               EXPECT_DOUBLE_EQ ( bases.getGradBasisFunctions ()( l, 3 * quadCount + 2 ), gradPhi ( l + 1, 3 ));
            }

            ++quadCount;
         }

      }

   }

}
