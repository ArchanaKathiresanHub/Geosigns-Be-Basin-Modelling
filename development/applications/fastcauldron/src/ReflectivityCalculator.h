#ifndef _FASTCAULDRON_REFLECTIVITY_CALCULATOR_H_
#define _FASTCAULDRON_REFLECTIVITY_CALCULATOR_H_

#include <vector>

#include "layer.h"
#include "CompoundLithologyArray.h"
#include "GeoPhysicsFluidType.h"
#include "Property.h"
#include "PropertyValue.h"
#include "OutputPropertyMap.h"
#include "timefilter.h"

#include "Surface.h"
#include "Snapshot.h"
#include "GridMap.h"

class ReflectivityCalculator {

public :

   ReflectivityCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );


   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   bool findPropertiesAbove ( std::vector<OutputPropertyMap*>& allThicknesses,
                              std::vector<OutputPropertyMap*>& allBulkDensities,
                              std::vector<OutputPropertyMap*>& allVelocities );

   LayerProps* m_layer;
   const Interface::Surface* m_surface;
   const Interface::Snapshot* m_snapshot;

   OutputPropertyMap* m_velocity;
   OutputPropertyMap* m_bulkDensity;
   OutputPropertyMap* m_thickness;

   bool m_isCalculated;
   bool m_computeReflectivity;
   double m_prescribedValue;

};

 
class ReflectivityVolumeCalculator {

public :

   ReflectivityVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot );


   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   bool findPropertiesAbove ( std::vector<OutputPropertyMap*>& allThicknesses,
                              std::vector<OutputPropertyMap*>& allBulkDensities,
                              std::vector<OutputPropertyMap*>& allVelocities );

   LayerProps* m_layer;
   const Interface::Snapshot* m_snapshot;

   OutputPropertyMap* m_velocity;
   OutputPropertyMap* m_bulkDensity;
   OutputPropertyMap* m_thickness;

   bool m_isCalculated;
   bool m_computeReflectivity;
   double m_prescribedValue;

};



OutputPropertyMap* allocateReflectivityCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateReflectivityVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot );


#endif // _FASTCAULDRON_REFLECTIVITY_CALCULATOR_H_
