#ifndef _FASTCAULDRON_TWOWAYTIMECALCULATOR_H_
#define _FASTCAULDRON_TWOWAYTIMECALCULATOR_H_

#include "layer.h"
#include "Property.h"
#include "PropertyValue.h"
#include "OutputPropertyMap.h"
#include "timefilter.h"

#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/GridMap.h"

class TwoWayTimeCalculator {

public :

   TwoWayTimeCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );


   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;
   const Interface::Surface* m_surface;
   const Interface::Snapshot* m_snapshot;

   OutputPropertyMap* m_seismicVelocity;

   bool m_isCalculated;

};


class TwoWayTimeVolumeCalculator {

public :

   TwoWayTimeVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot );


   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;
   const Interface::Snapshot* m_snapshot;

   OutputPropertyMap* m_seismicVelocity;

   bool m_isCalculated;

};


OutputPropertyMap* allocateTwoWayTimeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateTwoWayTimeVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot );

#endif // _FASTCAULDRON_TWOWAYTIMECALCULATOR_H_