#include "CapillaryEntryPressureCalculator.h"
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
#include "consts.h"

#include "timefilter.h"
#include "BrooksCorey.h"

#define DEBUG

using namespace FiniteElementMethod;
using namespace pvtFlash;

// OutputPropertyMap* allocateCapillaryEntryPressureCalculator ( const ::PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
//    return new DerivedOutputPropertyMap<CapillaryEntryPressureCalculator>( property, formation, surface, snapshot );
// }

OutputPropertyMap* allocateCapillaryEntryPressureVolumeCalculator ( const ::PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<CapillaryEntryPressureVolumeCalculator>( property, formation, snapshot );
}




//volume calculator 
//constructor
CapillaryEntryPressureVolumeCalculator::CapillaryEntryPressureVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {
   m_isCalculated = false;
}

CapillaryEntryPressureVolumeCalculator::~CapillaryEntryPressureVolumeCalculator() {
}

void CapillaryEntryPressureVolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {


   PropertyValue* phase;
   phase = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "HcLiquidBrineCapillaryEntryPressure", 
                                                                                              m_snapshot, 0,
                                                                                              m_formation,
                                                                                              m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( phase );

 
   phase = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "HcVapourBrineCapillaryEntryPressure", 
                                                                                              m_snapshot, 0,
                                                                                              m_formation,
                                                                                              m_formation->getMaximumNumberOfElements () + 1 )); 
   properties.push_back ( phase );
}

bool CapillaryEntryPressureVolumeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

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

bool CapillaryEntryPressureVolumeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                                OutputPropertyMap::PropertyValueList&  propertyValues )
{

   const int HcVapourDensityIndex = 0;
   const int HcLiquidDensityIndex = 1;
  
   using namespace CBMGenerics;

   if (m_isCalculated)
   {
      return true;
   }

   unsigned int i;
   unsigned int j;
   unsigned int k;
   

   Interface::GridMap * liquidWaterCapEntryPressureMap;
   Interface::GridMap * vapourWaterCapEntryPressureMap;

   const ElementVolumeGrid& concentrationGrid = m_formation->getVolumeGrid ( NumberOfPVTComponents );
   const ElementVolumeGrid& grid = m_formation->getVolumeGrid ( Saturation::NumberOfPhases );

   CompoundProperty porosity;
   double temperature;
   double undefinedValue; 
   double ves;
   double maxVes;
   double permeabilityNormal;
   double permeabilityPlane;
   double brineDensity;
   double brinePressure;
   double criticalTemperature;
   double lwcep;
   double vwcep;
 
   //get liquid phase pc
   liquidWaterCapEntryPressureMap = propertyValues[0]->getGridMap ();
   liquidWaterCapEntryPressureMap->retrieveData ();
   //get vapour phase pc
   vapourWaterCapEntryPressureMap = propertyValues[1]->getGridMap ();
   vapourWaterCapEntryPressureMap->retrieveData ();
   
   undefinedValue = liquidWaterCapEntryPressureMap->getUndefinedValue ();
   
   PetscBlockVector<Saturation> saturations;
   PetscBlockVector<PVTComponents> concentrations;
   Saturation saturation;

   PVTPhaseComponents phaseComposition;
   PVTComponents      molarMasses;
   PVTComponents      elementComposition;
   PVTPhaseValues     phaseDensities;
   PVTPhaseValues     phaseViscosities;

   OutputPropertyMap* pvtProperties = PropertyManager::getInstance().findOutputPropertyVolume ( "PVTProperties", m_formation, m_snapshot );

   molarMasses = PVTCalc::getInstance ().getMolarMass ();
   molarMasses *= 1.0e-3;
   
   bool vesIsActive = m_formation->Current_Properties.propertyIsActivated ( Basin_Modelling::VES_FP );
   bool maxVesIsActive = m_formation->Current_Properties.propertyIsActivated ( Basin_Modelling::Max_VES );
   bool pressureIsActive = m_formation->Current_Properties.propertyIsActivated ( Basin_Modelling::Pore_Pressure );
   bool temperatureIsActive = m_formation->Current_Properties.propertyIsActivated ( Basin_Modelling::Temperature );

   if ( not vesIsActive ) {
      m_formation->Current_Properties.Activate_Property ( Basin_Modelling::VES_FP, INSERT_VALUES, true );
   }

   if ( not maxVesIsActive ) {
      m_formation->Current_Properties.Activate_Property ( Basin_Modelling::Max_VES, INSERT_VALUES, true );
   }

   if ( not pressureIsActive ) {
      m_formation->Current_Properties.Activate_Property ( Basin_Modelling::Pore_Pressure, INSERT_VALUES, true );
   }

   if ( not temperatureIsActive ) {
      m_formation->Current_Properties.Activate_Property ( Basin_Modelling::Temperature, INSERT_VALUES, true );
   }

   saturations.setVector ( grid, m_formation->getPhaseSaturationVec (), INSERT_VALUES );
   concentrations.setVector ( concentrationGrid, m_formation->getPreviousComponentVec (), INSERT_VALUES );

   if ( not pvtProperties->isCalculated ()) {

      if ( not pvtProperties->calculate ()) {
         return false;
      }

   }

   if ( not FastcauldronSimulator::getInstance ().useCalculatedCapillaryEntryPressure ()) {
      temperature = CAULDRONIBSNULLVALUE;
      permeabilityNormal = CAULDRONIBSNULLVALUE;
      brineDensity = CAULDRONIBSNULLVALUE;
      criticalTemperature = CAULDRONIBSNULLVALUE;
   }

   for ( i = grid.firstI (); i <= grid.lastI (); ++i ) {

      for ( j = grid.firstJ (); j <= grid.lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

            for ( k = grid.firstK (); k <= grid.lastK (); ++k ) {

               const LayerElement& element = m_formation->getLayerElement ( i, j, k );

               if ( element.isActive ()) {

                  if ( not FastcauldronSimulator::getInstance ().useCalculatedCapillaryEntryPressure ()) {

                     lwcep = BrooksCorey::Pe;                     
                     vwcep = BrooksCorey::Pe;

                  } else {
                     elementComposition = concentrations ( k, j, i );
                     elementComposition *= molarMasses;

                     ves = computeProperty ( element, Basin_Modelling::VES_FP );
                     maxVes = computeProperty ( element, Basin_Modelling::Max_VES );

                     element.getLithology()->getPorosity ( ves, maxVes, false, 0.0, porosity );
                     element.getLithology()->calcBulkPermeabilityNP ( ves, maxVes, porosity, permeabilityNormal, permeabilityPlane );

                     // The critical temperature is not used as yet, because it is not always possible to compute it.
                     // This will be required to compute the interfacial tension. At the monent there are some default 
                     // values assigned to this for each of the phases.
#if 0
                     brinePressure = computeProperty ( element, Basin_Modelling::Pore_Pressure );
                     temperature = computeProperty ( element, Basin_Modelling::Temperature );
                     brineDensity = m_formation->fluid->density ( temperature, brinePressure );

                     criticalTemperature = PVTCalc::getInstance ().computeCriticalTemperature ( elementComposition );

                     if ( pvtProperties->getVolumeValue ( i, j, k, HcLiquidDensityIndex ) != CAULDRONIBSNULLVALUE ) {
                        lwcep = element.getLithology()->capillaryEntryPressure ( pvtFlash::LIQUID_PHASE,
                                                                                 temperature,
                                                                                 permeabilityNormal,
                                                                                 brineDensity,
                                                                                 pvtProperties->getVolumeValue ( i, j, k, HcLiquidDensityIndex ),
                                                                                 criticalTemperature );
                     } else {
                        lwcep = 0.0;
                     }

                     if ( pvtProperties->getVolumeValue ( i, j, k, HcVapourDensityIndex ) != CAULDRONIBSNULLVALUE ) {
                        vwcep = element.getLithology()->capillaryEntryPressure ( pvtFlash::VAPOUR_PHASE,
                                                                                 temperature,
                                                                                 permeabilityNormal,
                                                                                 brineDensity,
                                                                                 pvtProperties->getVolumeValue ( i, j, k, HcVapourDensityIndex ),
                                                                                 criticalTemperature );
                     } else {
                        vwcep = 0.0;
                     }
#endif

                     lwcep = element.getLithology()->capillaryEntryPressure ( pvtFlash::LIQUID_PHASE,
                                                                              CAULDRONIBSNULLVALUE,
                                                                              permeabilityNormal,
                                                                              CAULDRONIBSNULLVALUE,
                                                                              CAULDRONIBSNULLVALUE,
                                                                              CAULDRONIBSNULLVALUE );

                     vwcep = element.getLithology()->capillaryEntryPressure ( pvtFlash::VAPOUR_PHASE,
                                                                              CAULDRONIBSNULLVALUE,
                                                                              permeabilityNormal,
                                                                              CAULDRONIBSNULLVALUE,
                                                                              CAULDRONIBSNULLVALUE,
                                                                              CAULDRONIBSNULLVALUE );


                  }

                  liquidWaterCapEntryPressureMap->setValue ( i, j, k, lwcep );
                  vapourWaterCapEntryPressureMap->setValue ( i, j, k, vwcep );
               } else {
                  lwcep = undefinedValue;
                  vwcep = undefinedValue;

                  liquidWaterCapEntryPressureMap->setValue ( i, j, k, lwcep );
                  vapourWaterCapEntryPressureMap->setValue ( i, j, k, vwcep );
               }

               // adjust for element and node based difference
               // If last element in row or column or ... then copy value to fill array.
               // Since arrays are the same size as the number of nodes.
               if ( i == grid.getNumberOfXElements () - 1 ) {
                  liquidWaterCapEntryPressureMap->setValue ( i + 1, j, k, lwcep );
                  vapourWaterCapEntryPressureMap->setValue ( i + 1, j, k, vwcep );
               }

               if ( j == grid.getNumberOfYElements () - 1 ) {
                  liquidWaterCapEntryPressureMap->setValue ( i, j + 1, k, lwcep );
                  vapourWaterCapEntryPressureMap->setValue ( i, j + 1, k, vwcep );
               }

               if ( k == grid.getNumberOfZElements () - 1 ) {
                  liquidWaterCapEntryPressureMap->setValue ( i, j, k + 1, lwcep );
                  vapourWaterCapEntryPressureMap->setValue ( i, j, k + 1, vwcep );
               }

               if ( i == grid.getNumberOfXElements () - 1 and j == grid.getNumberOfYElements () - 1 ) {
                  liquidWaterCapEntryPressureMap->setValue ( i + 1, j + 1, k, lwcep );
                  vapourWaterCapEntryPressureMap->setValue ( i + 1, j + 1, k, vwcep );
               }

               if ( i == grid.getNumberOfXElements () - 1 and k == grid.getNumberOfZElements () - 1 ) {
                  liquidWaterCapEntryPressureMap->setValue ( i + 1, j, k + 1, lwcep );
                  vapourWaterCapEntryPressureMap->setValue ( i + 1, j, k + 1, vwcep );
               }

               if ( j == grid.getNumberOfYElements () - 1 and k == grid.getNumberOfZElements () - 1 ) {
                  liquidWaterCapEntryPressureMap->setValue ( i, j + 1, k + 1, lwcep );
                  vapourWaterCapEntryPressureMap->setValue ( i, j + 1, k + 1, vwcep );
               }

               if ( i == grid.getNumberOfXElements () - 1 and j == grid.getNumberOfYElements () - 1 and k == grid.getNumberOfZElements () - 1 ) {
                  liquidWaterCapEntryPressureMap->setValue ( i + 1, j + 1, k + 1, lwcep );
                  vapourWaterCapEntryPressureMap->setValue ( i + 1, j + 1, k + 1, vwcep );
               }

            }

         } else {

            for ( k = grid.firstK (); k <= grid.lastK (); ++k ) {
               liquidWaterCapEntryPressureMap->setValue(i,j,k,undefinedValue);
               vapourWaterCapEntryPressureMap->setValue(i,j,k,undefinedValue);
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

   // Restore the pressure to its original state.
   if ( not pressureIsActive ) {
      m_formation->Current_Properties.Restore_Property ( Basin_Modelling::Pore_Pressure );
   }

   // Restore the temperature to its original state.
   if ( not temperatureIsActive ) {
      m_formation->Current_Properties.Restore_Property ( Basin_Modelling::Temperature );
   }

   liquidWaterCapEntryPressureMap->restoreData ();
   vapourWaterCapEntryPressureMap->restoreData ();
   m_isCalculated = true;

   return true;
}
