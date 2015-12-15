#include "mpi.h"

#include "SourceTermInterpolatorCalculator.h"
#include "FiniteElement.h"
#include "FiniteElementTypes.h"
#include "Quadrature.h"
#include "Quadrature3D.h"
#include "ElementContributions.h"
#include "LayerElement.h"
#include "globaldefs.h"
#include "layer.h"

void SourceTermInterpolatorCalculator::compute ( const LayerElement& element,
                                                 const CoefficientArray&  xs,
                                                 MultiInterpolator& interpolator) const {

   int i, j;
   MultiInterpolator::CoefficientMatrix elementMassAdded;

   for (  i = 0; i < NumberOfEquations; ++ i ) {
      for ( j = 0; j <= MultiInterpolator::Degree; ++ j ) {
         elementMassAdded[ i ][ j ] = 0.0;
      }
   }
   if ( element.isActive ()) {
  
      MultiComponentVector<PVTComponents> generated;
      FiniteElementMethod::ElementGeometryMatrix geometryMatrix;
      FiniteElementMethod::FiniteElement finiteElement;
      PVTComponents term;
      PVTComponents computedSourceTerm;
      NumericFunctions::Quadrature3D::Iterator quad;
      NumericFunctions::Quadrature3D::getInstance ().get ( DarcyInterpolatorQuadratureDegree, quad );
      double elementVolume = 0.0;
      double weight;
      double lambda;
      double layerThickness;

      const LayerProps* srLayer = element.getFormation ();

      for ( j = 0; j <= MultiInterpolator::Degree; ++j ) {
         lambda = xs [ j ];
  
         getGeometryMatrix ( element, geometryMatrix, lambda );
          
         finiteElement.setGeometry ( geometryMatrix );
         
         for ( i = 1; i <= 4; ++i ) {
            srLayer->getGenexGenerated ( element.getNodeIPosition ( i - 1 ),
                                         element.getNodeJPosition ( i - 1 ),
                                         generated ( i ));
            
            generated ( i )( pvtFlash::COX ) = 0.0;
            generated ( i )( pvtFlash::H2S ) = 0.0;
            
            layerThickness = srLayer->getCurrentLayerThickness ( element.getNodeIPosition ( i - 1 ),
                                                                 element.getNodeJPosition ( i - 1 ));
            
            if ( layerThickness > DepositingThicknessTolerance ) {
               generated ( i ) *= 1.0 / layerThickness;
            } else {
               generated ( i ).zero ();
            }
            
            generated ( i + 4 ) = generated ( i );
         }
         
         computedSourceTerm.zero ();
        
         for ( quad.initialise (); not quad.isDone (); ++quad ) {
            finiteElement.setQuadraturePoint ( quad.getX (), quad.getY (), quad.getZ ());
            
            weight = determinant ( finiteElement.getJacobian ()) * quad.getWeight ();
            
            term = generated.dot ( finiteElement.getBasis ());
            term *= weight;
            computedSourceTerm += term;
         }
         for (  i = 0; i < NumberOfEquations; ++ i ) {
            pvtFlash::ComponentId component = static_cast<pvtFlash::ComponentId>( i );
            elementMassAdded[ i ][ j ] = computedSourceTerm ( component );
         }
      }
   }
   interpolator.compute ( xs, elementMassAdded );
}

