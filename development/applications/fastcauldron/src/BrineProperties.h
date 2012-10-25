#ifndef _FASTCAULDRON_BRINE_PROPERTIES_CALCULATOR_H_
#define _FASTCAULDRON_BRINE_PROPERTIES_CALCULATOR_H_

#include "layer.h"
#include "CompoundLithologyArray.h"
#include "Property.h"
#include "PropertyValue.h"
#include "OutputPropertyMap.h"
#include "timefilter.h"

#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/GridMap.h"

#include "EosPack.h"


class BrinePropertiesVolumeCalculator {
public :

   BrinePropertiesVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot );
   virtual ~BrinePropertiesVolumeCalculator();

   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

   //virtual void setProperties() = 0;
 protected:

   LayerProps* m_formation;
   const Interface::Snapshot* m_snapshot;

   OutputPropertyMap* m_porosity;
   OutputPropertyMap* m_temperature;
   OutputPropertyMap* m_pressure;

  
   bool m_isCalculated;

   /// Other dependencies.
   const CompoundLithologyArray* m_lithologies;
   const FluidType*              m_fluid;
   
  
};


OutputPropertyMap* allocateBrinePropertiesVolumeCalculator ( const ::PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot );






#endif 
