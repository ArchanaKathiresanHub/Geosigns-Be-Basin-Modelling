#ifndef _FASTCAULDRON_FLUID_VELOCITY_CALCULATOR_H_
#define _FASTCAULDRON_FLUID_VELOCITY_CALCULATOR_H_

#include "layer.h"
#include "CompoundLithologyArray.h"
#include "Property.h"
#include "PropertyValue.h"
#include "OutputPropertyMap.h"
#include "timefilter.h"

#include "Surface.h"
#include "Snapshot.h"
#include "GridMap.h"

class FluidVelocityCalculator {

public :

   FluidVelocityCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;
   const Interface::Surface* m_surface;
   const Interface::Snapshot* m_snapshot;

   OutputPropertyMap* m_depth;
   OutputPropertyMap* m_temperature;
   OutputPropertyMap* m_hydrostaticPressure;
   OutputPropertyMap* m_overpressure;
   OutputPropertyMap* m_ves;
   OutputPropertyMap* m_maxVes;
   OutputPropertyMap* m_chemicalCompaction;

   bool m_chemicalCompactionRequired;
   bool m_isCalculated;
   unsigned int m_kIndex;
   double m_zPosition;

   /// Other dependencies.
   const CompoundLithologyArray* m_lithologies;
   const FluidType*              m_fluid;

};

class FluidVelocityVolumeCalculator {

public :

   FluidVelocityVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot );

   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;
   const Interface::Snapshot* m_snapshot;

   OutputPropertyMap* m_depth;
   OutputPropertyMap* m_temperature;
   OutputPropertyMap* m_hydrostaticPressure;
   OutputPropertyMap* m_overpressure;
   OutputPropertyMap* m_ves;
   OutputPropertyMap* m_maxVes;
   OutputPropertyMap* m_chemicalCompaction;

   bool m_chemicalCompactionRequired;
   bool m_isCalculated;

   /// Other dependencies.
   const CompoundLithologyArray* m_lithologies;
   const FluidType*              m_fluid;

};

OutputPropertyMap* allocateFluidVelocityCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateFluidVelocityVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot );


#endif // _FASTCAULDRON_FLUID_VELOCITY_CALCULATOR_H_
