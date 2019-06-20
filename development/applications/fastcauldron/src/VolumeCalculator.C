//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "VolumeCalculator.h"

#include "EosPack.h"
#include "PetscBlockVector.h"
#include "ElementVolumeGrid.h"
#include "ComponentManager.h"
#include "PVTCalculator.h"

#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"

#include "LayerElement.h"
#include "ElementContributions.h"

// utilities library
#include "ConstantsNumerical.h"
using Utilities::Numerical::CauldronNoDataValue;


OutputPropertyMap* allocateVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<VolumeCalculator>( property, formation, snapshot );
}


VolumeCalculator::VolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {

   m_isCalculated = false;

}

void VolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   CauldronPropertyValue* component;

   component = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "HcVapourVolume",
                                                                                                  m_snapshot, 0,
                                                                                                  m_formation,
                                                                                                  m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( component );

   component = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "HcLiquidVolume",
                                                                                                  m_snapshot, 0,
                                                                                                  m_formation,
                                                                                                  m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( component );

   component = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "ElementVolume",
                                                                                                  m_snapshot, 0,
                                                                                                  m_formation,
                                                                                                  m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( component );

   component = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "ElementPoreVolume",
                                                                                                  m_snapshot, 0,
                                                                                                  m_formation,
                                                                                                  m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( component );

}

bool VolumeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {
   return true;
}

