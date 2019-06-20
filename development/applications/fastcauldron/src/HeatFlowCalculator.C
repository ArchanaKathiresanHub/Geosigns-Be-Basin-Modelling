//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "HeatFlowCalculator.h"
#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"
#include "element_contributions.h"
#include "FiniteElementTypes.h"
#include "PetscVectors.h"

#include "RunParameters.h"

// utilities library
#include "ConstantsNumerical.h"
using Utilities::Numerical::CauldronNoDataValue;

using namespace FiniteElementMethod;

OutputPropertyMap* allocateHeatFlowCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<HeatFlowCalculator>( property, formation, surface, snapshot );
}

OutputPropertyMap* allocateHeatFlowVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<HeatFlowVolumeCalculator>( property, formation, snapshot );
}

HeatFlowCalculator::HeatFlowCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {

   m_depth = 0;
   m_temperature = 0;
   m_porePressure = 0;
   m_overpressure = 0;
   m_lithoPressure = 0;
   m_ves = 0;
   m_maxVes = 0;
   m_chemicalCompaction = 0;
   m_isCalculated = false;
   m_lithologies = 0;
   m_fluid = 0;

   m_chemicalCompactionRequired = m_formation->hasChemicalCompaction () &&
                                  FastcauldronSimulator::getInstance ().getRunParameters ()->getChemicalCompaction ();

   m_isBasementFormation = m_formation->isBasement();
   m_isBasementFormationAndALC = m_isBasementFormation && FastcauldronSimulator::getInstance().isALC();

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

bool HeatFlowCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties,
                                            OutputPropertyMap::PropertyValueList&  propertyValues ) {

   using namespace Basin_Modelling;

   if ( m_isCalculated ) {
      return true;
   }

   const bool IncludeGhosts = true;
   const bool includeAdvectiveTerm = FastcauldronSimulator::getInstance ().getRunParameters ()->getConvectiveTerm ();

//   const bool IncludeAdvectiveTerm = basinModel->includeAdvectiveTerm &&
//                                    ( basinModel->Do_Iteratively_Coupled or
//                                      basinModel->IsCalculationCoupled );

   const ElementList& elements = FastcauldronSimulator::getInstance ().getCauldron ()->mapElementList;

   unsigned int elementCount;
   unsigned int i;
   unsigned int j;
   int k;
   int usableKIndex;
   int globalXNodes;
   int globalYNodes;
   int node;
   bool validElementFound;
   Interface::GridMap* heatFlowMapX;
   Interface::GridMap* heatFlowMapY;
   Interface::GridMap* heatFlowMapZ;
   const CompoundLithology* lithology;


   ThreeVector heatFlow;
   ElementGeometryMatrix geometryMatrix;
   ElementVector porePressure;
   ElementVector overpressure;
   ElementVector lithoPressure;
   ElementVector ves;
   ElementVector maxVes;
   ElementVector temperature;
   ElementVector chemCompaction;

   bool bottomOfMantleHeatFlow = ( m_formation->isMantle () && m_formation->getBottomSurface () == m_surface );

   DMDAGetInfo( *FastcauldronSimulator::getInstance ().getCauldron ()->mapDA,
                PETSC_NULL, &globalXNodes, &globalYNodes,
                PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL,
                PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL );

   if (!m_depth->isCalculated ()) {

      if (!m_depth->calculate ()) {
         return false;
      }

   }

   if (!m_temperature->isCalculated ()) {

      if (!m_temperature->calculate ()) {
         return false;
      }

   }

   if (!m_porePressure->isCalculated ()) {

      if (!m_porePressure->calculate ()) {
         return false;
      }

   }

   if (!m_overpressure->isCalculated ()) {

      if (!m_overpressure->calculate ()) {
         return false;
      }

   }

   if (!m_ves->isCalculated ()) {

      if (!m_ves->calculate ()) {
         return false;
      }

   }

   if (!m_maxVes->isCalculated ()) {

      if (!m_maxVes->calculate ()) {
         return false;
      }

   }

   if ( m_chemicalCompactionRequired && !m_chemicalCompaction->isCalculated ()) {

      if (!m_chemicalCompaction->calculate ()) {
         return false;
      }

   }

   if ( m_isBasementFormationAndALC && !m_lithoPressure->isCalculated () ) {

      if (!m_lithoPressure->calculate ()) {
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

   PETSC_3D_Array layerCurrentPp
      ( m_formation->layerDA,
        m_formation->Current_Properties ( Basin_Modelling::Pore_Pressure ),
        INSERT_VALUES, IncludeGhosts );

   PETSC_3D_Array layerCurrentLp;
   if( m_isBasementFormationAndALC ) {
      layerCurrentLp.Set_Global_Array( m_formation->layerDA,
                                       m_formation->Current_Properties ( Basin_Modelling::Lithostatic_Pressure ),
                                       INSERT_VALUES, IncludeGhosts );
   }


   // Retrieve the heat-flow result maps.
   heatFlowMapX = propertyValues [ 0 ]->getGridMap ();
   heatFlowMapX->retrieveData ();

   heatFlowMapY = propertyValues [ 1 ]->getGridMap ();
   heatFlowMapY->retrieveData ();

   heatFlowMapZ = propertyValues [ 2 ]->getGridMap ();
   heatFlowMapZ->retrieveData ();

   const double deltaX  = heatFlowMapX->deltaI ();
   const double deltaY  = heatFlowMapX->deltaJ ();
   const double originX = heatFlowMapX->minI ();
   const double originY = heatFlowMapX->minJ ();
   unsigned int topDepthIndex = m_formation->getMaximumNumberOfElements ();
   const double theTime = m_snapshot->getTime();


   for ( elementCount = 0; elementCount < elements.size(); elementCount++ ) {

      if ( elements[elementCount].exists) {
         i = elements [ elementCount ].i [ 0 ];
         j = elements [ elementCount ].j [ 0 ];

         lithology = (*m_lithologies)( i, j );

         if ( lithology != 0 ) {
            validElementFound = false;

            if ( bottomOfMantleHeatFlow ) {

               for ( k = m_kIndex; k <= m_formation->getMaximumNumberOfElements () - 1; ++k ) {

                  // Retrieve element data.
                  for ( node = 1; node <= 8; ++node ) {
                     int LidxZ = k + (( node - 1 ) < 4 ? 1 : 0);
                     int GidxY = elements [ elementCount ].j [( node - 1 ) % 4 ];
                     int GidxX = elements [ elementCount ].i [( node - 1 ) % 4 ];

                     geometryMatrix ( 1, node ) = originX + (deltaX * GidxX);
                     geometryMatrix ( 2, node ) = originY + (deltaY * GidxY);
                     geometryMatrix ( 3, node ) = layerDepth ( LidxZ, GidxY, GidxX );
                  }

                  if (!Degenerate_Element ( geometryMatrix )) {
                     validElementFound = true;
                     usableKIndex = k;
                     break;
                  }

               }

            } else {

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

                  if (!Degenerate_Element ( geometryMatrix )) {
                     validElementFound = true;
                     usableKIndex = k;
                     break;
                  }

               }

            }

            if ( validElementFound ) {

               // Retrieve element data.
               for ( node = 1; node <= 8; ++node ) {
                  int LidxZ = usableKIndex + (( node - 1 ) < 4 ? 1 : 0);
                  int GidxY = elements [ elementCount ].j [( node - 1 ) % 4 ];
                  int GidxX = elements [ elementCount ].i [( node - 1 ) % 4 ];

                  ves            ( node ) = layerCurrentVES ( LidxZ, GidxY, GidxX );
                  maxVes         ( node ) = layerCurrentMaxVES ( LidxZ, GidxY, GidxX );
                  overpressure   ( node ) = layerCurrentPo ( LidxZ, GidxY, GidxX );
                  porePressure   ( node ) = layerCurrentPp ( LidxZ, GidxY, GidxX );
                  lithoPressure  ( node ) = ( m_isBasementFormationAndALC ? layerCurrentLp ( LidxZ, GidxY, GidxX ) : 0.0 );
                  temperature    ( node ) = layerTemperature ( LidxZ, GidxY, GidxX );
                  chemCompaction ( node ) = layerCurrentChemicalCompaction ( LidxZ, GidxY, GidxX );

               }
               if( m_isBasementFormationAndALC ) {
                  ElementGeometryMatrix Geometry_Matrix1;
                  int xX, xY;
                  for ( node = 1; node <= 8; node ++ ) {
                     int LidxZ = usableKIndex + (( node - 1 ) < 4 ? 1 : 0);
                     xY = elements [ elementCount ].j [( node - 1 ) % 4 ];
                     xX = elements [ elementCount ].i [( node - 1 ) % 4 ];

                     Geometry_Matrix1 ( 3, node ) = layerDepth ( LidxZ,xY,xX ) - layerDepth( topDepthIndex, xY, xX );
                  }
                  double midPointDepth = Geometry_Matrix1.getMidPoint();

                  xX = elements [ elementCount ].i [ 0 ];
                  xY = elements [ elementCount ].j [ 0 ];

                  lithology = m_formation -> getLithology( theTime, xX, xY, midPointDepth );
               }


               computeHeatFlow ( m_isBasementFormation,
                                 0.0, 0.0, m_zPosition,
                                 lithology,
                                 m_fluid,
                                 m_chemicalCompactionRequired,
                                 includeAdvectiveTerm,
                                 geometryMatrix,
                                 ves,
                                 maxVes,
                                 overpressure,
                                 porePressure,
                                 lithoPressure,
                                 temperature,
                                 chemCompaction,
                                 heatFlow );

            } else {
               heatFlow ( 1 ) = CauldronNoDataValue;
               heatFlow ( 2 ) = CauldronNoDataValue;
               heatFlow ( 3 ) = CauldronNoDataValue;
            }

            heatFlowMapX->setValue ( i, j, heatFlow ( 1 ));
            heatFlowMapY->setValue ( i, j, heatFlow ( 2 ));
            heatFlowMapZ->setValue ( i, j, heatFlow ( 3 ));

            // Fill other heat flow nodes if current (i,j) position is at end of array
            if ( i == (unsigned int )(globalXNodes) - 2 ) {
               heatFlowMapX->setValue ( i + 1, j, heatFlow ( 1 ));
               heatFlowMapY->setValue ( i + 1, j, heatFlow ( 2 ));
               heatFlowMapZ->setValue ( i + 1, j, heatFlow ( 3 ));
            }

            if ( j == (unsigned int )(globalYNodes) - 2 ) {
               heatFlowMapX->setValue ( i, j + 1, heatFlow ( 1 ));
               heatFlowMapY->setValue ( i, j + 1, heatFlow ( 2 ));
               heatFlowMapZ->setValue ( i, j + 1, heatFlow ( 3 ));
            }

            if ( i == (unsigned int)(globalXNodes) - 2 && j == (unsigned int )(globalYNodes) - 2 ) {
               heatFlowMapX->setValue ( i + 1, j + 1, heatFlow ( 1 ));
               heatFlowMapY->setValue ( i + 1, j + 1, heatFlow ( 2 ));
               heatFlowMapZ->setValue ( i + 1, j + 1, heatFlow ( 3 ));
            }

         }

      }

   }

   heatFlowMapX->restoreData ();
   heatFlowMapY->restoreData ();
   heatFlowMapZ->restoreData ();

   m_isCalculated = true;
   return true;
}

void HeatFlowCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   CauldronPropertyValue* heatFlowX;
   CauldronPropertyValue* heatFlowY;
   CauldronPropertyValue* heatFlowZ;

   heatFlowX = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "HeatFlowX",
                                                                                               m_snapshot, 0,
                                                                                               m_formation,
                                                                                               m_surface ));

   heatFlowY = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "HeatFlowY",
                                                                                               m_snapshot, 0,
                                                                                               m_formation,
                                                                                               m_surface ));

   heatFlowZ = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "HeatFlowZ",
                                                                                               m_snapshot, 0,
                                                                                               m_formation,
                                                                                               m_surface ));

   properties.push_back ( heatFlowX );
   properties.push_back ( heatFlowY );
   properties.push_back ( heatFlowZ );

   if ( FastcauldronSimulator::getInstance ().getModellingMode () == Interface::MODE1D ) {
      heatFlowX->allowOutput ( false );
      heatFlowY->allowOutput ( false );
   }


}

