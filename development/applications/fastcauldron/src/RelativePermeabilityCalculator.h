#ifndef _FASTCUALDRON__RELATIVE_PERMEABILITY_CALCULATOR__H_
#define _FASTCUALDRON__RELATIVE_PERMEABILITY_CALCULATOR__H_

#include "layer.h"
#include "CompoundLithologyArray.h"
#include "Property.h"
#include "PropertyValue.h"
#include "OutputPropertyMap.h"
#include "timefilter.h"

#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/GridMap.h"


class RelativePermeabilityCalculator {

public :

   RelativePermeabilityCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot );

   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;
   const Interface::Snapshot* m_snapshot;

   bool m_isCalculated;

}; 

OutputPropertyMap* allocateRelativePermeabilityCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot );

#endif // _FASTCUALDRON__RELATIVE_PERMEABILITY_CALCULATOR__H_
