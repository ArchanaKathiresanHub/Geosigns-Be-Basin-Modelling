#include "mpi.h"
#include "FacePermeabilityInterpolatorCalculator.h"
#include "Lithology.h"

#include "FiniteElementTypes.h"
#include "property_manager.h"
#include "ElementContributions.h"
#include "LayerElement.h"

FacePermeabilityInterpolatorCalculator::FacePermeabilityInterpolatorCalculator ( const Subdomain& subdomain ) :
   m_averagedPermeability ( subdomain.getVolumeGrid ().lengthI (), subdomain.getVolumeGrid ().lengthJ (), subdomain.getVolumeGrid ().lengthK ())
{
   m_indexTransformation [ 0 ] = subdomain.getVolumeGrid ().firstI ();
   m_indexTransformation [ 1 ] = subdomain.getVolumeGrid ().firstJ ();
   m_indexTransformation [ 2 ] = subdomain.getVolumeGrid ().firstK ();

   const ElementVolumeGrid& permeabilityGrid  = subdomain.getVolumeGrid ( NumberOfPermeabilityValues );

   Vec intermediatePermeabilityVec;
   PermeabilityValuesVector intermediatePermeability;

   DACreateGlobalVector ( permeabilityGrid.getDa (), &intermediatePermeabilityVec );

   // Set the vector, only local values need be set
   intermediatePermeability.setVector ( permeabilityGrid, intermediatePermeabilityVec, INSERT_VALUES );

   // Compute the local permeability values that will be required.
   computeLocalPermeabilities ( subdomain, permeabilityGrid, intermediatePermeability );

   // only local values need be restored
   intermediatePermeability.restoreVector ( UPDATE_EXCLUDING_GHOSTS );

   // Set the vector, both local and non-local values need to be retrieved.
   intermediatePermeability.setVector ( permeabilityGrid, intermediatePermeabilityVec, INSERT_VALUES, true );

   // Average the permeability values using the harmonic average.
   averagePermeabilities ( subdomain, permeabilityGrid, intermediatePermeability );

   // only local values need be restored, since no update was made to the values.
   intermediatePermeability.restoreVector ( UPDATE_EXCLUDING_GHOSTS );

   VecDestroy ( intermediatePermeabilityVec );
}


void FacePermeabilityInterpolatorCalculator::computeLocalPermeabilities ( const Subdomain&         subdomain,
                                                                          const ElementVolumeGrid& elementGrid,
                                                                          PermeabilityValuesVector& permeabilityValues ) const {

   FiniteElementMethod::ElementGeometryMatrix geometryMatrix;
   FiniteElementMethod::ElementVector         startVes;
   FiniteElementMethod::ElementVector         endVes;
   FiniteElementMethod::ElementVector         ves;
   FiniteElementMethod::ElementVector         maxVes;
   FiniteElementMethod::ElementVector         startMaxVes;
   FiniteElementMethod::ElementVector         endMaxVes;
   FiniteElementMethod::FiniteElement         finiteElement;

   CoefficientArray lambdas;
   CompoundProperty porosity;

   // Check for Degree = 0.
   double h = 1.0 / static_cast<double>( MultiInterpolator::Degree );
   double lambda;
   double x;
   double vesValue;
   double maxVesValue;
   double y;
   double z;
   double permNormal;
   double permPlane;

   int i;
   int j;
   int k;
   int l;
   int face;
   int position;

   // Compute a uniform distribution of x values.
   for ( i = 0, lambda = 0.0; i <= MultiInterpolator::Degree; ++i, lambda += h ) {
      lambdas [ i ] = lambda;
   }

   for ( i = elementGrid.firstI (); i <= elementGrid.lastI (); ++i ) {

      for ( j = elementGrid.firstJ (); j <= elementGrid.lastJ (); ++j ) {

         for ( k = elementGrid.firstK (); k <= elementGrid.lastK (); ++k ) {

            const SubdomainElement& element = subdomain.getElement ( i, j, k );
            const LayerElement& layerElement = element.getLayerElement ();

            unsigned int elementK = element.getK ();

            PermeabilityValuesArray& elementPermeability = permeabilityValues ( elementK, j, i );

            if ( layerElement.isActive ()) {

               const Lithology* lithology = layerElement.getLithology ();
               const LayerProps* layer = layerElement.getFormation ();

               getPreviousCoefficients ( layerElement, Basin_Modelling::VES_FP,  startVes );
               getPreviousCoefficients ( layerElement, Basin_Modelling::Max_VES, startMaxVes );

               getCoefficients ( layerElement, Basin_Modelling::VES_FP,  endVes );
               getCoefficients ( layerElement, Basin_Modelling::Max_VES, endMaxVes );


               for ( face = VolumeData::GAMMA_1; face <= VolumeData::GAMMA_6; ++face ) {
                  const VolumeData::BoundaryId id = static_cast<VolumeData::BoundaryId>( face );

                  for ( l = 0; l <= MultiInterpolator::Degree; ++l ) {
                     position = ( face - VolumeData::GAMMA_1 ) * VolumeData::NumberOfBoundaries + l;

                     lambda = lambdas [ l ];

                     interpolate ( startVes,    endVes,    lambda, ves );
                     interpolate ( startMaxVes, endMaxVes, lambda, maxVes );

                     getGeometryMatrix ( element.getLayerElement (), geometryMatrix, lambda );

                     finiteElement.setGeometry ( geometryMatrix );

                     // Should we check for an active face?
                     // compute permeability at centre of face.
                     getCentreOfElementFace ( layerElement, id, x, y, z );
                     finiteElement.setQuadraturePoint ( x, y, z );

                     vesValue = finiteElement.interpolate ( ves );
                     maxVesValue = finiteElement.interpolate ( maxVes );
                  
                     lithology->getPorosity ( vesValue, maxVesValue, false, 0.0, porosity );
                     lithology->calcBulkPermeabilityNP ( vesValue, maxVesValue, porosity, permNormal, permPlane );

                     elementPermeability [ position ] = permNormal;
                     elementPermeability [ position + MultiInterpolator::NumberOfCoefficients ] = permPlane;

                  }

               }

            }

         }

      }

   }   
   
}