bool HeatFlowCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   m_depth = PropertyManager::getInstance().findOutputPropertyMap ( "Depth", m_formation, m_surface, m_snapshot );
   m_temperature = PropertyManager::getInstance().findOutputPropertyMap ( "Temperature", m_formation, m_surface, m_snapshot );
   m_porePressure = PropertyManager::getInstance().findOutputPropertyMap ( "Pressure", m_formation, m_surface, m_snapshot );
   m_overpressure = PropertyManager::getInstance().findOutputPropertyMap ( "OverPressure", m_formation, m_surface, m_snapshot );
   m_ves = PropertyManager::getInstance().findOutputPropertyMap ( "Ves", m_formation, m_surface, m_snapshot );
   m_maxVes = PropertyManager::getInstance().findOutputPropertyMap ( "MaxVes", m_formation, m_surface, m_snapshot );

   if ( m_chemicalCompactionRequired ) {
      m_chemicalCompaction = PropertyManager::getInstance().findOutputPropertyMap ( "ChemicalCompaction", m_formation, m_surface, m_snapshot );
   } else {
      m_chemicalCompaction = 0;
   }
   if( m_isBasementFormationAndALC ) {
      m_lithoPressure = PropertyManager::getInstance().findOutputPropertyMap ( "LithoStaticPressure", m_formation, m_surface, m_snapshot );
   } else {
      m_lithoPressure = 0;
   }

   if ( FastcauldronSimulator::getInstance ().getCauldron()->no2Doutput()) {
      propertyValues [ 0 ]->allowOutput ( false );
      propertyValues [ 1 ]->allowOutput ( false );
      propertyValues [ 2 ]->allowOutput ( false );
   }

   m_lithologies = &m_formation->getCompoundLithologyArray ();