bool VolumeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
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

   Interface::GridMap* hcVolumeMaps [ 2 ];
   Interface::GridMap* elementVolumeMaps [ 2 ];

   bool temperatureIsActive = m_formation->Current_Properties.propertyIsActivated ( Basin_Modelling::Temperature );
   bool porePressureIsActive = m_formation->Current_Properties.propertyIsActivated ( Basin_Modelling::Pore_Pressure );
   bool depthIsActive = m_formation->Current_Properties.propertyIsActivated ( Basin_Modelling::Depth );
   bool vesIsActive = m_formation->Current_Properties.propertyIsActivated ( Basin_Modelling::VES_FP );
   bool maxVesIsActive = m_formation->Current_Properties.propertyIsActivated ( Basin_Modelling::Max_VES );

   if ( not temperatureIsActive ) {
      m_formation->Current_Properties.Activate_Property ( Basin_Modelling::Temperature, INSERT_VALUES, true );
   }

   if ( not porePressureIsActive ) {
      m_formation->Current_Properties.Activate_Property ( Basin_Modelling::Pore_Pressure, INSERT_VALUES, true );
   }

   if ( not depthIsActive ) {
      m_formation->Current_Properties.Activate_Property ( Basin_Modelling::Depth, INSERT_VALUES, true );
   }

   if ( not vesIsActive ) {
      m_formation->Current_Properties.Activate_Property ( Basin_Modelling::VES_FP, INSERT_VALUES, true );
   }

   if ( not maxVesIsActive ) {
      m_formation->Current_Properties.Activate_Property ( Basin_Modelling::Max_VES, INSERT_VALUES, true );
   }

   // At this point previous and current have same values
   layerMolarConcentrations.setVector ( grid, m_formation->getPreviousComponentVec (), INSERT_VALUES );

   hcVolumeMaps [ 0 ] = propertyValues [ 0 ]->getGridMap ();
   hcVolumeMaps [ 0 ]->retrieveData ();

   hcVolumeMaps [ 1 ] = propertyValues [ 1 ]->getGridMap ();
   hcVolumeMaps [ 1 ]->retrieveData ();

   elementVolumeMaps [ 0 ] = propertyValues [ 2 ]->getGridMap ();
   elementVolumeMaps [ 0 ]->retrieveData ();

   elementVolumeMaps [ 1 ] = propertyValues [ 3 ]->getGridMap ();
   elementVolumeMaps [ 1 ]->retrieveData ();

   PVTPhaseComponents phaseComposition;
   PVTComponents      molarMasses;
   PVTComponents      massConcentration;
   PVTPhaseValues     phaseDensities;
   PVTPhaseValues     phaseViscosities;
   PVTPhaseValues     phaseVolume;
   double             elementVolume;
   double             elementPoreVolume;

   molarMasses = PVTCalc::getInstance ().getMolarMass ();
   molarMasses *= 1.0e-3;

   for ( i = grid.firstI (); i <= grid.lastI (); ++i ) {

      for ( j = grid.firstJ (); j <= grid.lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

            for ( k = grid.firstK (); k <= grid.lastK (); ++k ) {

               const LayerElement& element = m_formation->getLayerElement ( i, j, k );

               elementVolumeCalculations ( element, elementVolume, elementPoreVolume, 5 );

               // Convert temperature to Kelvin.
               temperature = computeProperty ( element, Basin_Modelling::Temperature ) + 273.15;

               // Convert pressure to Pa.
               pressure = 1.0e6 * computeProperty ( element, Basin_Modelling::Pore_Pressure );

               // massConcentration = layerMolarConcentrations ( k, j, i );

               // Get components that are to be flashed.
               for ( c = 0; c < NumberOfPVTComponents; ++c ) {
                  ComponentId pvtComponent = ComponentId ( c );
                  massConcentration ( pvtComponent ) = layerMolarConcentrations ( k, j, i )( pvtComponent );
               }

               // Convert molar concentrations (mol/m^3) to mass concentrations (kg/m^3).
               massConcentration *= molarMasses;

               // Flash concentrations.
               pvtFlash::EosPack::getInstance ().computeWithLumping ( temperature, pressure,
                                                           massConcentration.m_components,
                                                           phaseComposition.m_masses,
                                                           phaseDensities.m_values,
                                                           phaseViscosities.m_values );

               if ( phaseDensities ( PhaseId::VAPOUR ) != 1000.0 ) {
                  phaseVolume ( PhaseId::VAPOUR ) = phaseComposition.sum ( PhaseId::VAPOUR ) / phaseDensities ( PhaseId::VAPOUR );
               } else {
                  phaseVolume ( PhaseId::VAPOUR ) = CauldronNoDataValue;
               }

               if ( phaseDensities ( PhaseId::LIQUID ) != 1000.0 ) {
                  phaseVolume ( PhaseId::LIQUID ) = phaseComposition.sum ( PhaseId::LIQUID ) / phaseDensities ( PhaseId::LIQUID );
               } else {
                  phaseVolume ( PhaseId::LIQUID ) = CauldronNoDataValue;
               }

               hcVolumeMaps [ 0 ]->setValue ( i, j, k, phaseVolume ( PhaseId::VAPOUR ));
               hcVolumeMaps [ 1 ]->setValue ( i, j, k, phaseVolume ( PhaseId::LIQUID ));

               elementVolumeMaps [ 0 ]->setValue ( i, j, k, elementVolume );
               elementVolumeMaps [ 1 ]->setValue ( i, j, k, elementPoreVolume );

               // If last element in row or column or ... then copy value to fill array.
               // Since arrays are the same size as the number of nodes.
               if ( i == grid.getNumberOfXElements () - 1 ) {
                  hcVolumeMaps [ 0 ]->setValue ( i + 1, j, k, phaseVolume ( PhaseId::VAPOUR ));
                  hcVolumeMaps [ 1 ]->setValue ( i + 1, j, k, phaseVolume ( PhaseId::LIQUID ));

                  elementVolumeMaps [ 0 ]->setValue ( i + 1, j, k, elementVolume );
                  elementVolumeMaps [ 1 ]->setValue ( i + 1, j, k, elementPoreVolume );
               }

               if ( j == grid.getNumberOfYElements () - 1 ) {
                  hcVolumeMaps [ 0 ]->setValue ( i, j + 1, k, phaseVolume ( PhaseId::VAPOUR ));
                  hcVolumeMaps [ 1 ]->setValue ( i, j + 1, k, phaseVolume ( PhaseId::LIQUID ));

                  elementVolumeMaps [ 0 ]->setValue ( i, j + 1, k, elementVolume );
                  elementVolumeMaps [ 1 ]->setValue ( i, j + 1, k, elementPoreVolume );
               }

               if ( k == grid.getNumberOfZElements () - 1 ) {
                  hcVolumeMaps [ 0 ]->setValue ( i, j, k + 1, phaseVolume ( PhaseId::VAPOUR ));
                  hcVolumeMaps [ 1 ]->setValue ( i, j, k + 1, phaseVolume ( PhaseId::LIQUID ));

                  elementVolumeMaps [ 0 ]->setValue ( i, j, k + 1, elementVolume );
                  elementVolumeMaps [ 1 ]->setValue ( i, j, k + 1, elementPoreVolume );
               }

               if ( i == grid.getNumberOfXElements () - 1 and j == grid.getNumberOfYElements () - 1 ) {
                  hcVolumeMaps [ 0 ]->setValue ( i + 1, j + 1, k, phaseVolume ( PhaseId::VAPOUR ));
                  hcVolumeMaps [ 1 ]->setValue ( i + 1, j + 1, k, phaseVolume ( PhaseId::LIQUID ));

                  elementVolumeMaps [ 0 ]->setValue ( i + 1, j + 1, k, elementVolume );
                  elementVolumeMaps [ 1 ]->setValue ( i + 1, j + 1, k, elementPoreVolume );
               }

               if ( i == grid.getNumberOfXElements () - 1 and k == grid.getNumberOfZElements () - 1 ) {
                  hcVolumeMaps [ 0 ]->setValue ( i + 1, j, k + 1, phaseVolume ( PhaseId::VAPOUR ));
                  hcVolumeMaps [ 1 ]->setValue ( i + 1, j, k + 1, phaseVolume ( PhaseId::LIQUID ));

                  elementVolumeMaps [ 0 ]->setValue ( i + 1, j, k + 1, elementVolume );
                  elementVolumeMaps [ 1 ]->setValue ( i + 1, j, k + 1, elementPoreVolume );
               }

               if ( j == grid.getNumberOfYElements () - 1 and k == grid.getNumberOfZElements () - 1 ) {
                  hcVolumeMaps [ 0 ]->setValue ( i, j + 1, k + 1, phaseVolume ( PhaseId::VAPOUR ));
                  hcVolumeMaps [ 1 ]->setValue ( i, j + 1, k + 1, phaseVolume ( PhaseId::LIQUID ));

                  elementVolumeMaps [ 0 ]->setValue ( i, j + 1, k + 1, elementVolume );
                  elementVolumeMaps [ 1 ]->setValue ( i, j + 1, k + 1, elementPoreVolume );
               }

               if ( i == grid.getNumberOfXElements () - 1 and j == grid.getNumberOfYElements () - 1 and k == grid.getNumberOfZElements () - 1 ) {
                  hcVolumeMaps [ 0 ]->setValue ( i + 1, j + 1, k + 1, phaseVolume ( PhaseId::VAPOUR ));
                  hcVolumeMaps [ 1 ]->setValue ( i + 1, j + 1, k + 1, phaseVolume ( PhaseId::LIQUID ));

                  elementVolumeMaps [ 0 ]->setValue ( i + 1, j + 1, k + 1, elementVolume );
                  elementVolumeMaps [ 1 ]->setValue ( i + 1, j + 1, k + 1, elementPoreVolume );
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

   if ( not depthIsActive ) {
      m_formation->Current_Properties.Restore_Property ( Basin_Modelling::Depth );
   }

   if ( not vesIsActive ) {
      m_formation->Current_Properties.Restore_Property ( Basin_Modelling::VES_FP );
   }

   if ( not maxVesIsActive ) {
      m_formation->Current_Properties.Restore_Property ( Basin_Modelling::Max_VES );
   }

   hcVolumeMaps [ 0 ]->restoreData ();
   hcVolumeMaps [ 1 ]->restoreData ();

   elementVolumeMaps [ 0 ]->restoreData ();
   elementVolumeMaps [ 1 ]->restoreData ();

   m_isCalculated = true;
   return true;
}

