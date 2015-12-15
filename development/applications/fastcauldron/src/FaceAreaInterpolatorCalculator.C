#include "mpi.h"

#include "FaceAreaInterpolatorCalculator.h"
#include "FiniteElement.h"
#include "FiniteElementTypes.h"
#include "Quadrature.h"
#include "Quadrature3D.h"
#include "ElementContributions.h"
#include "LayerElement.h"
#include "globaldefs.h"


void FaceAreaInterpolatorCalculator::compute ( const SubdomainElement&  element,
                                               const CoefficientArray&  xs,
                                                     MultiInterpolator& interpolator ) const {

   FiniteElementMethod::ElementGeometryMatrix geometryMatrix;

   FiniteElementMethod::ElementVector         startVes;
   FiniteElementMethod::ElementVector         endVes;
   FiniteElementMethod::ElementVector         ves;

   FiniteElementMethod::ElementVector         maxVes;
   FiniteElementMethod::ElementVector         startMaxVes;
   FiniteElementMethod::ElementVector         endMaxVes;

   FiniteElementMethod::FiniteElement         finiteElement;
   MultiInterpolator::CoefficientMatrix       rhsValues;
   ThreeVector      normal;
   double       lambda;
   double       faceArea;
   double       dsDt;
   unsigned int face;
   unsigned int i;

   getPreviousCoefficients ( element.getLayerElement (), Basin_Modelling::VES_FP,  startVes );
   getPreviousCoefficients ( element.getLayerElement (), Basin_Modelling::Max_VES, startMaxVes );

   getCoefficients ( element.getLayerElement (), Basin_Modelling::VES_FP,  endVes );
   getCoefficients ( element.getLayerElement (), Basin_Modelling::Max_VES, endMaxVes );

   // Should swap the order of these loops to improve performance.
   // The ves and max-ves are then interpolated once per face.

   for ( face = 0; face < 6; ++face ) {
      VolumeData::BoundaryId faceId = static_cast<VolumeData::BoundaryId>( face);

      NumericFunctions::Quadrature3D::Iterator quad;
      NumericFunctions::Quadrature3D::getInstance ().get ( DarcyInterpolatorQuadratureDegree, faceId, quad );

      for ( i = 0; i <= MultiInterpolator::Degree; ++i ) {
         lambda = xs [ i ];
         rhsValues [ face ][ i ] = 0.0;

         getGeometryMatrix ( element.getLayerElement (), geometryMatrix, lambda );
         finiteElement.setGeometry ( geometryMatrix );

         interpolate ( startVes,    endVes,    lambda, ves );
         interpolate ( startMaxVes, endMaxVes, lambda, maxVes );
         faceArea = 0.0;

         for ( quad.initialise (); not quad.isDone (); ++quad ) {
            finiteElement.setQuadraturePoint ( quad.getX (), quad.getY (), quad.getZ ());
            getElementBoundaryNormal ( element.getLayerElement (), finiteElement.getJacobian (), faceId, normal, dsDt );

            faceArea += dsDt * quad.getWeight ();

         } // quadrature

         rhsValues [ face ][ i ] = faceArea;
      } // interpolation degree

   } // element faces

   interpolator.compute ( xs, rhsValues );
}