//    m_lithologies = &m_formation->Lithology;
   m_fluid = m_formation->fluid;

   return m_depth != 0 && m_temperature != 0 && m_porePressure != 0 && m_overpressure != 0 &&
          m_ves != 0 && m_maxVes != 0 && m_lithologies != 0 &&
          ( m_isBasementFormationAndALC ? m_lithoPressure != 0 : true ) &&
          ( m_chemicalCompactionRequired ? m_chemicalCompaction != 0 : true );
}


HeatFlowVolumeCalculator::HeatFlowVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {

   m_depth = 0;
   m_temperature = 0;
   m_porePressure = 0;
   m_overpressure = 0;
   m_lithoPressure = 0;
   m_ves = 0;
   m_maxVes = 0;
   m_chemicalCompaction = 0;
   m_isCalculated = false;
   m_lithologies = 0;
   m_fluid = 0;

   m_chemicalCompactionRequired = m_formation->hasChemicalCompaction () &&
                                  FastcauldronSimulator::getInstance ().getRunParameters ()->getChemicalCompaction ();
   m_isBasementFormation = m_formation->isBasement();
   m_isBasementFormationAndALC = m_isBasementFormation && FastcauldronSimulator::getInstance().isALC();
}

bool HeatFlowVolumeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties,
                                                  OutputPropertyMap::PropertyValueList&  propertyValues ) {

   using namespace Basin_Modelling;

   if ( m_isCalculated ) {
      return true;
   }

   const bool IncludeGhosts = true;
   const bool includeAdvectiveTerm = FastcauldronSimulator::getInstance ().getRunParameters ()->getConvectiveTerm ();

//   const bool IncludeAdvectiveTerm = basinModel->includeAdvectiveTerm &&
//                                    ( basinModel->Do_Iteratively_Coupled or
//                                      basinModel->IsCalculationCoupled );

   const ElementList& elements = FastcauldronSimulator::getInstance ().getCauldron ()->mapElementList;

   unsigned int elementCount;
   unsigned int i;
   unsigned int j;
   unsigned int k;
   int globalXNodes;
   int globalYNodes;
   int zCount;
   int node;
   Interface::GridMap* heatFlowMapX;
   Interface::GridMap* heatFlowMapY;
   Interface::GridMap* heatFlowMapZ;
   const CompoundLithology* lithology;


   ThreeVector heatFlow;
   ElementGeometryMatrix geometryMatrix;
   ElementVector porePressure;
   ElementVector overpressure;
   ElementVector lithoPressure;
   ElementVector ves;
   ElementVector maxVes;
   ElementVector temperature;
   ElementVector chemCompaction;

   DMDAGetInfo( m_formation->layerDA,
                PETSC_NULL,
                &globalXNodes, &globalYNodes, &zCount,
                PETSC_NULL, PETSC_NULL, PETSC_NULL,
                PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL );

   if (!m_depth->isCalculated ()) {

      if (!m_depth->calculate ()) {
         return false;
      }

   }

   if (!m_temperature->isCalculated ()) {

      if (!m_temperature->calculate ()) {
         return false;
      }

   }

   if (!m_porePressure->isCalculated ()) {

      if (!m_porePressure->calculate ()) {
         return false;
      }

   }

   if (!m_overpressure->isCalculated ()) {

      if (!m_overpressure->calculate ()) {
         return false;
      }

   }

   if (!m_ves->isCalculated ()) {

      if (!m_ves->calculate ()) {
         return false;
      }

   }

   if (!m_maxVes->isCalculated ()) {

      if (!m_maxVes->calculate ()) {
         return false;
      }

   }

   if ( m_chemicalCompactionRequired && !m_chemicalCompaction->isCalculated ()) {

      if (!m_chemicalCompaction->calculate ()) {
         return false;
      }

   }

   if ( m_isBasementFormationAndALC ) {

      if(!m_lithoPressure->isCalculated ()) {
         if (!m_lithoPressure->calculate ()) {
            return false;
         }
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

   PETSC_3D_Array layerCurrentPp
      ( m_formation->layerDA,
        m_formation->Current_Properties ( Basin_Modelling::Pore_Pressure ),
        INSERT_VALUES, IncludeGhosts );

   PETSC_3D_Array layerCurrentLp;
   if( m_isBasementFormationAndALC ) {
      layerCurrentLp.Set_Global_Array ( m_formation->layerDA,
        m_formation->Current_Properties ( Basin_Modelling::Lithostatic_Pressure ),
        INSERT_VALUES, IncludeGhosts );
   }
   // Retrieve the heat-flow result maps.
   heatFlowMapX = propertyValues [ 0 ]->getGridMap ();
   heatFlowMapX->retrieveData ();

   heatFlowMapY = propertyValues [ 1 ]->getGridMap ();
   heatFlowMapY->retrieveData ();

   heatFlowMapZ = propertyValues [ 2 ]->getGridMap ();
   heatFlowMapZ->retrieveData ();

   const double deltaX  = heatFlowMapX->deltaI ();
   const double deltaY  = heatFlowMapX->deltaJ ();
   const double originX = heatFlowMapX->minI ();
   const double originY = heatFlowMapX->minJ ();
   unsigned int topDepthIndex = m_formation->getMaximumNumberOfElements ();
   const double theTime = m_snapshot->getTime();

   for ( k = 0; k < (unsigned int )(zCount - 1); ++k ) {

      for ( elementCount = 0; elementCount < elements.size(); elementCount++ ) {

         if ( elements[elementCount].exists) {
            i = elements [ elementCount ].i [ 0 ];
            j = elements [ elementCount ].j [ 0 ];

            lithology = (*m_lithologies)( i, j );

            if ( lithology != 0 ) {

               // Retrieve element data.
               for ( node = 1; node <= 8; ++node ) {
                  int LidxZ = k + (( node - 1 ) < 4 ? 1 : 0);
                  int GidxY = elements [ elementCount ].j [( node - 1 ) % 4 ];
                  int GidxX = elements [ elementCount ].i [( node - 1 ) % 4 ];

                  geometryMatrix ( 1, node ) = originX + (deltaX * GidxX);
                  geometryMatrix ( 2, node ) = originY + (deltaY * GidxY);
                  geometryMatrix ( 3, node ) = layerDepth ( LidxZ, GidxY, GidxX );

                  ves            ( node ) = layerCurrentVES ( LidxZ, GidxY, GidxX );
                  maxVes         ( node ) = layerCurrentMaxVES ( LidxZ, GidxY, GidxX );
                  overpressure   ( node ) = layerCurrentPo ( LidxZ, GidxY, GidxX );
                  porePressure   ( node ) = layerCurrentPp ( LidxZ, GidxY, GidxX );
                  lithoPressure  ( node ) = ( m_isBasementFormationAndALC ? layerCurrentLp ( LidxZ, GidxY, GidxX ) : 0.0 );
                  temperature    ( node ) = layerTemperature ( LidxZ, GidxY, GidxX );
                  chemCompaction ( node ) = layerCurrentChemicalCompaction ( LidxZ, GidxY, GidxX );

               }

               if( m_isBasementFormationAndALC ) {
                  ElementGeometryMatrix Geometry_Matrix1;
                  int xX, xY;
                  for ( node = 1; node <= 8; node ++ ) {
                     int LidxZ = k + (( node - 1 ) < 4 ? 1 : 0);
                     xY = elements [ elementCount ].j [( node - 1 ) % 4 ];
                     xX = elements [ elementCount ].i [( node - 1 ) % 4 ];

                     Geometry_Matrix1 ( 3, node ) = layerDepth ( LidxZ,xY,xX ) - layerDepth( topDepthIndex, xY, xX );
                  }
                  double midPointDepth = Geometry_Matrix1.getMidPoint();

                  xX = elements [ elementCount ].i [ 0 ];
                  xY = elements [ elementCount ].j [ 0 ];

                  lithology = m_formation -> getLithology( theTime, xX, xY, midPointDepth );
               }

               computeHeatFlow ( m_isBasementFormation,
                                 0.0, 0.0, 0.0,
                                 lithology,
                                 m_fluid,
                                 m_chemicalCompactionRequired,
                                 includeAdvectiveTerm,
                                 geometryMatrix,
                                 ves,
                                 maxVes,
                                 overpressure,
                                 porePressure,
                                 lithoPressure,
                                 temperature,
                                 chemCompaction,
                                 heatFlow );

               heatFlowMapX->setValue ( i, j, k, heatFlow ( 1 ));
               heatFlowMapY->setValue ( i, j, k, heatFlow ( 2 ));
               heatFlowMapZ->setValue ( i, j, k, heatFlow ( 3 ));

               // Fill other heat flow nodes if current (i,j) position is at end of array
               if ( i == (unsigned int )(globalXNodes) - 2 ) {
                  heatFlowMapX->setValue ( i + 1, j, k, heatFlow ( 1 ));
                  heatFlowMapY->setValue ( i + 1, j, k, heatFlow ( 2 ));
                  heatFlowMapZ->setValue ( i + 1, j, k, heatFlow ( 3 ));
               }

               if ( j == (unsigned int )(globalYNodes) - 2 ) {
                  heatFlowMapX->setValue ( i, j + 1, k, heatFlow ( 1 ));
                  heatFlowMapY->setValue ( i, j + 1, k, heatFlow ( 2 ));
                  heatFlowMapZ->setValue ( i, j + 1, k, heatFlow ( 3 ));
               }

               if ( i == (unsigned int)(globalXNodes) - 2 && j == (unsigned int )(globalYNodes) - 2 ) {
                  heatFlowMapX->setValue ( i + 1, j + 1, k, heatFlow ( 1 ));
                  heatFlowMapY->setValue ( i + 1, j + 1, k, heatFlow ( 2 ));
                  heatFlowMapZ->setValue ( i + 1, j + 1, k, heatFlow ( 3 ));
               }

               // At top of layer.
               if ( k == (unsigned int)(zCount - 2)) {
                  heatFlowMapX->setValue ( i, j, k + 1, heatFlow ( 1 ));
                  heatFlowMapY->setValue ( i, j, k + 1, heatFlow ( 2 ));
                  heatFlowMapZ->setValue ( i, j, k + 1, heatFlow ( 3 ));

                  // Fill other heat flow nodes if current (i,j) position is at end of array if ( i == (unsigned int )(globalXNodes) - 2 ) {
                  if ( i == (unsigned int )(globalXNodes) - 2 ) {
                     heatFlowMapX->setValue ( i + 1, j, k + 1, heatFlow ( 1 ));
                     heatFlowMapY->setValue ( i + 1, j, k + 1, heatFlow ( 2 ));
                     heatFlowMapZ->setValue ( i + 1, j, k + 1, heatFlow ( 3 ));
                  }

                  if ( j == (unsigned int )(globalYNodes) - 2 ) {
                     heatFlowMapX->setValue ( i, j + 1, k + 1, heatFlow ( 1 ));
                     heatFlowMapY->setValue ( i, j + 1, k + 1, heatFlow ( 2 ));
                     heatFlowMapZ->setValue ( i, j + 1, k + 1, heatFlow ( 3 ));
                  }

                  if ( i == (unsigned int)(globalXNodes) - 2 && j == (unsigned int )(globalYNodes) - 2 ) {
                     heatFlowMapX->setValue ( i + 1, j + 1, k + 1, heatFlow ( 1 ));
                     heatFlowMapY->setValue ( i + 1, j + 1, k + 1, heatFlow ( 2 ));
                     heatFlowMapZ->setValue ( i + 1, j + 1, k + 1, heatFlow ( 3 ));
                  }

               }

            }

         }

      }

   }

   heatFlowMapX->restoreData ();
   heatFlowMapY->restoreData ();
   heatFlowMapZ->restoreData ();

   m_isCalculated = true;
   return true;
}

void HeatFlowVolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   unsigned int numberOfNodes = (unsigned int)(m_formation->getMaximumNumberOfElements () + 1);

   CauldronPropertyValue* heatFlowX;
   CauldronPropertyValue* heatFlowY;
   CauldronPropertyValue* heatFlowZ;

   heatFlowX = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "HeatFlowX",
                                                                                                  m_snapshot, 0,
                                                                                                  m_formation,
                                                                                                  numberOfNodes ));

   heatFlowY = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "HeatFlowY",
                                                                                                  m_snapshot, 0,
                                                                                                  m_formation,
                                                                                                  numberOfNodes ));

   heatFlowZ = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "HeatFlowZ",
                                                                                                  m_snapshot, 0,
                                                                                                  m_formation,
                                                                                                  numberOfNodes ));

   properties.push_back ( heatFlowX );
   properties.push_back ( heatFlowY );
   properties.push_back ( heatFlowZ );

   if ( FastcauldronSimulator::getInstance ().getModellingMode () == Interface::MODE1D ) {
      heatFlowX->allowOutput ( false );
      heatFlowY->allowOutput ( false );
   }

}

