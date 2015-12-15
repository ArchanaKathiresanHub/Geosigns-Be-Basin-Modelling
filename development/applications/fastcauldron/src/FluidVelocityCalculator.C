#include "FluidVelocityCalculator.h"
#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"
#include "MultiComponentFlowHandler.h"
#include "element_contributions.h"
#include "FiniteElementTypes.h"
#include "Saturation.h"
#include "Lithology.h"
#include "PetscBlockVector.h"

#include "Interface/RunParameters.h"



using namespace FiniteElementMethod;


OutputPropertyMap* allocateFluidVelocityCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<FluidVelocityCalculator>( property, formation, surface, snapshot );
}

OutputPropertyMap* allocateFluidVelocityVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<FluidVelocityVolumeCalculator>( property, formation, snapshot );
}

FluidVelocityCalculator::FluidVelocityCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {

   m_depth = 0;
   m_temperature = 0;
   m_hydrostaticPressure = 0;
   m_overpressure = 0;
   m_ves = 0;
   m_maxVes = 0;
   m_chemicalCompaction = 0;
   m_isCalculated = false;
   m_lithologies = 0;
   m_fluid = 0;

   m_chemicalCompactionRequired = m_formation->hasChemicalCompaction () and 
                                  FastcauldronSimulator::getInstance ().getRunParameters ()->getChemicalCompaction ();


   if ( m_formation->getTopSurface () == surface ) {
      // Number-of-nodes = number-of-elements + 1.
      // But C array indexing starts a 0, so 1 must be subtracted.
      m_kIndex = (unsigned int)(m_formation->getMaximumNumberOfElements ()) - 1;

      // The top of the element is the bottom of the reference element, since depth is positive.
      m_zPosition = -1.0;
   } else if ( m_formation->getBottomSurface () == surface ) {
      m_kIndex = 0;

      // The bottom of the element is the top of the reference element, since depth is positive.
      m_zPosition = 1.0;
   } else {
      assert ( false );
      // Error
   }

}

