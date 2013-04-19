#ifndef _FASTCAULDRON_CAPILLARY_PRESSURE_CALCULATOR_H_
#define _FASTCAULDRON_CAPILLARY_PRESSURE_CALCULATOR_H_

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


class CapillaryPressureVolumeCalculator {
public :

   CapillaryPressureVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot );
   virtual ~CapillaryPressureVolumeCalculator();

   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

   //virtual void setProperties() = 0;
 protected:

   LayerProps* m_formation;
   const Interface::Snapshot* m_snapshot;

   OutputPropertyMap* m_porosity;
   OutputPropertyMap* m_temperature;
   OutputPropertyMap* m_pressure;

  
   bool m_isCalculated;

   /// Other dependencies.
   const CompoundLithologyArray* m_lithologies;
   const FluidType*              m_fluid;
   
  
};


OutputPropertyMap* allocateCapillaryPressureVolumeCalculator ( const ::PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot );


//disabled the map calculation 
/* class CapillaryPressureCalculator { */

/* public : */
/*    CapillaryPressureCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ); */

/*    virtual ~CapillaryPressureCalculator(); */

/*    void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ); */

/*    bool operator ()( const OutputPropertyMap::OutputPropertyList& properties,  */
/*                            OutputPropertyMap::PropertyValueList&  propertyValues ); */

/*    bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues ); */

/*    //virtual void setProperties() = 0; */
/* protected : */

/*    LayerProps* m_formation; */
/*    const Interface::Surface* m_surface; */
/*    const Interface::Snapshot* m_snapshot; */

/*    OutputPropertyMap* m_porosity; */
/*    OutputPropertyMap* m_temperature; */
/*    OutputPropertyMap* m_pressure; */

/*    bool m_isCalculated; */
/*    unsigned int m_kIndex; */
/*    double m_zPosition; */

/*    /// Other dependencies. */
/*    const CompoundLithologyArray* m_lithologies; */
/*    const FluidType*              m_fluid; */

/*    double m_waterSaturation; */
/*    int m_componentId; */
/*    unsigned int m_phaseId; */

/*    const char * m_name; */
/* }; */






//for map
// OutputPropertyMap* allocateCapillaryPressureCalculator ( const ::PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );
//for volume






#endif // _FASTCAULDRON_CAPILLARY_PRESSURE_CALCULATOR_H_
