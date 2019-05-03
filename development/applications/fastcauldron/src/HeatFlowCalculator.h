#ifndef _FASTCAULDRON_HEAT_FLOW_CALCULATOR_H_
#define _FASTCAULDRON_HEAT_FLOW_CALCULATOR_H_

#include "layer.h"
#include "CompoundLithologyArray.h"
#include "Property.h"
#include "PropertyValue.h"
#include "OutputPropertyMap.h"
#include "timefilter.h"

#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/GridMap.h"

class HeatFlowCalculator {

public :

   HeatFlowCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

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
   OutputPropertyMap* m_porePressure;
   OutputPropertyMap* m_overpressure;
   OutputPropertyMap* m_lithoPressure;
   OutputPropertyMap* m_ves;
   OutputPropertyMap* m_maxVes;
   OutputPropertyMap* m_chemicalCompaction;

   bool m_chemicalCompactionRequired;
   bool m_isCalculated;
   bool m_isBasementFormation;
   bool m_isBasementFormationAndALC;
   unsigned int m_kIndex;
   double m_zPosition;

   /// Other dependencies.
   const CompoundLithologyArray* m_lithologies;
   const FluidType*              m_fluid;

};

class HeatFlowVolumeCalculator {

public :

   HeatFlowVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot );

   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties,
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;
   const Interface::Snapshot* m_snapshot;

   OutputPropertyMap* m_depth;
   OutputPropertyMap* m_temperature;
   OutputPropertyMap* m_porePressure;
   OutputPropertyMap* m_overpressure;
   OutputPropertyMap* m_lithoPressure;
   OutputPropertyMap* m_ves;
   OutputPropertyMap* m_maxVes;
   OutputPropertyMap* m_chemicalCompaction;

   bool m_chemicalCompactionRequired;
   bool m_isCalculated;
   bool m_isBasementFormation;
   bool m_isBasementFormationAndALC;

   /// Other dependencies.
   const CompoundLithologyArray* m_lithologies;
   const FluidType*              m_fluid;

};

OutputPropertyMap* allocateHeatFlowCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateHeatFlowVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot );



#endif // _FASTCAULDRON_HEAT_FLOW_CALCULATOR_H_
