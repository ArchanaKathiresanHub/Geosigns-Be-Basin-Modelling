#ifndef _FASTCAULDRON_VELOCITY_CALCULATOR_H_
#define _FASTCAULDRON_VELOCITY_CALCULATOR_H_

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

/*! \class VelocityCalculator
* \brief Class used to compute the 2D seismic velocity (Velocity) property.
*/
class VelocityCalculator {

public :

   VelocityCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );


   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps          const * const m_formation;
   Interface::Surface  const * const m_surface;
   Interface::Snapshot const * const m_snapshot;

   OutputPropertyMap* m_bulkDensity;
   OutputPropertyMap* m_porosity;
   OutputPropertyMap* m_pressure;
   OutputPropertyMap* m_temperature;
   OutputPropertyMap* m_ves;
   OutputPropertyMap* m_maxVes;

   bool m_isCalculated;

   /// The lithologies of the formation
   CompoundLithologyArray const * const m_lithologies;
   /// The fluid of the formation
   FluidType const * const m_fluid;

};

/*! \class VelocityVolumeCalculator
* \brief Class used to compute the 3D seismic velocity (Velocity) property.
*/
class VelocityVolumeCalculator {

public :

   VelocityVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot );


   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps                const * const m_formation;
   Interface::Snapshot       const * const m_snapshot;

   OutputPropertyMap* m_bulkDensity;
   OutputPropertyMap* m_porosity;
   OutputPropertyMap* m_pressure;
   OutputPropertyMap* m_temperature;
   OutputPropertyMap* m_ves;
   OutputPropertyMap* m_maxVes;

   bool m_isCalculated;

   /// The lithologies of the formation
   CompoundLithologyArray const * const m_lithologies;
   /// The fluid of the formation
   FluidType const * const m_fluid;

};


OutputPropertyMap* allocateVelocityCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateVelocityVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot );

#endif // _FASTCAULDRON_VELOCITY_CALCULATOR_H_
