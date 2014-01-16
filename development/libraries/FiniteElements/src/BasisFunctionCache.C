#include "BasisFunctionCache.h"
#include "BasisFunction.h"
#include "FiniteElementTypes.h"
#include "Quadrature.h"

FiniteElementMethod::BasisFunctionCache::BasisFunctionCache ( const int xPointCount,
                                                              const int yPointCount,
                                                              const int zPointCount ) {
   GradElementVector gradPhi;
   ElementVector phi;
   BasisFunction evaluateBasisFunctions;
   
   int i;
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

   m_numberOfPointsX = xPointCount;
   m_numberOfPointsY = yPointCount;
   m_numberOfPointsZ = zPointCount;

   m_numberOfQuadraturePoints = xPointCount * yPointCount * zPointCount;

   m_basisFunctions.resize ( 8, m_numberOfQuadraturePoints );
   m_gradBasisFunctions.resize ( 8, 3 * m_numberOfQuadraturePoints );

   quadCount = 0;

   for ( int i = 0; i < xPointCount; ++i ) {

      for ( int j = 0; j < yPointCount; ++j ) {

         for ( int k = 0; k < zPointCount; ++k ) {

            evaluateBasisFunctions ( xPoints [ i ], yPoints [ j ], zPoints [ k ], phi, gradPhi );

            // Copy basis and grad-basis to correct matrices.
            for ( int l = 0; l < 8; ++l ) {

               m_basisFunctions ( l, quadCount ) = phi ( l + 1 );

               m_gradBasisFunctions ( l, 3 * quadCount     ) = gradPhi ( l + 1, 1 );
               m_gradBasisFunctions ( l, 3 * quadCount + 1 ) = gradPhi ( l + 1, 2 );
               m_gradBasisFunctions ( l, 3 * quadCount + 2 ) = gradPhi ( l + 1, 3 );

            }

            ++quadCount;
         }

      }

   }



}
