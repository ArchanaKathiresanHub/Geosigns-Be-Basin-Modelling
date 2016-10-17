//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "CapillaryPressureCalculator.h"
#include "CompoundLithology.h"
#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"
#include "MultiComponentFlowHandler.h"
#include "element_contributions.h"
#include "FiniteElementTypes.h"

#include "LayerElement.h"
#include "PetscBlockVector.h"
#include "Saturation.h"
#include "ElementContributions.h"
#include "Lithology.h"

#include "Interface/RunParameters.h"
#include "Interface/Interface.h"


#include "timefilter.h"
#include "BrooksCorey.h"

#define DEBUG

using namespace FiniteElementMethod;
using namespace pvtFlash;

OutputPropertyMap* allocateCapillaryPressureVolumeCalculator ( const ::PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<CapillaryPressureVolumeCalculator>( property, formation, snapshot );
}

//volume calculator 
//constructor
CapillaryPressureVolumeCalculator::CapillaryPressureVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {
   m_isCalculated = false;
}

CapillaryPressureVolumeCalculator::~CapillaryPressureVolumeCalculator() {
}

void CapillaryPressureVolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {


   PropertyValue* phase;
   phase = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "HcLiquidBrineCapillaryPressure", 
                                                                                              m_snapshot, 0,
                                                                                              m_formation,
                                                                                              m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( phase );

 
   phase = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "HcVapourBrineCapillaryPressure", 
                                                                                              m_snapshot, 0,
                                                                                              m_formation,
                                                                                              m_formation->getMaximumNumberOfElements () + 1 )); 
   properties.push_back ( phase );
}

bool CapillaryPressureVolumeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   if ( not FastcauldronSimulator::getInstance ().getMcfHandler ().containsFormation ( m_formation )) {

      OutputPropertyMap::PropertyValueList::iterator iter;

      for ( iter = propertyValues.begin (); iter != propertyValues.end (); ++iter ) {
         (*iter)->allowOutput ( false );
      }

   } else {
      m_temperature = PropertyManager::getInstance().findOutputPropertyVolume ( "Temperature", m_formation, m_snapshot );
      m_pressure = PropertyManager::getInstance().findOutputPropertyVolume ( "Pressure", m_formation, m_snapshot );
      m_fluid = m_formation->fluid;
      m_lithologies = &m_formation->getCompoundLithologyArray ();
   }

   return true;
}

