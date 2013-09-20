#include "mpi.h"
#include "PoreVolumeInterpolatorCalculator.h"
#include "Lithology.h"

#include "FiniteElement.h"
#include "FiniteElementTypes.h"
#include "Quadrature.h"
#include "Quadrature3D.h"
#include "property_manager.h"
#include "ElementContributions.h"
#include "LayerElement.h"
#include "globaldefs.h"


void PoreVolumeInterpolatorCalculator::compute ( const SubdomainElement&  element,
                                                 const CoefficientArray&  xs,
                                                       MultiInterpolator& interpolator ) const {

   const Lithology* lithology = element.getLayerElement ().getLithology ();

   FiniteElementMethod::ElementGeometryMatrix geometryMatrix;

   FiniteElementMethod::ElementVector         startVes;
   FiniteElementMethod::ElementVector         endVes;
   FiniteElementMethod::ElementVector         ves;
   FiniteElementMethod::ElementVector         ves2;

   FiniteElementMethod::ElementVector         maxVes;
   FiniteElementMethod::ElementVector         maxVes2;
   FiniteElementMethod::ElementVector         startMaxVes;
   FiniteElementMethod::ElementVector         endMaxVes;

   FiniteElementMethod::FiniteElement         finiteElement;

   MultiInterpolator::CoefficientMatrix   rhsValues;

   double porosity;
   double porosity2;
   double lambda;
   double initialVes;
   double finalVes;
   double deltaLambda = 0.01;
   // We are decreasing time down to 0 (present day).
   // Since ves changes linearly the wrt time, its derivative is constant.
   double vesDerivativeWrtTime = 1.0;// / ( startTime - endTime );
   double porosityDerivativeWrtVes;
   int i;

   NumericFunctions::Quadrature3D::Iterator quad;
   NumericFunctions::Quadrature3D::getInstance ().get ( DarcyInterpolatorQuadratureDegree, quad );

   getPreviousCoefficients ( element.getLayerElement (), Basin_Modelling::VES_FP,  startVes );
   getPreviousCoefficients ( element.getLayerElement (), Basin_Modelling::Max_VES, startMaxVes );

   getCoefficients ( element.getLayerElement (), Basin_Modelling::VES_FP,  endVes );
   getCoefficients ( element.getLayerElement (), Basin_Modelling::Max_VES, endMaxVes );

   for ( i = 0; i <= MultiInterpolator::Degree; ++i ) {
      lambda = xs [ i ];
      rhsValues [ 0 ][ i ] = 0.0;
      rhsValues [ 1 ][ i ] = 0.0;

      getGeometryMatrix ( element.getLayerElement (), geometryMatrix, lambda );
      finiteElement.setGeometry ( geometryMatrix );

      interpolate ( startVes,    endVes,    lambda, ves );
      interpolate ( startMaxVes, endMaxVes, lambda, maxVes );

      if ( lambda == 0.0 ) {
         interpolate ( startVes,    endVes,    lambda + deltaLambda, ves2 );
         interpolate ( startMaxVes, endMaxVes, lambda + deltaLambda, maxVes2 );
      } else {
         interpolate ( startVes,    endVes,    lambda - deltaLambda, ves2 );
         interpolate ( startMaxVes, endMaxVes, lambda - deltaLambda, maxVes2 );
      }

      for ( quad.initialise (); not quad.isDone (); ++quad ) {
         finiteElement.setQuadraturePoint ( quad.getX (), quad.getY (), quad.getZ ());

         initialVes = finiteElement.interpolate ( startVes );
         finalVes = finiteElement.interpolate ( endVes );

         porosity = lithology->porosity ( finiteElement.interpolate ( ves ),
                                          finiteElement.interpolate ( maxVes ),
                                          false, 0.0 );

         porosity2 = lithology->porosity ( finiteElement.interpolate ( ves2 ),
                                           finiteElement.interpolate ( maxVes2 ),
                                           false, 0.0 );

         // porosityDerivativeWrtVes = lithology->computePorosityDerivativeWrtVes ( finiteElement.interpolate ( ves ),
         //                                                                         finiteElement.interpolate ( maxVes ),
         //                                                                         false, 0.0 );

         // // vesDerivativeWrtTime = initialVes - finalVes;
         // porosityDerivativeWrtVes *= ( finalVes - initialVes ) / ( 1.0 - porosity );


         // Use a left or right derivative depending on if its the first point or not.
         if ( lambda == xs [ 0 ] ) {
            porosityDerivativeWrtVes = ( porosity2 - porosity ) / ( 1.0 - porosity ) / deltaLambda;
         } else {
            porosityDerivativeWrtVes = ( porosity - porosity2 ) / ( 1.0 - porosity ) / deltaLambda;
         }


         rhsValues [ 0 ][ i ] += determinant ( finiteElement.getJacobian ()) * quad.getWeight () * porosity;
         rhsValues [ 1 ][ i ] -= determinant ( finiteElement.getJacobian ()) * quad.getWeight () * porosityDerivativeWrtVes;
      }

   }

   interpolator.compute ( xs, rhsValues );
}
