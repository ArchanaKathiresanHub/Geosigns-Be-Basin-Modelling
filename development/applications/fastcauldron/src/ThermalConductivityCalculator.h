#ifndef _FASTCAULDRON_THERMAL_CONDUCTIVITY_CALCULATOR_H_
#define _FASTCAULDRON_THERMAL_CONDUCTIVITY_CALCULATOR_H_

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

class ThermalConductivityCalculator {

public :

   ThermalConductivityCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;
   const Interface::Surface* m_surface;
   const Interface::Snapshot* m_snapshot;

   OutputPropertyMap* m_porosity;
   OutputPropertyMap* m_temperature;
   OutputPropertyMap* m_lithopressure;
   OutputPropertyMap* m_porePressure;

   unsigned int m_kIndex;

   bool m_isCalculated;
   bool m_isBasementFormationAndALC;

   /// Other dependencies.
   const CompoundLithologyArray* m_lithologies;
   const FluidType*              m_fluid;

};

class ThermalConductivityVolumeCalculator {

public :

   ThermalConductivityVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot );

   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;
   const Interface::Snapshot* m_snapshot;

   OutputPropertyMap* m_porosity;
   OutputPropertyMap* m_temperature;
   OutputPropertyMap* m_lithopressure;
   OutputPropertyMap* m_porePressure;

   bool m_isCalculated;
   bool m_isBasementFormationAndALC;

   /// Other dependencies.
   const CompoundLithologyArray* m_lithologies;
   const FluidType*              m_fluid;

};


OutputPropertyMap* allocateThermalConductivityCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateThermalConductivityVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot );


#endif // _THERMAL_CONDUCTIVITY_CALCULATOR_H_
