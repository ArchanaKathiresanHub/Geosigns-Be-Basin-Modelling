//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "PvtHcProperties.h"

#include "EosPack.h"
#include "PetscBlockVector.h"
#include "ElementVolumeGrid.h"
#include "ComponentManager.h"
#include "PVTCalculator.h"

#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"
#include "MultiComponentFlowHandler.h"

#include "LayerElement.h"
#include "ElementContributions.h"


// utilities library
#include "ConstantsNumerical.h"
using Utilities::Numerical::CauldronNoDataValue;

OutputPropertyMap* allocatePvtHcPropertiesVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<PvtHcPropertiesVolumeCalculator>( property, formation, snapshot );
}


PvtHcPropertiesVolumeCalculator::PvtHcPropertiesVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {

   m_isCalculated = false;

}

void PvtHcPropertiesVolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   PropertyValue* component;


   component = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "HcVapourDensity",
                                                                                                  m_snapshot, 0,
                                                                                                  m_formation,
                                                                                                  m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( component );

   component = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "HcLiquidDensity",
                                                                                                  m_snapshot, 0,
                                                                                                  m_formation,
                                                                                                  m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( component );

   component = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "HcVapourViscosity",
                                                                                                  m_snapshot, 0,
                                                                                                  m_formation,
                                                                                                  m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( component );

   component = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "HcLiquidViscosity",
                                                                                                  m_snapshot, 0,
                                                                                                  m_formation,
                                                                                                  m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( component );

}

bool PvtHcPropertiesVolumeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   if ( not FastcauldronSimulator::getInstance ().getMcfHandler ().containsFormation ( m_formation )) {

      OutputPropertyMap::PropertyValueList::iterator iter;

      for ( iter = propertyValues.begin (); iter != propertyValues.end (); ++iter ) {
         (*iter)->allowOutput ( false );
      }

   }

   return true;
}

bool PvtHcPropertiesVolumeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                         OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   double temperature;
   double pressure;

   unsigned int i;
   unsigned int j;
   unsigned int k;
   unsigned int c;

   const ElementVolumeGrid& grid = m_formation->getVolumeGrid ( NumberOfPVTComponents );

   Interface::GridMap* componentMaps [ NumberOfPVTComponents ];

   PetscBlockVector<PVTComponents> layerMolarConcentrations;

   Interface::GridMap* hcDensityMaps [ 2 ];
   Interface::GridMap* hcViscosityMaps [ 2 ];

   bool temperatureIsActive = m_formation->Current_Properties.propertyIsActivated ( Basin_Modelling::Temperature );
   bool porePressureIsActive = m_formation->Current_Properties.propertyIsActivated ( Basin_Modelling::Pore_Pressure );

   if ( not temperatureIsActive ) {
      m_formation->Current_Properties.Activate_Property ( Basin_Modelling::Temperature, INSERT_VALUES, true );
   }

   if ( not porePressureIsActive ) {
      m_formation->Current_Properties.Activate_Property ( Basin_Modelling::Pore_Pressure, INSERT_VALUES, true );
   }

   // At this point previous and current have same values
   layerMolarConcentrations.setVector ( grid, m_formation->getPreviousComponentVec (), INSERT_VALUES );

   hcDensityMaps [ 0 ] = propertyValues [ 0 ]->getGridMap ();
   hcDensityMaps [ 0 ]->retrieveData ();
   hcDensityMaps [ 1 ] = propertyValues [ 1 ]->getGridMap ();
   hcDensityMaps [ 1 ]->retrieveData ();

   hcViscosityMaps [ 0 ] = propertyValues [ 2 ]->getGridMap ();
   hcViscosityMaps [ 0 ]->retrieveData ();
   hcViscosityMaps [ 1 ] = propertyValues [ 3 ]->getGridMap ();
   hcViscosityMaps [ 1 ]->retrieveData ();

   PVTPhaseComponents phaseComposition;
   PVTComponents      molarMasses;
   PVTComponents      massConcentration;
   PVTPhaseValues     phaseDensities;
   PVTPhaseValues     phaseViscosities;

   molarMasses = PVTCalc::getInstance ().getMolarMass ();
   molarMasses *= 1.0e-3;

   for ( i = grid.firstI (); i <= grid.lastI (); ++i ) {

      for ( j = grid.firstJ (); j <= grid.lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

            for ( k = grid.firstK (); k <= grid.lastK (); ++k ) {

               const LayerElement& element = m_formation->getLayerElement ( i, j, k );

               // Convert temperature to Kelvin.
               temperature = computeProperty ( element, Basin_Modelling::Temperature ) + 273.15;

               // Convert pressure to Pa.
               pressure = 1.0e6 * computeProperty ( element, Basin_Modelling::Pore_Pressure );

               // massConcentration = layerMolarConcentrations ( k, j, i );

               // Get components that are to be flashed.
               for ( c = 0; c < NumberOfPVTComponents; ++c ) {
                  pvtFlash::ComponentId pvtComponent = pvtFlash::ComponentId ( c );
                  massConcentration ( pvtComponent ) = layerMolarConcentrations ( k, j, i )( pvtComponent );
               }

               // Convert molar concentrations to mass concentrations mol/m^3 -> kg/m^3.
               // Should use the molar-mass computed in PVT based on the concentrations.
               massConcentration *= molarMasses;

               // Flash concentrations.
               pvtFlash::EosPack::getInstance ().computeWithLumping ( temperature, pressure,
                                                           massConcentration.m_components,
                                                           phaseComposition.m_masses,
                                                           phaseDensities.m_values,
                                                           phaseViscosities.m_values );

               // Convert to correct units.
               phaseViscosities *= 0.001;

               if ( phaseDensities ( pvtFlash::VAPOUR_PHASE ) == 1000.0 ) {
                  phaseDensities ( pvtFlash::VAPOUR_PHASE ) = CauldronNoDataValue;
                  phaseViscosities ( pvtFlash::VAPOUR_PHASE ) = CauldronNoDataValue;
               }

               if ( phaseDensities ( pvtFlash::LIQUID_PHASE ) == 1000.0 ) {
                  phaseDensities ( pvtFlash::LIQUID_PHASE ) = CauldronNoDataValue;
                  phaseViscosities ( pvtFlash::LIQUID_PHASE ) = CauldronNoDataValue;
               }

               hcDensityMaps [ 0 ]->setValue ( i, j, k, phaseDensities ( pvtFlash::VAPOUR_PHASE ));
               hcDensityMaps [ 1 ]->setValue ( i, j, k, phaseDensities ( pvtFlash::LIQUID_PHASE ));

               hcViscosityMaps [ 0 ]->setValue ( i, j, k, phaseViscosities ( pvtFlash::VAPOUR_PHASE ));
               hcViscosityMaps [ 1 ]->setValue ( i, j, k, phaseViscosities ( pvtFlash::LIQUID_PHASE ));

               // If last element in row or column or ... then copy value to fill array.
               // Since arrays are the same size as the number of nodes.
               if ( i == grid.getNumberOfXElements () - 1 ) {
                  hcDensityMaps [ 0 ]->setValue ( i + 1, j, k, phaseDensities ( pvtFlash::VAPOUR_PHASE ));
                  hcDensityMaps [ 1 ]->setValue ( i + 1, j, k, phaseDensities ( pvtFlash::LIQUID_PHASE ));

                  hcViscosityMaps [ 0 ]->setValue ( i + 1, j, k, phaseViscosities ( pvtFlash::VAPOUR_PHASE ));
                  hcViscosityMaps [ 1 ]->setValue ( i + 1, j, k, phaseViscosities ( pvtFlash::LIQUID_PHASE ));
               }

               if ( j == grid.getNumberOfYElements () - 1 ) {
                  hcDensityMaps [ 0 ]->setValue ( i, j + 1, k, phaseDensities ( pvtFlash::VAPOUR_PHASE ));
                  hcDensityMaps [ 1 ]->setValue ( i, j + 1, k, phaseDensities ( pvtFlash::LIQUID_PHASE ));

                  hcViscosityMaps [ 0 ]->setValue ( i, j + 1, k, phaseViscosities ( pvtFlash::VAPOUR_PHASE ));
                  hcViscosityMaps [ 1 ]->setValue ( i, j + 1, k, phaseViscosities ( pvtFlash::LIQUID_PHASE ));
               }

               if ( k == grid.getNumberOfZElements () - 1 ) {
                  hcDensityMaps [ 0 ]->setValue ( i, j, k + 1, phaseDensities ( pvtFlash::VAPOUR_PHASE ));
                  hcDensityMaps [ 1 ]->setValue ( i, j, k + 1, phaseDensities ( pvtFlash::LIQUID_PHASE ));

                  hcViscosityMaps [ 0 ]->setValue ( i, j, k + 1, phaseViscosities ( pvtFlash::VAPOUR_PHASE ));
                  hcViscosityMaps [ 1 ]->setValue ( i, j, k + 1, phaseViscosities ( pvtFlash::LIQUID_PHASE ));
               }

               if ( i == grid.getNumberOfXElements () - 1 and j == grid.getNumberOfYElements () - 1 ) {
                  hcDensityMaps [ 0 ]->setValue ( i + 1, j + 1, k, phaseDensities ( pvtFlash::VAPOUR_PHASE ));
                  hcDensityMaps [ 1 ]->setValue ( i + 1, j + 1, k, phaseDensities ( pvtFlash::LIQUID_PHASE ));

                  hcViscosityMaps [ 0 ]->setValue ( i + 1, j + 1, k, phaseViscosities ( pvtFlash::VAPOUR_PHASE ));
                  hcViscosityMaps [ 1 ]->setValue ( i + 1, j + 1, k, phaseViscosities ( pvtFlash::LIQUID_PHASE ));
               }

               if ( i == grid.getNumberOfXElements () - 1 and k == grid.getNumberOfZElements () - 1 ) {
                  hcDensityMaps [ 0 ]->setValue ( i + 1, j, k + 1, phaseDensities ( pvtFlash::VAPOUR_PHASE ));
                  hcDensityMaps [ 1 ]->setValue ( i + 1, j, k + 1, phaseDensities ( pvtFlash::LIQUID_PHASE ));

                  hcViscosityMaps [ 0 ]->setValue ( i + 1, j, k + 1, phaseViscosities ( pvtFlash::VAPOUR_PHASE ));
                  hcViscosityMaps [ 1 ]->setValue ( i + 1, j, k + 1, phaseViscosities ( pvtFlash::LIQUID_PHASE ));
               }

               if ( j == grid.getNumberOfYElements () - 1 and k == grid.getNumberOfZElements () - 1 ) {
                  hcDensityMaps [ 0 ]->setValue ( i, j + 1, k + 1, phaseDensities ( pvtFlash::VAPOUR_PHASE ));
                  hcDensityMaps [ 1 ]->setValue ( i, j + 1, k + 1, phaseDensities ( pvtFlash::LIQUID_PHASE ));

                  hcViscosityMaps [ 0 ]->setValue ( i, j + 1, k + 1, phaseViscosities ( pvtFlash::VAPOUR_PHASE ));
                  hcViscosityMaps [ 1 ]->setValue ( i, j + 1, k + 1, phaseViscosities ( pvtFlash::LIQUID_PHASE ));
               }

               if ( i == grid.getNumberOfXElements () - 1 and j == grid.getNumberOfYElements () - 1 and k == grid.getNumberOfZElements () - 1 ) {
                  hcDensityMaps [ 0 ]->setValue ( i + 1, j + 1, k + 1, phaseDensities ( pvtFlash::VAPOUR_PHASE ));
                  hcDensityMaps [ 1 ]->setValue ( i + 1, j + 1, k + 1, phaseDensities ( pvtFlash::LIQUID_PHASE ));

                  hcViscosityMaps [ 0 ]->setValue ( i + 1, j + 1, k + 1, phaseViscosities ( pvtFlash::VAPOUR_PHASE ));
                  hcViscosityMaps [ 1 ]->setValue ( i + 1, j + 1, k + 1, phaseViscosities ( pvtFlash::LIQUID_PHASE ));
               }

            }

         }

      }

   }

   if ( not temperatureIsActive ) {
      m_formation->Current_Properties.Restore_Property ( Basin_Modelling::Temperature );
   }

   if ( not porePressureIsActive ) {
      m_formation->Current_Properties.Restore_Property ( Basin_Modelling::Pore_Pressure );
   }

   hcDensityMaps [ 0 ]->restoreData ();
   hcDensityMaps [ 1 ]->restoreData ();

   hcViscosityMaps [ 0 ]->restoreData ();
   hcViscosityMaps [ 1 ]->restoreData ();

   m_isCalculated = true;
   return true;
}
