#ifndef FASTCAULDRON__FRACTURE_PRESSURE_VOLUME_CALCULATOR_H
#define FASTCAULDRON__FRACTURE_PRESSURE_VOLUME_CALCULATOR_H

#include "layer.h"
#include "CompoundLithologyArray.h"
#include "Property.h"
#include "PropertyValue.h"
#include "OutputPropertyMap.h"
#include "timefilter.h"

#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/GridMap.h"

class FracturePressureVolumeCalculator {

public :

   FracturePressureVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot );

   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;
   const Interface::Snapshot* m_snapshot;

   OutputPropertyMap* m_depth;
   OutputPropertyMap* m_hydrostaticPressure;
   OutputPropertyMap* m_lithostaticPressure;

   const CompoundLithologyArray* m_lithologies;
   bool m_isCalculated;


};

OutputPropertyMap* allocateFracturePressureVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot );


#endif // FASTCAULDRON__FRACTURE_PRESSURE_VOLUME_CALCULATOR_H