bool FluidVelocityCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                 OutputPropertyMap::PropertyValueList&  propertyValues ) {

   using namespace Basin_Modelling;

   if ( m_isCalculated ) {
      return true;
   }

   const bool IncludeGhosts = true;
   const ElementList& elements = FastcauldronSimulator::getInstance ().getCauldron ()->mapElementList;

   const ElementVolumeGrid& grid = m_formation->getVolumeGrid ( Saturation::NumberOfPhases );

   bool includeWaterSaturation = FastcauldronSimulator::getInstance ().getMcfHandler ().includeWaterSaturationInOp () and
                                ( FastcauldronSimulator::getInstance ().getMcfHandler ().solveFlowEquations ()
                                    and ( not m_formation->isCrust () and not m_formation->isMantle ()));

   unsigned int elementCount;
   unsigned int i;
   unsigned int j;
   int globalXNodes;
   int globalYNodes;
   int node;
   int k;
   int usableKIndex;
   bool validElementFound;
   Interface::GridMap* fluidVelocityMapX;
   Interface::GridMap* fluidVelocityMapY;
   Interface::GridMap* fluidVelocityMapZ;
   const CompoundLithology* lithology;


   ThreeVector fluidVelocity;
   ElementGeometryMatrix geometryMatrix;
   ElementVector hydrostaticPressure;
   ElementVector overpressure;
   ElementVector ves;
   ElementVector maxVes;
   ElementVector temperature;
   ElementVector chemCompaction;
   double        relativePermeability;

   DMDAGetInfo( *FastcauldronSimulator::getInstance ().getCauldron ()->mapDA, 
                PETSC_NULL, &globalXNodes, &globalYNodes,
                PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, 
                PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL );

   if ( not m_depth->isCalculated ()) {

      if ( not m_depth->calculate ()) {
         return false;
      }

   }

   if ( not m_temperature->isCalculated ()) {

      if ( not m_temperature->calculate ()) {
         return false;
      }

   }

   if ( not m_hydrostaticPressure->isCalculated ()) {

      if ( not m_hydrostaticPressure->calculate ()) {
         return false;
      }

   }

   if ( not m_overpressure->isCalculated ()) {

      if ( not m_overpressure->calculate ()) {
         return false;
      }

   }

   if ( not m_ves->isCalculated ()) {

      if ( not m_ves->calculate ()) {
         return false;
      }

   }

   if ( not m_maxVes->isCalculated ()) {

      if ( not m_maxVes->calculate ()) {
         return false;
      } 

   }

   if ( m_chemicalCompactionRequired and not m_chemicalCompaction->isCalculated ()) {

      if ( not m_chemicalCompaction->calculate ()) {
         return false;
      } 

   }

   PetscBlockVector<Saturation> saturations;

   if ( includeWaterSaturation ) {
      saturations.setVector ( grid, m_formation->getPhaseSaturationVec (), INSERT_VALUES );
   }

   // Get all the properties required  for the calculation of the heat-flow.
   PETSC_3D_Array layerTemperature
      ( m_formation->layerDA,
        m_formation->Current_Properties ( Basin_Modelling::Temperature ),
        INSERT_VALUES, IncludeGhosts );

   PETSC_3D_Array layerDepth
      ( m_formation->layerDA,
        m_formation->Current_Properties ( Basin_Modelling::Depth ),
        INSERT_VALUES, IncludeGhosts );

   PETSC_3D_Array layerCurrentVES
      ( m_formation->layerDA, 
        m_formation->Current_Properties ( Basin_Modelling::VES_FP ),
        INSERT_VALUES, IncludeGhosts );

   PETSC_3D_Array layerCurrentMaxVES
      ( m_formation->layerDA, 
        m_formation->Current_Properties ( Basin_Modelling::Max_VES ),
        INSERT_VALUES, IncludeGhosts );

   PETSC_3D_Array layerCurrentChemicalCompaction
      ( m_formation->layerDA, 
        m_formation->Current_Properties ( Basin_Modelling::Chemical_Compaction ),
        INSERT_VALUES, IncludeGhosts );

   PETSC_3D_Array layerCurrentPo
      ( m_formation->layerDA, 
        m_formation->Current_Properties ( Basin_Modelling::Overpressure ),
        INSERT_VALUES, IncludeGhosts );

   PETSC_3D_Array layerCurrentPh
      ( m_formation->layerDA, 
        m_formation->Current_Properties ( Basin_Modelling::Hydrostatic_Pressure ),
        INSERT_VALUES, IncludeGhosts );

   // Retrieve the heat-flow result maps.
   fluidVelocityMapX = propertyValues [ 0 ]->getGridMap ();
   fluidVelocityMapX->retrieveData ();

   fluidVelocityMapY = propertyValues [ 1 ]->getGridMap ();
   fluidVelocityMapY->retrieveData ();

   fluidVelocityMapZ = propertyValues [ 2 ]->getGridMap ();
   fluidVelocityMapZ->retrieveData ();


   const double deltaX  = fluidVelocityMapX->deltaI ();
   const double deltaY  = fluidVelocityMapX->deltaJ ();
   const double originX = fluidVelocityMapX->minI ();
   const double originY = fluidVelocityMapX->minJ ();

   for ( elementCount = 0; elementCount < elements.size(); elementCount++ ) {

      if ( elements[elementCount].exists) {
         i = elements [ elementCount ].i [ 0 ];
         j = elements [ elementCount ].j [ 0 ];

         lithology = (*m_lithologies)( i, j );
         const Lithology* litho = static_cast<const Lithology*>( lithology );

         if ( lithology != 0 ) {
            validElementFound = false;

            for ( k = m_kIndex; k >= 0; --k ) {

               // Retrieve element data.
               for ( node = 1; node <= 8; ++node ) {
                  int LidxZ = k + (( node - 1 ) < 4 ? 1 : 0);
                  int GidxY = elements [ elementCount ].j [( node - 1 ) % 4 ];
                  int GidxX = elements [ elementCount ].i [( node - 1 ) % 4 ];

                  geometryMatrix ( 1, node ) = originX + (deltaX * GidxX);
                  geometryMatrix ( 2, node ) = originY + (deltaY * GidxY);
                  geometryMatrix ( 3, node ) = layerDepth ( LidxZ, GidxY, GidxX );
               }

               if ( not Degenerate_Element ( geometryMatrix )) {
                  validElementFound = true;
                  usableKIndex = k;
                  break;
               }

            }

            if ( validElementFound ) {

               // Retrieve element data.
               for ( node = 1; node <= 8; ++node ) {
                  int LidxZ = usableKIndex + (( node - 1 ) < 4 ? 1 : 0);
                  int GidxY = elements [ elementCount ].j [( node - 1 ) % 4 ];
                  int GidxX = elements [ elementCount ].i [( node - 1 ) % 4 ];

                  ves                 ( node ) = layerCurrentVES ( LidxZ, GidxY, GidxX );
                  maxVes              ( node ) = layerCurrentMaxVES ( LidxZ, GidxY, GidxX );
                  overpressure        ( node ) = layerCurrentPo ( LidxZ, GidxY, GidxX );
                  hydrostaticPressure ( node ) = layerCurrentPh ( LidxZ, GidxY, GidxX );
                  temperature         ( node ) = layerTemperature ( LidxZ, GidxY, GidxX );
                  chemCompaction      ( node ) = layerCurrentChemicalCompaction ( LidxZ, GidxY, GidxX );
               }

               relativePermeability = ( includeWaterSaturation ? litho->relativePermeability ( Saturation::WATER, saturations (  k, j, i )) : 1.0 );

               computeFluidVelocity ( 0.0, 0.0, m_zPosition,
                                      lithology,
                                      m_fluid,
                                      m_chemicalCompactionRequired,
                                      geometryMatrix,
                                      ves,
                                      maxVes,
                                      hydrostaticPressure,
                                      overpressure,
                                      temperature,
                                      chemCompaction,
                                      relativePermeability,
                                      fluidVelocity );
            } else {
               fluidVelocity ( 1 ) = CAULDRONIBSNULLVALUE;
               fluidVelocity ( 2 ) = CAULDRONIBSNULLVALUE;
               fluidVelocity ( 3 ) = CAULDRONIBSNULLVALUE;
            }

            fluidVelocityMapX->setValue ( i, j, fluidVelocity ( 1 ));
            fluidVelocityMapY->setValue ( i, j, fluidVelocity ( 2 ));
            fluidVelocityMapZ->setValue ( i, j, fluidVelocity ( 3 ));

            // Fill other heat flow nodes if current (i,j) position is at end of array
            if ( i == (unsigned int )(globalXNodes) - 2 ) {
               fluidVelocityMapX->setValue ( i + 1, j, fluidVelocity ( 1 ));
               fluidVelocityMapY->setValue ( i + 1, j, fluidVelocity ( 2 ));
               fluidVelocityMapZ->setValue ( i + 1, j, fluidVelocity ( 3 ));
            }

            if ( j == (unsigned int )(globalYNodes) - 2 ) {
               fluidVelocityMapX->setValue ( i, j + 1, fluidVelocity ( 1 ));
               fluidVelocityMapY->setValue ( i, j + 1, fluidVelocity ( 2 ));
               fluidVelocityMapZ->setValue ( i, j + 1, fluidVelocity ( 3 ));
            }

            if ( i == (unsigned int)(globalXNodes) - 2 and j == (unsigned int )(globalYNodes) - 2 ) {
               fluidVelocityMapX->setValue ( i + 1, j + 1, fluidVelocity ( 1 ));
               fluidVelocityMapY->setValue ( i + 1, j + 1, fluidVelocity ( 2 ));
               fluidVelocityMapZ->setValue ( i + 1, j + 1, fluidVelocity ( 3 ));
            }

         }
          
      }

   }

   fluidVelocityMapX->restoreData ();
   fluidVelocityMapY->restoreData ();
   fluidVelocityMapZ->restoreData ();

   m_isCalculated = true;
   return true;
}

void FluidVelocityCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {


   const Interface::Formation* formation = m_formation;

   PropertyValue* fluidVelocityX;
   PropertyValue* fluidVelocityY;
   PropertyValue* fluidVelocityZ;
   

   fluidVelocityX = (PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "FluidVelocityX", 
                                                                                                    m_snapshot, 0,
                                                                                                    formation,
                                                                                                    m_surface ));

   fluidVelocityY = (PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "FluidVelocityY", 
                                                                                                    m_snapshot, 0,
                                                                                                    formation,
                                                                                                    m_surface ));

   fluidVelocityZ = (PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "FluidVelocityZ", 
                                                                                                    m_snapshot, 0,
                                                                                                    formation,
                                                                                                    m_surface ));

   properties.push_back ( fluidVelocityX );
   properties.push_back ( fluidVelocityY );
   properties.push_back ( fluidVelocityZ );

   if ( FastcauldronSimulator::getInstance ().getModellingMode () == Interface::MODE1D ) {
      fluidVelocityX->allowOutput ( false );
      fluidVelocityY->allowOutput ( false );
   }

}

bool FluidVelocityCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   m_depth = PropertyManager::getInstance().findOutputPropertyMap ( "Depth", m_formation, m_surface, m_snapshot );
   m_temperature = PropertyManager::getInstance().findOutputPropertyMap ( "Temperature", m_formation, m_surface, m_snapshot );
   m_hydrostaticPressure = PropertyManager::getInstance().findOutputPropertyMap ( "HydroStaticPressure", m_formation, m_surface, m_snapshot );
   m_overpressure = PropertyManager::getInstance().findOutputPropertyMap ( "OverPressure", m_formation, m_surface, m_snapshot );
   m_ves = PropertyManager::getInstance().findOutputPropertyMap ( "Ves", m_formation, m_surface, m_snapshot );
   m_maxVes = PropertyManager::getInstance().findOutputPropertyMap ( "MaxVes", m_formation, m_surface, m_snapshot );

   if ( m_chemicalCompactionRequired ) {
      m_chemicalCompaction = PropertyManager::getInstance().findOutputPropertyMap ( "ChemicalCompaction", m_formation, m_surface, m_snapshot );
   } else {
      m_chemicalCompaction = 0;
   }

   m_lithologies = &m_formation->getCompoundLithologyArray ();
