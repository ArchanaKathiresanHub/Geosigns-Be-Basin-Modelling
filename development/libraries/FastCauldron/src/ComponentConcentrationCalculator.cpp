//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "ComponentConcentrationCalculator.h"

#include "EosPack.h"
#include "PetscBlockVector.h"
#include "ElementVolumeGrid.h"
#include "ComponentManager.h"
#include "PVTCalculator.h"
#include "NumericFunctions.h"

#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"
#include "MultiComponentFlowHandler.h"
#include "ElementContributions.h"


OutputPropertyMap* allocateComponentConcentrationCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<ComponentConcentrationCalculator>( property, formation, surface, snapshot );
}

OutputPropertyMap* allocateComponentConcentrationVolumeCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<ComponentConcentrationVolumeCalculator>( property, formation, snapshot );
}



ComponentConcentrationVolumeCalculator::ComponentConcentrationVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {

   m_isCalculated = false;
   m_defaultMolarMasses = PVTCalc::getInstance ().getMolarMass ();
   m_defaultMolarMasses *= 1.0e-3;

}

void ComponentConcentrationVolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   CauldronPropertyValue* component;
   int i;

   for ( i = 0; i < NumberOfPVTComponents; ++i ) {
      component = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( CBMGenerics::ComponentManager::getInstance ().getSpeciesName ( i ) + "Concentration", 
                                                                                                     m_snapshot, 0,
                                                                                                     m_formation,
                                                                                                     m_formation->getMaximumNumberOfElements () + 1 ));

      properties.push_back ( component );
   }

   if ( FastcauldronSimulator::getInstance ().getMcfHandler ().outputElementMasses ()) {
      CauldronPropertyValue* masses;

      masses = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "ElementMass", 
                                                                                                  m_snapshot, 0,
                                                                                                  m_formation,
                                                                                                  m_formation->getMaximumNumberOfElements () + 1 ));
      properties.push_back ( masses );
   }

}

bool ComponentConcentrationVolumeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   if ( not FastcauldronSimulator::getInstance ().getMcfHandler ().containsFormation ( m_formation )) {

      OutputPropertyMap::PropertyValueList::iterator iter;

      for ( iter = propertyValues.begin (); iter != propertyValues.end (); ++iter ) {
         (*iter)->allowOutput ( false );
      }

   }

   return true;
}

bool ComponentConcentrationVolumeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                                OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   static const int quadratureDegree = 3;

   unsigned int i;
   unsigned int j;
   unsigned int k;
   unsigned int c;

   double elementAveragePoreVolume;
   double elementPoreVolume;
   double elementVolume;

   const ElementVolumeGrid& grid = m_formation->getVolumeGrid ( NumberOfPVTComponents );

   Interface::GridMap* componentMaps [ NumberOfPVTComponents ];
   Interface::GridMap* massMap = 0;
   double elementMass;

   PetscBlockVector<PVTComponents> components;
   //molar mass for conversion to kg/m3
   PVTComponents  molarMasses = m_defaultMolarMasses;

   Basin_Modelling::Fundamental_Property_Manager& currentProperties  = m_formation->Current_Properties;

   bool depthIsActive  = currentProperties.propertyIsActivated ( Basin_Modelling::Depth );
   bool vesIsActive    = currentProperties.propertyIsActivated ( Basin_Modelling::VES_FP );
   bool maxVesIsActive = currentProperties.propertyIsActivated ( Basin_Modelling::Max_VES );

   if ( not depthIsActive ) {
      currentProperties.Activate_Property ( Basin_Modelling::Depth, INSERT_VALUES, true );
   }

   if ( not vesIsActive ) {
      currentProperties.Activate_Property ( Basin_Modelling::VES_FP, INSERT_VALUES, true );
   }

   if ( not maxVesIsActive ) {
      currentProperties.Activate_Property ( Basin_Modelling::Max_VES, INSERT_VALUES, true );
   }

   components.setVector ( grid, m_formation->getPreviousComponentVec (), INSERT_VALUES );

   for ( c = 0; c < NumberOfPVTComponents; ++c ) {
      componentMaps [ c ] = propertyValues [ c ]->getGridMap ();
      componentMaps [ c ]->retrieveData ();
   }

   if ( FastcauldronSimulator::getInstance ().getMcfHandler ().outputElementMasses ()) {
      massMap = propertyValues [ NumberOfPVTComponents ]->getGridMap ();
      massMap->retrieveData ();
   } else {
      massMap = 0;
   }


   for ( i = grid.firstI (); i <= grid.lastI (); ++i ) {

      for ( j = grid.firstJ (); j <= grid.lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

            for ( k = grid.firstK (); k <= grid.lastK (); ++k ) {

               const LayerElement& element = m_formation->getLayerElement ( i, j,  k );

               if ( element.isActive ()) {
                  elementVolumeCalculations ( element, elementVolume, elementPoreVolume, quadratureDegree );

                  if ( elementVolume > 0.0 ) {
                     elementAveragePoreVolume = elementPoreVolume / elementVolume;
                  }

               } else {
                  elementAveragePoreVolume = 0.0;
               }

               elementMass = 0.0;

               for ( c = 0; c < NumberOfPVTComponents; ++c ) {
                  ComponentId pvtComponent = ComponentId ( c );

                  componentMaps [ c ]->setValue ( i, j, k, molarMasses(pvtComponent)*components ( k, j, i )( pvtComponent ));
                  elementMass += elementPoreVolume * molarMasses(pvtComponent)*components ( k, j, i )( pvtComponent );
                  
                  if ( massMap != 0 ) {
                     massMap->setValue ( i, j, k, elementMass );
                  }

                  // Fill other heat flow nodes if current (i,j) position is at end of array
                  if ( i == grid.getNumberOfXElements () - 1 ) {
                     componentMaps [ c ]->setValue ( i + 1, j, k, molarMasses(pvtComponent)*components ( k, j, i )( pvtComponent ));

                     if ( massMap != 0 ) {
                        massMap->setValue ( i + 1, j, k, elementMass );
                     }

                  }

                  if ( j == grid.getNumberOfYElements () - 1 ) {
                     componentMaps [ c ]->setValue ( i, j + 1, k, molarMasses(pvtComponent)*components ( k, j, i )( pvtComponent ));

                     if ( massMap != 0 ) {
                        massMap->setValue ( i, j + 1, k, elementMass );
                     }

                  }

                  if ( k == grid.getNumberOfZElements () - 1 ) {
                     componentMaps [ c ]->setValue ( i, j, k + 1, molarMasses(pvtComponent)*components ( k, j, i )( pvtComponent ));

                     if ( massMap != 0 ) {
                        massMap->setValue ( i, j, k + 1, elementMass );
                     }

                  }

                  if ( i == grid.getNumberOfXElements () - 1  and j == grid.getNumberOfYElements () - 1 ) {
                     componentMaps [ c ]->setValue ( i + 1, j + 1, k, molarMasses(pvtComponent)*components ( k, j, i )( pvtComponent ));

                     if ( massMap != 0 ) {
                        massMap->setValue ( i + 1, j + 1, k, elementMass );
                     }

                  }

                  if ( i == grid.getNumberOfXElements () - 1  and k == grid.getNumberOfZElements () - 1 ) {
                     componentMaps [ c ]->setValue ( i + 1, j, k + 1, molarMasses(pvtComponent)*components ( k, j, i )( pvtComponent ));

                     if ( massMap != 0 ) {
                        massMap->setValue ( i + 1, j, k + 1, elementMass );
                     }

                  }

                  if ( j == grid.getNumberOfYElements () - 1  and k == grid.getNumberOfZElements () - 1 ) {
                     componentMaps [ c ]->setValue ( i, j + 1, k + 1, molarMasses(pvtComponent)*components ( k, j, i )( pvtComponent ));

                     if ( massMap != 0 ) {
                        massMap->setValue ( i, j + 1, k + 1, elementMass );
                     }

                  }

                  if ( i == grid.getNumberOfXElements () - 1  and j == grid.getNumberOfYElements () - 1  and k == grid.getNumberOfZElements () - 1 ) {
                     componentMaps [ c ]->setValue ( i + 1, j + 1, k + 1, molarMasses(pvtComponent)*components ( k, j, i )( pvtComponent ));

                     if ( massMap != 0 ) {
                        massMap->setValue ( i + 1, j + 1, k + 1, elementMass );
                     }

                  }

               }

            }

         }

      }

   }

   for ( c = 0; c < NumberOfPVTComponents; ++c ) {
      componentMaps [ c ]->restoreData ();
   }

   if ( massMap != 0 ) {
      massMap->restoreData ();
   }

   if ( not depthIsActive ) {
      currentProperties.Restore_Property ( Basin_Modelling::Depth );
   }

   if ( not vesIsActive ) {
      currentProperties.Restore_Property ( Basin_Modelling::VES_FP );
   }

   if ( not maxVesIsActive ) {
      currentProperties.Restore_Property ( Basin_Modelling::Max_VES );
   }

   m_isCalculated = true;
   return true;
}




