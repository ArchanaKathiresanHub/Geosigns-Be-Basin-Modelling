#ifndef _FASTCAULDRON_TIME_OF_ELEMENT_INVASION_CALCULATOR_H_
#define _FASTCAULDRON_TIME_OF_ELEMENT_INVASION_CALCULATOR_H_

#include "layer.h"
#include "Property.h"
#include "PropertyValue.h"
#include "OutputPropertyMap.h"
#include "timefilter.h"

#include "Surface.h"
#include "Snapshot.h"
#include "GridMap.h"

class TimeOfElementInvasionVolumeCalculator {

public :

   TimeOfElementInvasionVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot );


   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties,
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;

   const Interface::Snapshot* m_snapshot;

   bool m_isCalculated;

};


OutputPropertyMap* allocateTimeOfElementInvasionVolumeCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Snapshot* snapshot );

#endif
