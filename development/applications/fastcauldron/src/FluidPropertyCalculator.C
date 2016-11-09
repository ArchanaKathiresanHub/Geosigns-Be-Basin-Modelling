//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "FluidPropertyCalculator.h"

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

#define DEBUG

// OutputPropertyMap* allocateFluidPropertyCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
//    return new DerivedOutputPropertyMap<FluidPropertyCalculator>( property, formation, surface, snapshot );
// }

OutputPropertyMap* allocateFluidPropertyVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<FluidPropertyVolumeCalculator>( property, formation, snapshot );
}



FluidPropertyVolumeCalculator::FluidPropertyVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {
   
   m_pressure = 0;
   m_temperature = 0;
   m_isCalculated = false;
}

void FluidPropertyVolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties )
{

   //order
   //0 -> GOR
   //1 -> CGR
   //2 -> OilApi
   //3 -> CondensateApi

   PropertyValue* phase;
   phase = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "GOR", 
                                                                                              m_snapshot, 0,
                                                                                              m_formation,
                                                                                              m_formation->getMaximumNumberOfElements () + 1 ));
   properties.push_back ( phase );
   
   
   phase = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "CGR", 
                                                                                              m_snapshot, 0,
                                                                                              m_formation,
                                                                                              m_formation->getMaximumNumberOfElements () + 1 )); 
   properties.push_back ( phase );
   
   phase = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "OilAPI", 
                                                                                              m_snapshot, 0,
                                                                                              m_formation,
                                                                                              m_formation->getMaximumNumberOfElements () + 1 )); 
   properties.push_back ( phase );
   
   phase = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "CondensateAPI", 
                                                                                              m_snapshot, 0,
                                                                                              m_formation,
                                                                                              m_formation->getMaximumNumberOfElements () + 1 )); 
   properties.push_back ( phase );
   
   
}

bool FluidPropertyVolumeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   if ( not FastcauldronSimulator::getInstance ().getMcfHandler ().containsFormation ( m_formation )) {

      OutputPropertyMap::PropertyValueList::iterator iter;

      for ( iter = propertyValues.begin (); iter != propertyValues.end (); ++iter ) {
         (*iter)->allowOutput ( false );
      }

   }

   return true;
}