bool CapillaryPressureVolumeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                     OutputPropertyMap::PropertyValueList&  propertyValues )
{
  
   using namespace CBMGenerics;

   if (m_isCalculated)
   {
      return true;
   }

   unsigned int i;
   unsigned int j;
   unsigned int k;
   

   Interface::GridMap * liquidWaterCapPressureMap;
   Interface::GridMap * vapourWaterCapPressureMap;

   const ElementVolumeGrid& grid = m_formation->getVolumeGrid ( Saturation::NumberOfPhases );

   double hcVapourDensity = 0.0;
   double hcLiquidDensity = 0.0;
   double temperature = 0.0;
   double pressure = 0.0;
   double undefinedValue; 
   double lwcp;
   double vwcp;

   CompoundProperty porosity;
   double ves;
   double maxVes;
   double entryPressure;
   double permeabilityNormal;
   double permeabilityPlane;
 
   //get liquid phase pc
   liquidWaterCapPressureMap = propertyValues[0]->getGridMap ();
   liquidWaterCapPressureMap->retrieveData ();
   //get vapour phase pc
   vapourWaterCapPressureMap = propertyValues[1]->getGridMap ();
   vapourWaterCapPressureMap->retrieveData ();
   
   undefinedValue = liquidWaterCapPressureMap->getUndefinedValue ();

   
   PetscBlockVector<Saturation> saturations;
   Saturation saturation;
   saturations.setVector ( grid, m_formation->getPhaseSaturationVec (), INSERT_VALUES );

   PVTPhaseComponents phaseComposition;
   PVTComponents      molarMasses;
   PVTComponents      massConcentration;
   PVTPhaseValues     phaseDensities;
   PVTPhaseValues     phaseViscosities;

   molarMasses = PVTCalc::getInstance ().getMolarMass ();
   molarMasses *= 1.0e-3;
   
   bool vesIsActive = m_formation->Current_Properties.propertyIsActivated ( Basin_Modelling::VES_FP );
   bool maxVesIsActive = m_formation->Current_Properties.propertyIsActivated ( Basin_Modelling::Max_VES );


   if ( not vesIsActive ) {
      m_formation->Current_Properties.Activate_Property ( Basin_Modelling::VES_FP, INSERT_VALUES, true );
   }

   if ( not maxVesIsActive ) {
      m_formation->Current_Properties.Activate_Property ( Basin_Modelling::Max_VES, INSERT_VALUES, true );
   }

   const double ConcentrationLowerLimit = 1.0e-20;
   
   for ( i = grid.firstI (); i <= grid.lastI (); ++i ) {

      for ( j = grid.firstJ (); j <= grid.lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

            for ( k = grid.firstK (); k <= grid.lastK (); ++k ) {

               const LayerElement& element = m_formation->getLayerElement ( i, j, k );

               if ( element.isActive ()) {

                  // get element saturation
                  saturation = saturations ( k, j, i );

                  ves = computeProperty ( element, Basin_Modelling::VES_FP );
                  maxVes = computeProperty ( element, Basin_Modelling::Max_VES );
                  element.getLithology()->getPorosity ( ves, maxVes, false, 0.0, porosity );
                  element.getLithology()->calcBulkPermeabilityNP ( ves, maxVes, porosity, permeabilityNormal, permeabilityPlane );

                  if ( FastcauldronSimulator::getInstance ().useCalculatedCapillaryPressure ()) {
                     GeoPhysics::BrooksCorey brooksCorey;
                     entryPressure = brooksCorey.computeCapillaryEntryPressure ( permeabilityNormal, element.getLithology()->capC1 (), element.getLithology()->capC2 ());
                  } else {
                     entryPressure = BrooksCorey::Pe;
                  }
					   
                  lwcp = element.getLithology()->capillaryPressure ( Saturation::LIQUID,
                                                                     saturation,
                                                                     entryPressure );

                  vwcp = element.getLithology()->capillaryPressure ( Saturation::VAPOUR,
                                                                     saturation,
                                                                     entryPressure );

                  liquidWaterCapPressureMap->setValue(i,j,k,lwcp);
                  vapourWaterCapPressureMap->setValue(i,j,k,vwcp);

               } else {
                  liquidWaterCapPressureMap->setValue(i,j,k,undefinedValue);
                  vapourWaterCapPressureMap->setValue(i,j,k,undefinedValue);
               }

               // adjust for element and node based difference
               // If last element in row or column or ... then copy value to fill array.
               // Since arrays are the same size as the number of nodes.
               if ( i == grid.getNumberOfXElements () - 1 ) {
                  liquidWaterCapPressureMap->setValue ( i + 1, j, k, liquidWaterCapPressureMap->getValue ( i, j, k ));
                  vapourWaterCapPressureMap->setValue ( i + 1, j, k, vapourWaterCapPressureMap->getValue ( i, j, k ));
               }

               if ( j == grid.getNumberOfYElements () - 1 ) {
                  liquidWaterCapPressureMap->setValue ( i, j + 1, k, liquidWaterCapPressureMap->getValue ( i, j, k ));
                  vapourWaterCapPressureMap->setValue ( i, j + 1, k, vapourWaterCapPressureMap->getValue ( i, j, k ));
               }

               if ( k == grid.getNumberOfZElements () - 1 ) {
                  liquidWaterCapPressureMap->setValue ( i, j, k + 1, liquidWaterCapPressureMap->getValue ( i, j, k ));
                  vapourWaterCapPressureMap->setValue ( i, j, k + 1, vapourWaterCapPressureMap->getValue ( i, j, k ));
               }

               if ( i == grid.getNumberOfXElements () - 1 and j == grid.getNumberOfYElements () - 1 ) {
                  liquidWaterCapPressureMap->setValue ( i + 1, j + 1, k, liquidWaterCapPressureMap->getValue ( i, j, k ));
                  vapourWaterCapPressureMap->setValue ( i + 1, j + 1, k, vapourWaterCapPressureMap->getValue ( i, j, k ));
               }

               if ( i == grid.getNumberOfXElements () - 1 and k == grid.getNumberOfZElements () - 1 ) {
                  liquidWaterCapPressureMap->setValue ( i + 1, j, k + 1, liquidWaterCapPressureMap->getValue ( i, j, k ));
                  vapourWaterCapPressureMap->setValue ( i + 1, j, k + 1, vapourWaterCapPressureMap->getValue ( i, j, k ));
               }

               if ( j == grid.getNumberOfYElements () - 1 and k == grid.getNumberOfZElements () - 1 ) {
                  liquidWaterCapPressureMap->setValue ( i, j + 1, k + 1, liquidWaterCapPressureMap->getValue ( i, j, k ));
                  vapourWaterCapPressureMap->setValue ( i, j + 1, k + 1, vapourWaterCapPressureMap->getValue ( i, j, k ));
               }

               if ( i == grid.getNumberOfXElements () - 1 and j == grid.getNumberOfYElements () - 1 and k == grid.getNumberOfZElements () - 1 ) {
                  liquidWaterCapPressureMap->setValue ( i + 1, j + 1, k + 1, liquidWaterCapPressureMap->getValue ( i, j, k ));
                  vapourWaterCapPressureMap->setValue ( i + 1, j + 1, k + 1, vapourWaterCapPressureMap->getValue ( i, j, k ));
               }

            }

         } else {

            for ( k = grid.firstK (); k <= grid.lastK (); ++k ) {
               liquidWaterCapPressureMap->setValue(i,j,k,undefinedValue);
               vapourWaterCapPressureMap->setValue(i,j,k,undefinedValue);
            }

         }

      }

   }

   // Restore the ves to its original state.
   if ( not vesIsActive ) {
      m_formation->Current_Properties.Restore_Property ( Basin_Modelling::VES_FP );
   }

   // Restore the max-ves to its original state.
   if ( not maxVesIsActive ) {
      m_formation->Current_Properties.Restore_Property ( Basin_Modelling::Max_VES );
   }

   liquidWaterCapPressureMap->restoreData ();
   vapourWaterCapPressureMap->restoreData ();
   m_isCalculated = true;

   return true;
}
