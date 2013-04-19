#ifndef _FASTCAULDRON_SMECTITE_ILLITE_ADAPTER_H_
#define _FASTCAULDRON_SMECTITE_ILLITE_ADAPTER_H_

#include "layer.h"
#include "CompoundLithologyArray.h"
#include "Property.h"
#include "PropertyValue.h"
#include "OutputPropertyMap.h"
#include "timefilter.h"

#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/GridMap.h"

class SmectiteIlliteAdapter {

public :

   SmectiteIlliteAdapter ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );


   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;
   const Interface::Surface* m_surface;
   const Interface::Snapshot* m_snapshot;

   bool m_isCalculated;
   unsigned int m_kIndex;

};



class SmectiteIlliteVolumeAdapter {

public :

   SmectiteIlliteVolumeAdapter ( LayerProps* formation, const Interface::Snapshot* snapshot );


   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;
   const Interface::Snapshot* m_snapshot;

   bool m_isCalculated;

};


OutputPropertyMap* allocateSmectiteIlliteAdapter ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateSmectiteIlliteVolumeAdapter ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot );

#endif // _FASTCAULDRON_SMECTITE_ILLITE_ADAPTER_H_