bool FluidPropertyVolumeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                                OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

    bool temperatureIsActive = m_formation->Current_Properties.propertyIsActivated ( Basin_Modelling::Temperature );
   bool porePressureIsActive = m_formation->Current_Properties.propertyIsActivated ( Basin_Modelling::Pore_Pressure );

   if ( not temperatureIsActive ) {
      m_formation->Current_Properties.Activate_Property ( Basin_Modelling::Temperature, INSERT_VALUES, true );
   }

   if ( not porePressureIsActive ) {
      m_formation->Current_Properties.Activate_Property ( Basin_Modelling::Pore_Pressure, INSERT_VALUES, true );
   }


   
   unsigned int i;
   unsigned int j;
   unsigned int k;

   Interface::GridMap* gorMap;
   Interface::GridMap* cgrMap;
   Interface::GridMap* oilApiMap;
   Interface::GridMap* condensateApiMap;

   const ElementVolumeGrid& grid = m_formation->getVolumeGrid ( NumberOfPVTComponents);
   PetscBlockVector<PVTComponents> layerMolarConcentrations;
 // At this point previous and current have same values
   layerMolarConcentrations.setVector ( grid, m_formation->getPreviousComponentVec (), INSERT_VALUES );
   
   double value;
   double undefinedValue;
   double temperature;
   double pressure; 

   
   gorMap = propertyValues[0]->getGridMap ();
   gorMap->retrieveData ();
   cgrMap = propertyValues[1]->getGridMap ();
   cgrMap->retrieveData ();
   oilApiMap = propertyValues[2]->getGridMap ();
   oilApiMap->retrieveData ();
   condensateApiMap = propertyValues[3]->getGridMap ();
   condensateApiMap->retrieveData ();
   
   undefinedValue = gorMap->getUndefinedValue ();



   double freeGasVolume;
   double condensateVolume;
   double solutionGasVolume;
   double liquidOilVolume; // This is the volume of oil in the liquid phase.
   double condensateApi;
   double gor;
   double cgr;
   double oilApi;

   PVTPhaseComponents phaseComposition;
   PVTComponents      molarMasses;
   PVTPhaseValues     phaseDensities;
   PVTPhaseValues     phaseViscosities;
   PVTComponents      massConcentration;
   PVTComponents  vapourComponents;
   PVTComponents  liquidComponents;

   molarMasses = PVTCalc::getInstance ().getMolarMass ();
   molarMasses *= 1.0e-3;

                     
   // Standard conditions.
   double standardTemperature = 15.5555556 + 273.15; //Kelvin
   double standardPressure    = 101325.353; //Pa
                        
   for ( i = grid.firstI (); i <= grid.lastI (); ++i )
      {

      for ( j = grid.firstJ (); j <= grid.lastJ (); ++j )
         {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ())
            {

            for ( k = grid.firstK (); k <= grid.lastK (); ++k )
               {
                  const LayerElement& element = m_formation->getLayerElement ( i, j, k );
                  
                  if ( element.isActive ())
                     {
                        //=============Subsurface conditions ==============//
                        // Convert temperature to Kelvin.
                        temperature = computeProperty ( element, Basin_Modelling::Temperature ) + 273.15;
                        
                        // Convert pressure to Pa.
                        pressure = 1.0e6 * computeProperty ( element, Basin_Modelling::Pore_Pressure );
                        
                        // Get components that are to be flashed.
                        for ( unsigned int c = 0; c < NumberOfPVTComponents; ++c )
                           {
                              pvtFlash::ComponentId pvtComponent = pvtFlash::ComponentId ( c );
                              massConcentration ( pvtComponent ) = layerMolarConcentrations ( k, j, i )( pvtComponent );
                           }
                        
                        // Convert molar concentrations to mass concentrations mol/m^3 -> kg/m^3.
                        // Should use the molar-mass computed in PVT based on the concentrations.
                        massConcentration *= molarMasses;
                        
                        // Flash concentrations 
                        pvtFlash::EosPack::getInstance ().computeWithLumping ( temperature, pressure,
                                                                               massConcentration.m_components,
                                                                               phaseComposition.m_masses,
                                                                               phaseDensities.m_values,
                                                                               phaseViscosities.m_values );

                        //=============Standard P/T  conditions ==============//
                        

                        vapourComponents = phaseComposition.getPhaseComponents(pvtFlash::VAPOUR_PHASE );
                        liquidComponents = phaseComposition.getPhaseComponents(pvtFlash::LIQUID_PHASE );

                        pvtFlash::EosPack::getInstance ().computeWithLumping ( standardTemperature, standardPressure,
                                                                               vapourComponents.m_components,
                                                                               phaseComposition.m_masses,
                                                                               phaseDensities.m_values,
                                                                               phaseViscosities.m_values );

                        //Vapour Gas ?
                        if ( phaseDensities ( pvtFlash::VAPOUR_PHASE ) == 1000.0 ) {
                           // If there is no vapour then pvt returns 1000 for the density.
                           freeGasVolume = 0.0;
                        } else {
                           freeGasVolume = phaseComposition.sum ( pvtFlash::VAPOUR_PHASE ) / phaseDensities ( pvtFlash::VAPOUR_PHASE );
                        }

                        if ( phaseDensities ( pvtFlash::LIQUID_PHASE ) == 1000.0 ) {
                           // If there is no liquid then pvt returns 1000 for the density.
                           condensateVolume = 0.0;
                        } else {
                           condensateVolume = phaseComposition.sum ( pvtFlash::LIQUID_PHASE ) / phaseDensities ( pvtFlash::LIQUID_PHASE );
                        }
                        
                        //CondensateAPI
                        if ( phaseDensities ( pvtFlash::LIQUID_PHASE ) != 1000.0 )
                        {
                           condensateApi = 141.5 / phaseDensities ( pvtFlash::LIQUID_PHASE ) * 1000.0 - 131.5;
                              
                           if ( condensateApi < 1.99 )
                           {
                              condensateApi = undefinedValue;
                           }

                        }
                        else
                        {
                           condensateApi = undefinedValue;
                        }
                        
                        condensateApiMap->setValue(i,j,k,condensateApi);

                        //CGR
                        if ( freeGasVolume > 0.0 )
                        {
                           cgr = condensateVolume / freeGasVolume;
                        }
                        else
                        {
                           cgr = undefinedValue;
                        }
                        
                        cgrMap->setValue(i,j,k,cgr);


                        pvtFlash::EosPack::getInstance ().computeWithLumping ( standardTemperature, standardPressure,
                                                                               liquidComponents.m_components,
                                                                               phaseComposition.m_masses,
                                                                               phaseDensities.m_values,
                                                                               phaseViscosities.m_values );

                        //Liquid Gas ??
                        if ( phaseDensities ( pvtFlash::VAPOUR_PHASE ) == 1000.0 ) {
                           // If there is no vapour then pvt returns 1000 for the density.
                           solutionGasVolume = 0.0;
                        } else {
                           solutionGasVolume = phaseComposition.sum ( pvtFlash::VAPOUR_PHASE ) / phaseDensities ( pvtFlash::VAPOUR_PHASE );
                        }

                        if ( phaseDensities ( pvtFlash::LIQUID_PHASE ) == 1000.0 ) {
                           // If there is no liquid then pvt returns 1000 for the density.
                           liquidOilVolume = 0.0;
                        } else {
                           liquidOilVolume = phaseComposition.sum ( pvtFlash::LIQUID_PHASE ) / phaseDensities ( pvtFlash::LIQUID_PHASE );
                        }
                        
                        //OilAPI 
                        if ( phaseDensities ( pvtFlash::LIQUID_PHASE ) != 1000.0 )
                        {
                           oilApi = 141.5 / phaseDensities ( pvtFlash::LIQUID_PHASE ) * 1000.0 - 131.5;
                           
                           if ( oilApi < 1.99 )
                           {
                              oilApi = undefinedValue;
                           }
                           
                        }
                        else
                        {
                           oilApi = undefinedValue;
                        }

                        oilApiMap->setValue(i,j,k,oilApi);

                        //GOR
                        if ( liquidOilVolume > 0.0 )
                        {
                           gor = solutionGasVolume / liquidOilVolume;
                        }
                        else
                        {
                           gor = undefinedValue;
                        }

                        gorMap->setValue(i,j,k,gor);
                        
                     }
                  else
                     {
                        condensateApiMap->setValue(i,j,k,undefinedValue);
                        cgrMap->setValue(i,j,k,undefinedValue);
                        oilApiMap->setValue(i,j,k,undefinedValue);
                        gorMap->setValue(i,j,k,undefinedValue);
                     }

                  // adjust for element and node based difference
                  // If last element in row or column or ... then copy value to fill array.
                  // Since arrays are the same size as the number of nodes.
                  if ( i == grid.getNumberOfXElements () - 1 ) {
                     
                     condensateApiMap->setValue ( i + 1, j, k, condensateApiMap->getValue ( i, j, k ));
                     cgrMap->setValue ( i + 1, j, k, cgrMap->getValue ( i, j, k ));
                     oilApiMap->setValue ( i + 1, j, k, oilApiMap->getValue ( i, j, k ));
                     gorMap->setValue ( i + 1, j, k, gorMap->getValue ( i, j, k ));
                  }
                  
                  if ( j == grid.getNumberOfYElements () - 1 ) {
                     
                     condensateApiMap->setValue ( i, j + 1, k, condensateApiMap->getValue ( i, j, k ));
                     cgrMap->setValue ( i, j + 1, k, cgrMap->getValue ( i, j, k ));
                     oilApiMap->setValue ( i, j + 1, k, oilApiMap->getValue ( i, j, k ));
                     gorMap->setValue ( i, j + 1, k, gorMap->getValue ( i, j, k ));
                  }
                  
                  if ( k == grid.getNumberOfZElements () - 1 ) {
                     
                     condensateApiMap->setValue ( i, j, k + 1, condensateApiMap->getValue ( i, j, k ));
                     cgrMap->setValue ( i, j, k + 1, cgrMap->getValue ( i, j, k ));
                     oilApiMap->setValue ( i, j, k + 1, oilApiMap->getValue ( i, j, k ));
                     gorMap->setValue ( i, j, k + 1, gorMap->getValue ( i, j, k ));
                  }
                  
                  if ( i == grid.getNumberOfXElements () - 1 and j == grid.getNumberOfYElements () - 1 ) {
                     
                     condensateApiMap->setValue ( i + 1, j + 1, k, condensateApiMap->getValue ( i, j, k ));
                     cgrMap->setValue ( i + 1, j + 1, k, cgrMap->getValue ( i, j, k ));
                     oilApiMap->setValue ( i + 1, j + 1, k, oilApiMap->getValue ( i, j, k ));
                     gorMap->setValue ( i + 1, j + 1, k, gorMap->getValue ( i, j, k ));
                  }
                  
                  if ( i == grid.getNumberOfXElements () - 1 and k == grid.getNumberOfZElements () - 1 ) {
                     
                     condensateApiMap->setValue ( i + 1, j, k + 1, condensateApiMap->getValue ( i, j, k ));
                     cgrMap->setValue ( i + 1, j, k + 1, cgrMap->getValue ( i, j, k ));
                     oilApiMap->setValue ( i + 1, j, k + 1, oilApiMap->getValue ( i, j, k ));
                     gorMap->setValue ( i + 1, j, k + 1, gorMap->getValue ( i, j, k ));
                  }
                  
                  if ( j == grid.getNumberOfYElements () - 1 and k == grid.getNumberOfZElements () - 1 ) {
                     
                     condensateApiMap->setValue ( i, j + 1, k + 1, condensateApiMap->getValue ( i, j, k ));
                     cgrMap->setValue ( i, j + 1, k + 1, cgrMap->getValue ( i, j, k ));
                     oilApiMap->setValue ( i, j + 1, k + 1, oilApiMap->getValue ( i, j, k ));
                     gorMap->setValue ( i, j + 1, k + 1, gorMap->getValue ( i, j, k ));
                  }
                  
                  if ( i == grid.getNumberOfXElements () - 1 and j == grid.getNumberOfYElements () - 1 and k == grid.getNumberOfZElements () - 1 ) {
                     
                     condensateApiMap->setValue ( i + 1, j + 1, k + 1, condensateApiMap->getValue ( i, j, k ));
                     cgrMap->setValue ( i + 1, j + 1, k + 1, cgrMap->getValue ( i, j, k ));
                     oilApiMap->setValue ( i + 1, j + 1, k + 1, oilApiMap->getValue ( i, j, k ));
                     gorMap->setValue ( i + 1, j + 1, k + 1, gorMap->getValue ( i, j, k ));
                  }

               }
            }
         }
      }
   
   
   
   gorMap->restoreData();
   cgrMap->restoreData();
   oilApiMap->restoreData();
   condensateApiMap->restoreData();
   m_isCalculated = true;
   return true;
}




