#ifndef _FASTCAULDRON__SATURATION_CALCULATOR__H_
#define _FASTCAULDRON__SATURATION_CALCULATOR__H_


#include "layer.h"
#include "CompoundLithologyArray.h"
#include "Property.h"
#include "PropertyValue.h"
#include "OutputPropertyMap.h"
#include "timefilter.h"

#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/GridMap.h"


class SaturationCalculator {

public :

   SaturationCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;
   const Interface::Surface*  m_surface;
   const Interface::Snapshot* m_snapshot;
   bool m_isCalculated;

}; 


class SaturationVolumeCalculator {

public :

   SaturationVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot );

   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;
   const Interface::Snapshot* m_snapshot;
   bool m_isCalculated;

}; 

OutputPropertyMap* allocateSaturationCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateSaturationVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot );


//averaged saturation for map

class AverageSaturationCalculator {

public :

   AverageSaturationCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;
   const Interface::Surface*  m_surface;
   const Interface::Snapshot* m_snapshot;
   bool m_isCalculated;

}; 

//average calculation for 3d
class AverageSaturationVolumeCalculator {

public :

   AverageSaturationVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot );

   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;
   const Interface::Snapshot* m_snapshot;
   bool m_isCalculated;

}; 

OutputPropertyMap* allocateAverageSaturationCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateAverageSaturationVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot );





#endif // _FASTCAULDRON__SATURATION_CALCULATOR__H_
