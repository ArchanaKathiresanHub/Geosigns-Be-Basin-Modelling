#ifndef _FASTCAULDRON_EROSION_FACTOR_CALCULATOR_H_
#define _FASTCAULDRON_EROSION_FACTOR_CALCULATOR_H_

#include "layer.h"
#include "Property.h"
#include "PropertyValue.h"
#include "OutputPropertyMap.h"
#include "timefilter.h"

#include "Surface.h"
#include "Snapshot.h"
#include "GridMap.h"

class ErosionFactorCalculator {

public :

   ErosionFactorCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );


   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties,
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;
   const Interface::Surface* m_surface;
   const Interface::Snapshot* m_snapshot;

   bool m_isCalculated;

};


OutputPropertyMap* allocateErosionFactorCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

#endif // _FASTCAULDRON_EROSION_FACTOR_CALCULATOR_H_