// FluidPropertyCalculator::FluidPropertyCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
//    m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {

//    m_isCalculated = false;
//    m_defaultMolarMasses = PVTCalc::getInstance ().getMolarMass ();
//    m_defaultMolarMasses *= 1.0e-3;

// }

// void FluidPropertyCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

//    PropertyValue* phase;
//  phase = (PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "GOR", 
//                                                                                               m_snapshot, 0,
//                                                                                               m_formation,
//                                                                                               0 ));
//  properties.push_back ( phase );

 
//  phase = (PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "COR", 
//                                                                                               m_snapshot, 0,
//                                                                                               m_formation,
//                                                                                               0 )); 
//  properties.push_back ( phase );

//   phase = (PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "OilAPI", 
//                                                                                               m_snapshot, 0,
//                                                                                               m_formation,
//                                                                                               0)); 
//  properties.push_back ( phase );

//   phase = (PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "CondensateAPI", 
//                                                                                               m_snapshot, 0,
//                                                                                               m_formation,
//                                                                                               0 )); 
//  properties.push_back ( phase );
 

// }

// bool FluidPropertyCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {
//    return true;
// }

// bool FluidPropertyCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
//                                                           OutputPropertyMap::PropertyValueList&  propertyValues ) {

//    if ( m_isCalculated ) {
//       return true;
//    }

//    m_isCalculated = true;
//    return true;
// }
