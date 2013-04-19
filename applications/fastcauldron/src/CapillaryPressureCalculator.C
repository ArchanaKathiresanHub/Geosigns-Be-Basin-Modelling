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
#include "consts.h"

#include "timefilter.h"

#define DEBUG

using namespace FiniteElementMethod;
using namespace pvtFlash;

// OutputPropertyMap* allocateCapillaryPressureCalculator ( const ::PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
//    return new DerivedOutputPropertyMap<CapillaryPressureCalculator>( property, formation, surface, snapshot );
// }

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
   
   const double ConcentrationLowerLimit = 1.0e-20;
   
   for ( i = grid.firstI (); i <= grid.lastI (); ++i ) {

      for ( j = grid.firstJ (); j <= grid.lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

            for ( k = grid.firstK (); k <= grid.lastK (); ++k ) {

               const LayerElement& element = m_formation->getLayerElement ( i, j, k );

               if ( element.isActive ()) {

                  // get element saturation
                  saturation = saturations ( k, j, i );
					   
                  lwcp = element.getLithology()->capillaryPressure ( Saturation::LIQUID,
                                                                     saturation );

                  vwcp = element.getLithology()->capillaryPressure ( Saturation::VAPOUR,
                                                                     saturation );

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

   liquidWaterCapPressureMap->restoreData ();
   vapourWaterCapPressureMap->restoreData ();
   m_isCalculated = true;

   return true;
}


// //map
// CapillaryPressureCalculator::CapillaryPressureCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
//    m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {
//   m_isCalculated = false;
// }


// void CapillaryPressureCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

//  PropertyValue* phase;
//  phase = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "CapillaryPressureLiquidWater", 
//                                                                                               m_snapshot, 0,
//                                                                                               m_formation,
//                                                                                               m_formation->getMaximumNumberOfElements () + 1 ));
//  properties.push_back ( phase );

 
//  phase = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "CapillaryPressureVapourWater", 
//                                                                                               m_snapshot, 0,
//                                                                                               m_formation,
//                                                                                               m_formation->getMaximumNumberOfElements () + 1 )); 
//  properties.push_back ( phase );
  
// }

// CapillaryPressureCalculator::~CapillaryPressureCalculator() {
// }

// bool CapillaryPressureCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

//    m_temperature = PropertyManager::getInstance().findOutputPropertyMap ( "Temperature", m_formation, m_surface, m_snapshot );
//    m_pressure = PropertyManager::getInstance().findOutputPropertyMap ( "Pressure", m_formation, m_surface, m_snapshot );
//    m_porosity = PropertyManager::getInstance().findOutputPropertyMap ( "Porosity", m_formation, m_surface, m_snapshot );
//    m_lithologies = &m_formation->getCompoundLithologyArray ();
//    m_fluid = m_formation->fluid;
//    return true;
// }

// bool CapillaryPressureCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
//                                                OutputPropertyMap::PropertyValueList&  propertyValues )
// {   
// unsigned int i;
//    unsigned int j;
//    double capillaryPressure;
//    double undefinedValue;
//    double waterDensityValue = 0.0;
//    double tempValue = 0.0;
//    double pressValue = 0.0;
//    double porosityValue;
//    double hcDensityValue = 0.0;
//    double hcViscosityValue;
//    int hcPhase;
//    double hcTempValue = 0.0;

//    capPressureMap = propertyValues[0]->getGridMap ();
//    capPressureMap->retrieveData ();
//    undefinedValue = capPressureMap->getUndefinedValue ();

//    for (i = capPressureMap->firstI (); i <= capPressureMap->lastI (); ++i)
//    {

//       for (j = capPressureMap->firstJ (); j <= capPressureMap->lastJ (); ++j)
//       {

//          if (FastcauldronSimulator::getInstance ().nodeIsDefined (i, j))
//          {

//             tempValue = (*m_temperature) (i, j);
//             pressValue = (*m_pressure) (i, j);

//             waterDensityValue = m_fluid->density (tempValue, pressValue);

//             pvtFlash::EosPack::getInstance ().compute (tempValue + CBMGenerics::C2K,
//                                                        pressValue * CBMGenerics::MPa2Pa, m_componentId, 
//                                                        hcPhase, hcDensityValue, hcViscosityValue);

//             if (waterDensityValue <= hcDensityValue)
//             {
//                cerr << "water density (" << waterDensityValue << ") <= " <<
//                   pvtFlash::ComponentIdNames[m_componentId] << " density (" << hcDensityValue <<
//                   " for temperature " << tempValue + CBMGenerics::C2K << " K" <<
//                   ", pressure " << pressValue * CBMGenerics::MPa2Pa << " Pa" <<
//                   ", in phase " << pvtFlash::PVTPhaseNames[hcPhase] << endl;
//             }
//             porosityValue = 0.01 * (*m_porosity) (i, j);

//             hcTempValue = pvtFlash::getCriticalTemperature (m_componentId, 0);
            
//             capillaryPressure =
//                (*m_lithologies) (i, j)->capillaryPressure (m_phaseId, waterDensityValue, hcDensityValue,
//                                                            tempValue + CBMGenerics::C2K, hcTempValue, 
//                                                            m_waterSaturation, porosityValue);
//             if (isinf (capillaryPressure))
//             {
//                capillaryPressure = 1e24;
//             }
            
//             capPressureMap->setValue (i, j, 0.12345);
//          }
//          else
//          {
//             capPressureMap->setValue (i, j, undefinedValue);
//          }

//       }
//    }
//    liquidWaterCapPressureMap->restoreData ();
//    vapourWaterCapPressureMap->restoreData ();
//    m_isCalculated = true;

//    return true;
   
//}