ComponentConcentrationCalculator::ComponentConcentrationCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {

   m_isCalculated = false;
   m_defaultMolarMasses = PVTCalc::getInstance ().getMolarMass ();
   m_defaultMolarMasses *= 1.0e-3;

}

void ComponentConcentrationCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   CauldronPropertyValue* component;
   int i;

   for ( i = 0; i < NumberOfPVTComponents; ++i ) {
      component = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( CBMGenerics::ComponentManager::getInstance ().getSpeciesName ( i ) + "Concentration", 
                                                                                                  m_snapshot, 0,
                                                                                                  m_formation,
                                                                                                  0 ));

      properties.push_back ( component );
   }

}

bool ComponentConcentrationCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {
   return true;
}

bool ComponentConcentrationCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                          OutputPropertyMap::PropertyValueList&  propertyValues ) {

   static const int quadratureDegree = 3;

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   unsigned int k;
   unsigned int c;

   const ElementVolumeGrid& grid = m_formation->getVolumeGrid ( NumberOfPVTComponents );

   Interface::GridMap* componentMaps [ NumberOfPVTComponents ];

   PetscBlockVector<PVTComponents> components;
   double value;
   double elementAveragePoreVolume;
   //molar mass for conversion to kg/m3
   // The mass per column of elements at position i,j.
   PVTComponents  massPerColumn;
   PVTComponents  molarMasses = m_defaultMolarMasses;

   Basin_Modelling::Fundamental_Property_Manager& currentProperties  = m_formation->Current_Properties;

   bool depthIsActive  = currentProperties.propertyIsActivated ( Basin_Modelling::Depth );
   bool vesIsActive    = currentProperties.propertyIsActivated ( Basin_Modelling::VES_FP );
   bool maxVesIsActive = currentProperties.propertyIsActivated ( Basin_Modelling::Max_VES );

   if ( not depthIsActive ) {
      currentProperties.Activate_Property ( Basin_Modelling::Depth, INSERT_VALUES, true );
   }

   if ( not vesIsActive ) {
      currentProperties.Activate_Property ( Basin_Modelling::VES_FP, INSERT_VALUES, true );
   }

   if ( not maxVesIsActive ) {
      currentProperties.Activate_Property ( Basin_Modelling::Max_VES, INSERT_VALUES, true );
   }

   components.setVector ( grid, m_formation->getPreviousComponentVec (), INSERT_VALUES );

   for ( c = 0; c < NumberOfPVTComponents; ++c ) {
      componentMaps [ c ] = propertyValues [ c ]->getGridMap ();
      componentMaps [ c ]->retrieveData ();
   }

   for ( i = grid.firstI (); i <= grid.lastI (); ++i ) {

      for ( j = grid.firstJ (); j <= grid.lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {
            value = 0.0;

            for ( c = 0; c < NumberOfPVTComponents; ++c ) {
               ComponentId pvtComponent = ComponentId ( c );

               for ( k = grid.firstK (); k <= grid.lastK (); ++k ) {

                  const LayerElement& element = m_formation->getLayerElement ( i, j,  k );

                  if ( element.isActive ()) {
                     elementAveragePoreVolume = averageElementPorosity ( element, quadratureDegree );
                     // value in moles per cubic-metre.
                     // value += components ( k, j, i )( pvtComponent );
                     value += elementAveragePoreVolume * components ( k, j, i )( pvtComponent );
                  }

               }

               // convert to kg per element.
               value *= molarMasses(pvtComponent);

               componentMaps [ c ]->setValue ( i, j, value );

               // Fill other heat flow nodes if current (i,j) position is at end of array
               if ( i == grid.getNumberOfXElements () - 1 ) {
                  componentMaps [ c ]->setValue ( i + 1, j, value );
               }
            
               if ( j == grid.getNumberOfYElements () - 1 ) {
                  componentMaps [ c ]->setValue ( i, j + 1, value );
               }

               if ( i == grid.getNumberOfXElements () - 1  and j == grid.getNumberOfYElements () - 1 ) {
                  componentMaps [ c ]->setValue ( i + 1, j + 1, value );
               }

            }

         }

      }

   }

   for ( c = 0; c < NumberOfPVTComponents; ++c ) {
      componentMaps [ c ]->restoreData ();
   }

   if ( not depthIsActive ) {
      currentProperties.Restore_Property ( Basin_Modelling::Depth );
   }

   if ( not vesIsActive ) {
      currentProperties.Restore_Property ( Basin_Modelling::VES_FP );
   }

   if ( not maxVesIsActive ) {
      currentProperties.Restore_Property ( Basin_Modelling::Max_VES );
   }


   m_isCalculated = true;
   return true;
}

