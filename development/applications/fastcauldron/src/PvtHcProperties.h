#ifndef _FASTCAULDRON__PVT_HC_PROPERTIES_CALCULATOR__H_
#define _FASTCAULDRON__PVT_HC_PROPERTIES_CALCULATOR__H_

#include "layer.h"
#include "Property.h"
#include "PropertyValue.h"
#include "OutputPropertyMap.h"
#include "timefilter.h"

#include "Snapshot.h"
#include "GridMap.h"


class PvtHcPropertiesVolumeCalculator {

public :

   PvtHcPropertiesVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot );

   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties,
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;
   const Interface::Snapshot* m_snapshot;
   bool m_isCalculated;

};

OutputPropertyMap* allocatePvtHcPropertiesVolumeCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Snapshot* snapshot );

#endif // _FASTCAULDRON__PVT_HC_PROPERTIES_CALCULATOR__H_
