#ifndef _FASTCAULDRON_THICKNESS_CALCULATOR_H_
#define _FASTCAULDRON_THICKNESS_CALCULATOR_H_

#include "layer.h"
#include "CompoundLithologyArray.h"
#include "Property.h"
#include "PropertyValue.h"
#include "OutputPropertyMap.h"
#include "timefilter.h"

#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/GridMap.h"

class ThicknessCalculator {

public :

   ThicknessCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;
   const Interface::Surface* m_surface;
   const Interface::Snapshot* m_snapshot;

   OutputPropertyMap* m_depthAbove;
   OutputPropertyMap* m_depthBelow;

   bool m_isCalculated;

   /// Other dependencies.

};

OutputPropertyMap* allocateThicknessCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );


#endif // _FASTCAULDRON_THICKNESS_CALCULATOR_H_
