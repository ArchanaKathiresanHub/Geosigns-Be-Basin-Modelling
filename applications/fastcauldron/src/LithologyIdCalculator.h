#ifndef _FASTCAULDRON_LITHOLOGY_ID_CALCULATOR_H_
#define _FASTCAULDRON_LITHOLOGY_ID_CALCULATOR_H_

#include "layer.h"
#include "CompoundLithologyArray.h"
#include "Property.h"
#include "PropertyValue.h"
#include "OutputPropertyMap.h"
#include "timefilter.h"

#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/GridMap.h"

class LithologyIdVolumeCalculator {

public :

   LithologyIdVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot );


   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;
   const Interface::Snapshot* m_snapshot;

   bool m_isCalculated;

   /// Other dependencies.
   const CompoundLithologyArray* m_lithologies;

};


OutputPropertyMap* allocateLithologyIdVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot );

#endif // _FASTCAULDRON_LITHOLOGY_ID_CALCULATOR_H_
