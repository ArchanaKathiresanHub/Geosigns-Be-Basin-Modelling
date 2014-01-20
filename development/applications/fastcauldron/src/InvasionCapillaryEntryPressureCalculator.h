#ifndef _FASTCAULDRON_INVASION_CAPILLARY_ENTRY_PRESSURE_CALCULATOR_H_
#define _FASTCAULDRON_INVASION_CAPILLARY_ENTRY_PRESSURE_CALCULATOR_H_

#include "layer.h"
#include "CompoundLithologyArray.h"
#include "Property.h"
#include "PropertyValue.h"
#include "OutputPropertyMap.h"
#include "timefilter.h"

#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/GridMap.h"

#include "EosPack.h"


class InvasionCapillaryEntryPressureVolumeCalculator {
public :

   InvasionCapillaryEntryPressureVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot );
   virtual ~InvasionCapillaryEntryPressureVolumeCalculator();

   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private:

   LayerProps* m_formation;
   const Interface::Snapshot* m_snapshot;

   bool m_isCalculated;

};

OutputPropertyMap* allocateInvasionCapillaryEntryPressureVolumeCalculator ( const ::PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot );


#endif // _FASTCAULDRON_INVASION_CAPILLARY_ENTRY_PRESSURE_CALCULATOR_H_
