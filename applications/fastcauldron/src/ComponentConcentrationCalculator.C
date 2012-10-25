#include "ComponentConcentrationCalculator.h"

#include "EosPack.h"
#include "PetscBlockVector.h"
#include "ElementVolumeGrid.h"
#include "ComponentManager.h"
#include "PVTCalculator.h"

#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"
#include "MultiComponentFlowHandler.h"


OutputPropertyMap* allocateComponentConcentrationCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<ComponentConcentrationCalculator>( property, formation, surface, snapshot );
}

OutputPropertyMap* allocateComponentConcentrationVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<ComponentConcentrationVolumeCalculator>( property, formation, snapshot );
}



ComponentConcentrationVolumeCalculator::ComponentConcentrationVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {

   m_isCalculated = false;
   m_defaultMolarMasses = PVTCalc::getInstance ().getMolarMass ();
   m_defaultMolarMasses *= 1.0e-3;

}

void ComponentConcentrationVolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   PropertyValue* component;
   int i;

   for ( i = 0; i < NumberOfPVTComponents; ++i ) {
      component = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( CBMGenerics::ComponentManager::getInstance ().GetSpeciesName ( i ) + "Concentration", 
                                                                                                     m_snapshot, 0,
                                                                                                     m_formation,
                                                                                                     m_formation->getMaximumNumberOfElements () + 1 ));

      properties.push_back ( component );
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

   unsigned int i;
   unsigned int j;
   unsigned int k;
   unsigned int c;

   const ElementVolumeGrid& grid = m_formation->getVolumeGrid ( NumberOfPVTComponents );

   Interface::GridMap* componentMaps [ NumberOfPVTComponents ];

   PetscBlockVector<PVTComponents> components;
   //molar mass for conversion to kg/m3
   PVTComponents  molarMasses = m_defaultMolarMasses;

   components.setVector ( grid, m_formation->getPreviousComponentVec (), INSERT_VALUES );

   for ( c = 0; c < NumberOfPVTComponents; ++c ) {
      componentMaps [ c ] = propertyValues [ c ]->getGridMap ();
      componentMaps [ c ]->retrieveData ();
   }

  
   for ( c = 0; c < NumberOfPVTComponents; ++c ) {
      pvtFlash::ComponentId pvtComponent = pvtFlash::ComponentId ( c );

      for ( i = grid.firstI (); i <= grid.lastI (); ++i ) {

         for ( j = grid.firstJ (); j <= grid.lastJ (); ++j ) {

            if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

               for ( k = grid.firstK (); k <= grid.lastK (); ++k ) {
                  componentMaps [ c ]->setValue ( i, j, k, molarMasses(pvtComponent)*components ( k, j, i )( pvtComponent ));

                  // Fill other heat flow nodes if current (i,j) position is at end of array
                  if ( i == grid.getNumberOfXElements () - 1 ) {
                     componentMaps [ c ]->setValue ( i + 1, j, k, molarMasses(pvtComponent)*components ( k, j, i )( pvtComponent ));
                  }

                  if ( j == grid.getNumberOfYElements () - 1 ) {
                     componentMaps [ c ]->setValue ( i, j + 1, k, molarMasses(pvtComponent)*components ( k, j, i )( pvtComponent ));
                  }

                  if ( k == grid.getNumberOfZElements () - 1 ) {
                     componentMaps [ c ]->setValue ( i, j, k + 1, molarMasses(pvtComponent)*components ( k, j, i )( pvtComponent ));
                  }

                  if ( i == grid.getNumberOfXElements () - 1  and j == grid.getNumberOfYElements () - 1 ) {
                     componentMaps [ c ]->setValue ( i + 1, j + 1, k, molarMasses(pvtComponent)*components ( k, j, i )( pvtComponent ));
                  }

                  if ( i == grid.getNumberOfXElements () - 1  and k == grid.getNumberOfZElements () - 1 ) {
                     componentMaps [ c ]->setValue ( i + 1, j, k + 1, molarMasses(pvtComponent)*components ( k, j, i )( pvtComponent ));
                  }

                  if ( j == grid.getNumberOfYElements () - 1  and k == grid.getNumberOfZElements () - 1 ) {
                     componentMaps [ c ]->setValue ( i, j + 1, k + 1, molarMasses(pvtComponent)*components ( k, j, i )( pvtComponent ));
                  }

                  if ( i == grid.getNumberOfXElements () - 1  and j == grid.getNumberOfYElements () - 1  and k == grid.getNumberOfZElements () - 1 ) {
                     componentMaps [ c ]->setValue ( i + 1, j + 1, k + 1, molarMasses(pvtComponent)*components ( k, j, i )( pvtComponent ));
                  }

               }

            }

         }

      }

   }

   for ( c = 0; c < NumberOfPVTComponents; ++c ) {
      componentMaps [ c ]->restoreData ();
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

   PropertyValue* component;
   int i;

   for ( i = 0; i < NumberOfPVTComponents; ++i ) {
      component = (PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( CBMGenerics::ComponentManager::getInstance ().GetSpeciesName ( i ) + "Concentration", 
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
   //molar mass for conversion to kg/m3
   PVTComponents  molarMasses = m_defaultMolarMasses;

   components.setVector ( grid, m_formation->getPreviousComponentVec (), INSERT_VALUES );

   for ( c = 0; c < NumberOfPVTComponents; ++c ) {
      componentMaps [ c ] = propertyValues [ c ]->getGridMap ();
      componentMaps [ c ]->retrieveData ();
   }

   
   std::stringstream buffer;
   

   for ( c = 0; c < NumberOfPVTComponents; ++c ) {
      pvtFlash::ComponentId pvtComponent = pvtFlash::ComponentId ( c );

      for ( i = grid.firstI (); i <= grid.lastI (); ++i ) {

         for ( j = grid.firstJ (); j <= grid.lastJ (); ++j ) {

            if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {
               value = 0.0;

               for ( k = grid.firstK (); k <= grid.lastK (); ++k ) {
                  value += components ( k, j, i )( pvtComponent );
               }

               //value in moles/m3
               value = components ( 0, j, i )( pvtComponent );
               // //convert to kg/m3
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

   m_isCalculated = true;
   return true;
}

