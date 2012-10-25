#ifndef _FASTCAULDRON__HC_FLUID_VELOCITY_CALCULATOR__H_
#define _FASTCAULDRON__HC_FLUID_VELOCITY_CALCULATOR__H_

#include "layer.h"
#include "CompoundLithologyArray.h"
#include "Property.h"
#include "PropertyValue.h"
#include "OutputPropertyMap.h"
#include "timefilter.h"

#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/GridMap.h"

// class HcFluidVelocityCalculator {

// public :

//    HcFluidVelocityCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

//    void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

//    bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
//                            OutputPropertyMap::PropertyValueList&  propertyValues );

//    bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

// private :

//    LayerProps* m_formation;
//    const Interface::Surface* m_surface;
//    const Interface::Snapshot* m_snapshot;

//    OutputPropertyMap* m_depth;
//    OutputPropertyMap* m_temperature;
//    OutputPropertyMap* m_hydrostaticPressure;
//    OutputPropertyMap* m_overpressure;
//    OutputPropertyMap* m_ves;
//    OutputPropertyMap* m_maxVes;
//    OutputPropertyMap* m_chemicalCompaction;

//    bool m_chemicalCompactionRequired;
//    bool m_isCalculated;
//    unsigned int m_kIndex;
//    double m_zPosition;

// };


class HcFluidVelocityVolumeCalculator {

public :

   HcFluidVelocityVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot );

   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;
   const Interface::Snapshot* m_snapshot;

   OutputPropertyMap* m_pvtProperties;

   bool m_isCalculated;

};

// OutputPropertyMap* allocateHcFluidVelocityCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateHcFluidVelocityVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot );

#endif // _FASTCAULDRON__HC_FLUID_VELOCITY_CALCULATOR__H_