//    m_lithologies = &m_formation->Lithology;
   m_fluid = m_formation->fluid;
   
   if ( FastcauldronSimulator::getInstance ().getCauldron()->no2Doutput()) {
      propertyValues [ 0 ]->allowOutput ( false );
      propertyValues [ 1 ]->allowOutput ( false );
      propertyValues [ 2 ]->allowOutput ( false );
   }

   return m_depth != 0 and m_temperature != 0 and m_hydrostaticPressure != 0 and m_overpressure != 0 and 
          m_ves != 0 and m_maxVes != 0 and m_lithologies != 0 and
          ( m_chemicalCompactionRequired ? m_chemicalCompaction != 0 : true );
}



FluidVelocityVolumeCalculator::FluidVelocityVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {

   m_depth = 0;
   m_temperature = 0;
   m_hydrostaticPressure = 0;
   m_overpressure = 0;
   m_ves = 0;
   m_maxVes = 0;
   m_chemicalCompaction = 0;
   m_isCalculated = false;
   m_lithologies = 0;
   m_fluid = 0;

   m_chemicalCompactionRequired = m_formation->hasChemicalCompaction () and 
                                  FastcauldronSimulator::getInstance ().getRunParameters ()->getChemicalCompaction ();

}

bool FluidVelocityVolumeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                 OutputPropertyMap::PropertyValueList&  propertyValues ) {

   using namespace Basin_Modelling;

   if ( m_isCalculated ) {
      return true;
   }

   const bool IncludeGhosts = true;
   const ElementList& elements = FastcauldronSimulator::getInstance ().getCauldron ()->mapElementList;

   unsigned int elementCount;
   unsigned int i;
   unsigned int j;
   unsigned int k;
   int globalXNodes;
   int globalYNodes;
   int zCount;
   int node;
   Interface::GridMap* fluidVelocityMapX;
   Interface::GridMap* fluidVelocityMapY;
   Interface::GridMap* fluidVelocityMapZ;
   CompoundLithology* lithology;


   ThreeVector fluidVelocity;
   ElementGeometryMatrix geometryMatrix;
   ElementVector hydrostaticPressure;
   ElementVector overpressure;
   ElementVector ves;
   ElementVector maxVes;
   ElementVector temperature;
   ElementVector chemCompaction;
   double        relativePermeability;

   DMDAGetInfo( m_formation->layerDA, 
                PETSC_NULL,
                &globalXNodes, &globalYNodes, &zCount,
                PETSC_NULL, PETSC_NULL, PETSC_NULL, 
                PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL );
   
   if ( not m_depth->isCalculated ()) {

      if ( not m_depth->calculate ()) {
         return false;
      }

   }

   if ( not m_temperature->isCalculated ()) {

      if ( not m_temperature->calculate ()) {
         return false;
      }

   }

   if ( not m_hydrostaticPressure->isCalculated ()) {

      if ( not m_hydrostaticPressure->calculate ()) {
         return false;
      }

   }

   if ( not m_overpressure->isCalculated ()) {

      if ( not m_overpressure->calculate ()) {
         return false;
      }

   }

   if ( not m_ves->isCalculated ()) {

      if ( not m_ves->calculate ()) {
         return false;
      }

   }

   if ( not m_maxVes->isCalculated ()) {

      if ( not m_maxVes->calculate ()) {
         return false;
      } 

   }

   if ( m_chemicalCompactionRequired and not m_chemicalCompaction->isCalculated ()) {

      if ( not m_chemicalCompaction->calculate ()) {
         return false;
      } 

   }

   // Get all the properties required  for the calculation of the heat-flow.
   PETSC_3D_Array layerTemperature
      ( m_formation->layerDA,
        m_formation->Current_Properties ( Basin_Modelling::Temperature ),
        INSERT_VALUES, IncludeGhosts );

   PETSC_3D_Array layerDepth
      ( m_formation->layerDA,
        m_formation->Current_Properties ( Basin_Modelling::Depth ),
        INSERT_VALUES, IncludeGhosts );

   PETSC_3D_Array layerCurrentVES
      ( m_formation->layerDA, 
        m_formation->Current_Properties ( Basin_Modelling::VES_FP ),
        INSERT_VALUES, IncludeGhosts );

   PETSC_3D_Array layerCurrentMaxVES
      ( m_formation->layerDA, 
        m_formation->Current_Properties ( Basin_Modelling::Max_VES ),
        INSERT_VALUES, IncludeGhosts );

   PETSC_3D_Array layerCurrentChemicalCompaction
      ( m_formation->layerDA, 
        m_formation->Current_Properties ( Basin_Modelling::Chemical_Compaction ),
        INSERT_VALUES, IncludeGhosts );

   PETSC_3D_Array layerCurrentPo
      ( m_formation->layerDA, 
        m_formation->Current_Properties ( Basin_Modelling::Overpressure ),
        INSERT_VALUES, IncludeGhosts );

   PETSC_3D_Array layerCurrentPh
      ( m_formation->layerDA, 
        m_formation->Current_Properties ( Basin_Modelling::Hydrostatic_Pressure ),
        INSERT_VALUES, IncludeGhosts );

   const ElementVolumeGrid& grid = m_formation->getVolumeGrid ( Saturation::NumberOfPhases );

   PetscBlockVector<Saturation> saturations;

   bool includeWaterSaturation = FastcauldronSimulator::getInstance ().getMcfHandler ().includeWaterSaturationInOp () and
                                ( FastcauldronSimulator::getInstance ().getMcfHandler ().solveFlowEquations ()
                                    and ( not m_formation->isCrust () and not m_formation->isMantle ()));

   if ( includeWaterSaturation ) {
      saturations.setVector ( grid, m_formation->getPhaseSaturationVec (), INSERT_VALUES );
   }

   // Retrieve the heat-flow result maps.
   fluidVelocityMapX = propertyValues [ 0 ]->getGridMap ();
   fluidVelocityMapX->retrieveData ();

   fluidVelocityMapY = propertyValues [ 1 ]->getGridMap ();
   fluidVelocityMapY->retrieveData ();

   fluidVelocityMapZ = propertyValues [ 2 ]->getGridMap ();
   fluidVelocityMapZ->retrieveData ();

   const double deltaX  = fluidVelocityMapX->deltaI ();
   const double deltaY  = fluidVelocityMapX->deltaJ ();
   const double originX = fluidVelocityMapX->minI ();
   const double originY = fluidVelocityMapX->minJ ();

   
   for ( k = fluidVelocityMapX->firstK (); k <= fluidVelocityMapX->lastK () - 1; ++k ) {

      for ( elementCount = 0; elementCount < elements.size(); elementCount++ ) {

         if ( elements[elementCount].exists) {
            i = elements [ elementCount ].i [ 0 ];
            j = elements [ elementCount ].j [ 0 ];

            lithology = (*m_lithologies)( i, j );
            const Lithology* litho = static_cast<const Lithology*>( lithology );

            if ( lithology != 0 ) {

               // Retrieve element data.
               for ( node = 1; node <= 8; ++node ) {
                  int LidxZ = k + (( node - 1 ) < 4 ? 1 : 0);
                  int GidxY = elements [ elementCount ].j [( node - 1 ) % 4 ];
                  int GidxX = elements [ elementCount ].i [( node - 1 ) % 4 ];

                  geometryMatrix ( 1, node ) = originX + (deltaX * GidxX);
                  geometryMatrix ( 2, node ) = originY + (deltaY * GidxY);
                  geometryMatrix ( 3, node ) = layerDepth ( LidxZ, GidxY, GidxX );

                  ves                 ( node ) = layerCurrentVES ( LidxZ, GidxY, GidxX );
                  maxVes              ( node ) = layerCurrentMaxVES ( LidxZ, GidxY, GidxX );
                  overpressure        ( node ) = layerCurrentPo ( LidxZ, GidxY, GidxX );
                  hydrostaticPressure ( node ) = layerCurrentPh ( LidxZ, GidxY, GidxX );
                  temperature         ( node ) = layerTemperature ( LidxZ, GidxY, GidxX );
                  chemCompaction      ( node ) = layerCurrentChemicalCompaction ( LidxZ, GidxY, GidxX );
               }

               relativePermeability = ( includeWaterSaturation ? litho->relativePermeability ( Saturation::WATER, saturations (  k, j, i )) : 1.0 );

               computeFluidVelocity ( lithology,
                                      m_fluid,
                                      m_chemicalCompactionRequired,
                                      geometryMatrix,
                                      ves,
                                      maxVes,
                                      hydrostaticPressure,
                                      overpressure,
                                      temperature,
                                      chemCompaction,
                                      relativePermeability,
                                      fluidVelocity );

               fluidVelocityMapX->setValue ( i, j, k, fluidVelocity ( 1 ));
               fluidVelocityMapY->setValue ( i, j, k, fluidVelocity ( 2 ));
               fluidVelocityMapZ->setValue ( i, j, k, fluidVelocity ( 3 ));

               // Fill other heat flow nodes if current (i,j) position is at end of array
               if ( i == (unsigned int )(globalXNodes) - 2 ) {
                  fluidVelocityMapX->setValue ( i + 1, j, k, fluidVelocity ( 1 ));
                  fluidVelocityMapY->setValue ( i + 1, j, k, fluidVelocity ( 2 ));
                  fluidVelocityMapZ->setValue ( i + 1, j, k, fluidVelocity ( 3 ));
               }

               if ( j == (unsigned int )(globalYNodes) - 2 ) {
                  fluidVelocityMapX->setValue ( i, j + 1, k, fluidVelocity ( 1 ));
                  fluidVelocityMapY->setValue ( i, j + 1, k, fluidVelocity ( 2 ));
                  fluidVelocityMapZ->setValue ( i, j + 1, k, fluidVelocity ( 3 ));
               }

               if ( i == (unsigned int)(globalXNodes) - 2 and j == (unsigned int )(globalYNodes) - 2 ) {
                  fluidVelocityMapX->setValue ( i + 1, j + 1, k, fluidVelocity ( 1 ));
                  fluidVelocityMapY->setValue ( i + 1, j + 1, k, fluidVelocity ( 2 ));
                  fluidVelocityMapZ->setValue ( i + 1, j + 1, k, fluidVelocity ( 3 ));
               }

               // At top of layer.
               if ( k == fluidVelocityMapX->lastK () - 1 ) {
                  fluidVelocityMapX->setValue ( i, j, k + 1, fluidVelocity ( 1 ));
                  fluidVelocityMapY->setValue ( i, j, k + 1, fluidVelocity ( 2 ));
                  fluidVelocityMapZ->setValue ( i, j, k + 1, fluidVelocity ( 3 ));

                  // Fill other heat flow nodes if current (i,j) position is at end of array if ( i == (unsigned int )(globalXNodes) - 2 ) {
                  if ( i == (unsigned int )(globalXNodes) - 2 ) {
                     fluidVelocityMapX->setValue ( i + 1, j, k + 1, fluidVelocity ( 1 ));
                     fluidVelocityMapY->setValue ( i + 1, j, k + 1, fluidVelocity ( 2 ));
                     fluidVelocityMapZ->setValue ( i + 1, j, k + 1, fluidVelocity ( 3 ));
                  }

                  if ( j == (unsigned int )(globalYNodes) - 2 ) {
                     fluidVelocityMapX->setValue ( i, j + 1, k + 1, fluidVelocity ( 1 ));
                     fluidVelocityMapY->setValue ( i, j + 1, k + 1, fluidVelocity ( 2 ));
                     fluidVelocityMapZ->setValue ( i, j + 1, k + 1, fluidVelocity ( 3 ));
                  }

                  if ( i == (unsigned int)(globalXNodes) - 2 && j == (unsigned int )(globalYNodes) - 2 ) {
                     fluidVelocityMapX->setValue ( i + 1, j + 1, k + 1, fluidVelocity ( 1 ));
                     fluidVelocityMapY->setValue ( i + 1, j + 1, k + 1, fluidVelocity ( 2 ));
                     fluidVelocityMapZ->setValue ( i + 1, j + 1, k + 1, fluidVelocity ( 3 ));
                  }

               }


            }

         }
          
      }

   }

   fluidVelocityMapX->restoreData ();
   fluidVelocityMapY->restoreData ();
   fluidVelocityMapZ->restoreData ();

   m_isCalculated = true;
   return true;
}

void FluidVelocityVolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   PropertyValue* fluidVelocityX;
   PropertyValue* fluidVelocityY;
   PropertyValue* fluidVelocityZ;
   
   fluidVelocityX = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "FluidVelocityX", 
                                                                                                       m_snapshot, 0,
                                                                                                       m_formation,
                                                                                                       m_formation->getMaximumNumberOfElements () + 1 ));

   fluidVelocityY = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "FluidVelocityY", 
                                                                                                       m_snapshot, 0,
                                                                                                       m_formation,
                                                                                                       m_formation->getMaximumNumberOfElements () + 1 ));

   fluidVelocityZ = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "FluidVelocityZ",
                                                                                                       m_snapshot, 0,
                                                                                                       m_formation,
                                                                                                       m_formation->getMaximumNumberOfElements () + 1 ));

   properties.push_back ( fluidVelocityX );
   properties.push_back ( fluidVelocityY );
   properties.push_back ( fluidVelocityZ );

   if ( FastcauldronSimulator::getInstance ().getModellingMode () == Interface::MODE1D ) {
      fluidVelocityX->allowOutput ( false );
      fluidVelocityY->allowOutput ( false );
   }


}

bool FluidVelocityVolumeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   m_depth = PropertyManager::getInstance().findOutputPropertyVolume ( "Depth", m_formation, m_snapshot );
   m_temperature = PropertyManager::getInstance().findOutputPropertyVolume ( "Temperature", m_formation, m_snapshot );
   m_hydrostaticPressure = PropertyManager::getInstance().findOutputPropertyVolume ( "HydroStaticPressure", m_formation, m_snapshot );
   m_overpressure = PropertyManager::getInstance().findOutputPropertyVolume ( "OverPressure", m_formation, m_snapshot );
   m_ves = PropertyManager::getInstance().findOutputPropertyVolume ( "Ves", m_formation, m_snapshot );
   m_maxVes = PropertyManager::getInstance().findOutputPropertyVolume ( "MaxVes", m_formation, m_snapshot );

   if ( m_chemicalCompactionRequired ) {
      m_chemicalCompaction = PropertyManager::getInstance().findOutputPropertyVolume ( "ChemicalCompaction", m_formation, m_snapshot );
   } else {
      m_chemicalCompaction = 0;
   }

   m_lithologies = &m_formation->getCompoundLithologyArray ();
//    m_lithologies = &m_formation->Lithology;
   m_fluid = m_formation->fluid;

   return m_depth != 0 and m_temperature != 0 and m_hydrostaticPressure != 0 and m_overpressure != 0 and 
          m_ves != 0 and m_maxVes != 0 and m_lithologies != 0 and
          ( m_chemicalCompactionRequired ? m_chemicalCompaction != 0 : true );
}
