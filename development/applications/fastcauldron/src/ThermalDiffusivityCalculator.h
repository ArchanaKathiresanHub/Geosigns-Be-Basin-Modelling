#ifndef _FASTCAULDRON_DIFFUSIVITY_CALCULATOR_H_
#define _FASTCAULDRON_DIFFUSIVITY_CALCULATOR_H_

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

class ThermalDiffusivityCalculator {

public :

   ThermalDiffusivityCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );


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
   OutputPropertyMap* m_pressure;
   OutputPropertyMap* m_lithopressure;

   bool m_isCalculated;
   bool m_isBasementFormationAndALC;

   /// Other dependencies.
   const CompoundLithologyArray* m_lithologies;
   const FluidType*              m_fluid;
   AppCtx*                       m_BasinModel;

   unsigned int m_kIndex;
};


class ThermalDiffusivityVolumeCalculator {

public :

   ThermalDiffusivityVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot );


   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;
   const Interface::Snapshot* m_snapshot;

   OutputPropertyMap* m_porosity;
   OutputPropertyMap* m_temperature;
   OutputPropertyMap* m_pressure;
   OutputPropertyMap* m_lithopressure;

   bool m_isCalculated;
   bool m_isBasementFormationAndALC;

   /// Other dependencies.
   const CompoundLithologyArray* m_lithologies;
   const FluidType*              m_fluid;
   AppCtx*                       m_BasinModel;
};


OutputPropertyMap* allocateThermalDiffusivityCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateThermalDiffusivityVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot );

#endif // _FASTCAULDRON_DIFFUSIVITY_CALCULATOR_H_
