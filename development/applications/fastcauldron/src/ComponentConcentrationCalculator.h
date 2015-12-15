#ifndef _FASTCAULDRON__COMPONENT_CONCENTRATION_CALCULATOR__H_
#define _FASTCAULDRON__COMPONENT_CONCENTRATION_CALCULATOR__H_


#include "layer.h"
#include "CompoundLithologyArray.h"
#include "Property.h"
#include "PropertyValue.h"
#include "OutputPropertyMap.h"
#include "timefilter.h"

#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/GridMap.h"


class ComponentConcentrationCalculator {

public :

   ComponentConcentrationCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;
   const Interface::Surface*  m_surface;
   const Interface::Snapshot* m_snapshot;
   bool m_isCalculated;
   /// Default molar masses, kg/mol.
   PVTComponents m_defaultMolarMasses;
}; 


class ComponentConcentrationVolumeCalculator {

public :

   ComponentConcentrationVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot );

   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;
   const Interface::Snapshot* m_snapshot;
   bool m_isCalculated;
   /// Default molar masses, kg/mol.
   PVTComponents m_defaultMolarMasses;

}; 

OutputPropertyMap* allocateComponentConcentrationCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateComponentConcentrationVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot );

#endif // _FASTCAULDRON__COMPONENT_CONCENTRATION_CALCULATOR__H_