bool HeatFlowVolumeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   m_depth = PropertyManager::getInstance().findOutputPropertyVolume ( "Depth", m_formation, m_snapshot );
   m_temperature = PropertyManager::getInstance().findOutputPropertyVolume ( "Temperature", m_formation, m_snapshot );
   m_porePressure = PropertyManager::getInstance().findOutputPropertyVolume ( "Pressure", m_formation, m_snapshot );
   m_overpressure = PropertyManager::getInstance().findOutputPropertyVolume ( "OverPressure", m_formation, m_snapshot );
   m_ves = PropertyManager::getInstance().findOutputPropertyVolume ( "Ves", m_formation, m_snapshot );
   m_maxVes = PropertyManager::getInstance().findOutputPropertyVolume ( "MaxVes", m_formation, m_snapshot );

   if ( m_chemicalCompactionRequired ) {
      m_chemicalCompaction = PropertyManager::getInstance().findOutputPropertyVolume ( "ChemicalCompaction", m_formation, m_snapshot );
   } else {
      m_chemicalCompaction = 0;
   }
   if( m_isBasementFormationAndALC ) {
      m_lithoPressure = PropertyManager::getInstance().findOutputPropertyVolume ( "LithoStaticPressure", m_formation, m_snapshot );
   } else {
      m_lithoPressure = 0;
   }
   m_lithologies = &m_formation->getCompoundLithologyArray ();
   m_fluid = m_formation->fluid;

   return m_depth != 0 && m_temperature != 0 && m_porePressure != 0 && m_overpressure != 0 &&
          m_ves != 0 && m_maxVes != 0 && m_lithologies != 0 &&
          ( m_isBasementFormationAndALC ? m_lithoPressure != 0 : true ) &&
          ( m_chemicalCompactionRequired ? m_chemicalCompaction != 0 : true );
}
