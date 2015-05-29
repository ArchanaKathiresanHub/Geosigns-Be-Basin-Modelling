#ifndef _FASTCAULDRON_TWOWAYTIMECALCULATOR_H_
#define _FASTCAULDRON_TWOWAYTIMECALCULATOR_H_

#include "GeoPhysicsFluidType.h"
#include "layer.h"
#include "OutputPropertyMap.h"
#include "Property.h"
#include "PropertyValue.h"
#include "timefilter.h"

#include "Interface/GridMap.h"
#include "Interface/Snapshot.h"
#include "Interface/Surface.h"


/*! \class TwoWayTimeCalculator
* \brief Class used to compute the 2D two way travel time (TwoWayTime) property.
*/
class TwoWayTimeCalculator {

public :

   TwoWayTimeCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );


   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps const * const m_formation;
   /// The formation above the m_formation if there is one
   LayerProps const * const m_topFormation;
   /// The formation under the m_formation if there is one
   LayerProps const * const m_bottomFormation;
   Interface::Surface  const * const m_surface;
   Interface::Snapshot const * const m_snapshot;

   OutputPropertyMap * m_depth;
   OutputPropertyMap * m_pressure;
   OutputPropertyMap * m_seismicVelocity;
   OutputPropertyMap * m_temperature;
   /// The TwoWayTime of the formation above the m_formation (m_topFormation) if there is one
   OutputPropertyMap * m_twoWayTimeTop;

   bool m_isCalculated;

};

/*! \class TwoWayTimeVolumeCalculator
* \brief Class used to compute the 3D two way travel time (TwoWayTime) property
*/
class TwoWayTimeVolumeCalculator {

public :

   TwoWayTimeVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot );


   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps          const * const m_formation;
   Interface::Snapshot const * const m_snapshot;

   OutputPropertyMap* m_depth;
   OutputPropertyMap* m_seismicVelocity;
   /// The TwoWayTime of the formation above the m_formation (m_topFormation) if there is one
   OutputPropertyMap* m_twoWayTimeTop;

   bool m_isCalculated;

};

/*! \class TwoWayTimeResidualCalculator
* \brief Class used to compute the 2D two way travel time residual (TwoWayTimeResidual) property.
* \details This is the difference between the 2D two way time map linked to the surface stratigraphy in TwoWayTimeIoTbl (input by user)
*    and the two way time 2D property map computed in fastcauldron : TwoWayTimeResidual = TwoWayTimeCauldron - TwoWayTimeInput.
*/
class TwoWayTimeResidualCalculator {

public:

   TwoWayTimeResidualCalculator( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );


   void allocatePropertyValues( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties,
      OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise( OutputPropertyMap::PropertyValueList& propertyValues );

private:

   LayerProps          const * const m_formation;
   Interface::Surface  const * const m_surface;
   Interface::Snapshot const * const m_snapshot;

   /// The two way travel time maps computed from TwoWayTimeCalculator
   OutputPropertyMap * m_twoWayTimeCauldron;
   /// The initial two way travel time (at t=0Ma) of m_surface, this is a user input
   const Interface::GridMap * m_twoWayTimeInitial;

   bool m_isCalculated;

};


OutputPropertyMap* allocateTwoWayTimeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateTwoWayTimeVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateTwoWayTimeResidualCalculator( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

#endif // _FASTCAULDRON_TWOWAYTIMECALCULATOR_H_