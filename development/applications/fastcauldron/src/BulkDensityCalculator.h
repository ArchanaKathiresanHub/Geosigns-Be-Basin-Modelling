#ifndef _FASTCAULDRON_BULK_DENSITY_CALCULATOR_H_
#define _FASTCAULDRON_BULK_DENSITY_CALCULATOR_H_

#include "layer.h"
#include "CompoundLithologyArray.h"
#include "GeoPhysicsFluidType.h"
#include "Property.h"
#include "PropertyValue.h"
#include "OutputPropertyMap.h"
#include "timefilter.h"

#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/GridMap.h"

class BulkDensityCalculator {

public :

   BulkDensityCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );


   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties,
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;
   const Interface::Surface* m_surface;
   const Interface::Snapshot* m_snapshot;

   OutputPropertyMap* m_porosity;
   OutputPropertyMap* m_pressure;
   OutputPropertyMap* m_lithopressure;
   OutputPropertyMap* m_temperature;

   bool m_isCalculated;
   unsigned int m_kIndex;
   bool m_isBasementFormationAndALC;

   /// Other dependencies.
   const CompoundLithologyArray* m_lithologies;
   const FluidType*              m_fluid;

};

class BulkDensityVolumeCalculator {

public :

   BulkDensityVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot );


   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties,
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;
   const Interface::Snapshot* m_snapshot;

   OutputPropertyMap* m_porosity;
   OutputPropertyMap* m_pressure;
   OutputPropertyMap* m_lithopressure;
   OutputPropertyMap* m_temperature;

   bool m_isCalculated;
   bool m_isBasementFormationAndALC;

   /// Other dependencies.
   const CompoundLithologyArray* m_lithologies;
   const FluidType*              m_fluid;

};


OutputPropertyMap* allocateBulkDensityCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateBulkDensityVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot );

#endif // _FASTCAULDRON_BULK_DENSITY_CALCULATOR_H_