void FacePermeabilityInterpolatorCalculator::averagePermeabilities ( const Subdomain& subdomain,
                                                                     const ElementVolumeGrid& elementGrid,
                                                                     const PermeabilityValuesVector& permeabilityValues ) {

   double elementPermeability;
   double neighbourPermeability;
   int i;
   int j;
   int k;
   int l;
   int face;
   unsigned int elementPosition;
   unsigned int neighbourPosition;

   for ( i = elementGrid.firstI (); i <= elementGrid.lastI (); ++i ) {

      for ( j = elementGrid.firstJ (); j <= elementGrid.lastJ (); ++j ) {

         for ( k = elementGrid.firstK (); k <= elementGrid.lastK (); ++k ) {

            const SubdomainElement& element = subdomain.getElement ( i, j, k );
            const LayerElement& layerElement = element.getLayerElement ();

            if ( layerElement.isActive ()) {

               const PermeabilityValuesArray& elementPermeabilityValues = permeabilityValues ( element.getK (), element.getJ (), element.getI ());

               PermeabilityValuesArray& elementAveragedPermeability = m_averagedPermeability ( element.getI () - m_indexTransformation [ 0 ],
                                                                                               element.getJ () - m_indexTransformation [ 1 ],
                                                                                               element.getK () - m_indexTransformation [ 2 ]);


               for ( face = VolumeData::GAMMA_1; face <= VolumeData::GAMMA_6; ++face ) {
                  const VolumeData::BoundaryId id = static_cast<VolumeData::BoundaryId>( face );
                  const VolumeData::BoundaryId opposite = VolumeData::opposite ( id );

                  const SubdomainElement* neighbour = element.getActiveNeighbour ( id );

                  // The start position for the current element face.
                  elementPosition = ( face - VolumeData::GAMMA_1 ) * VolumeData::NumberOfBoundaries;

                  if ( neighbour != 0 ) {

                     const PermeabilityValuesArray& neighbourPermeabilityValues = permeabilityValues ( neighbour->getK (),
                                                                                                       neighbour->getJ (),
                                                                                                       neighbour->getI ());

                     // The start position for the face of the neighbour.
                     neighbourPosition = ( opposite - VolumeData::GAMMA_1 ) * VolumeData::NumberOfBoundaries;

                     for ( l = 1; l <= NumberOfDirections * MultiInterpolator::NumberOfCoefficients; ++l, ++elementPosition, ++neighbourPosition ) {
                        elementPermeability = elementPermeabilityValues [ elementPosition ];
                        neighbourPermeability = neighbourPermeabilityValues [ neighbourPosition ];

                        // Do we need to check if the sum is zero?
                        // If we are here then they should not be 0.0.
                        elementAveragedPermeability [ elementPosition ] = 2.0 * elementPermeability * neighbourPermeability / ( elementPermeability + neighbourPermeability );

                     } 

                  } else {

                     for ( l = 1; l <= NumberOfDirections * MultiInterpolator::NumberOfCoefficients; ++l, ++elementPosition ) {
                        elementAveragedPermeability [ elementPosition ] = elementPermeabilityValues [ elementPosition ];
                     } 

                  }

               }

            }

         }

      }

   }   

}

void FacePermeabilityInterpolatorCalculator::compute ( const SubdomainElement&  element,
                                                       const CoefficientArray&  xs,
                                                             MultiInterpolator& interpolator ) const {

   unsigned int count = 0;
   unsigned int face;
   unsigned int direction;
   unsigned int valIndex;
   MultiInterpolator::CoefficientTensor permRhs;

   const PermeabilityValuesArray& elementAveragedPermeabilities = m_averagedPermeability ( element.getI () - m_indexTransformation [ 0 ],
                                                                                           element.getJ () - m_indexTransformation [ 1 ],
                                                                                           element.getK () - m_indexTransformation [ 2 ]);

   for ( face = 0; face < 6; ++face ) {

      for ( direction = 0; direction < 2; ++direction ) {

         for ( valIndex = 0; valIndex <= MultiInterpolator::Degree; ++valIndex, ++count ) {
            permRhs [ face ][ direction ][ valIndex ] = elementAveragedPermeabilities [ count ];
         }

      }

   }

   interpolator.compute ( xs, permRhs );
}
