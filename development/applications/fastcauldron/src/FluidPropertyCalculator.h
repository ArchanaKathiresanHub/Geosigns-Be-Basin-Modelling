#ifndef _FASTCAULDRON__FLUID__PROPERTY__CALCULATOR__H_
#define _FASTCAULDRON__FLUID__PROPERTY__CALCULATOR__H_

#include "layer.h"
#include "CompoundLithologyArray.h"
#include "Property.h"
#include "PropertyValue.h"
#include "OutputPropertyMap.h"
#include "timefilter.h"

#include "Surface.h"
#include "Snapshot.h"
#include "GridMap.h"

#include "EosPack.h"






//volume
class FluidPropertyVolumeCalculator {

public :

   FluidPropertyVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot );

   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;
   const Interface::Snapshot* m_snapshot;
   bool m_isCalculated;
   /// Default molar masses, kg/mol.
   PVTComponents m_defaultMolarMasses;
    OutputPropertyMap* m_pressure;
   OutputPropertyMap* m_temperature;

}; 


OutputPropertyMap* allocateFluidPropertyVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot );



/* //map */
/* class FluidPropertyCalculator { */

/* public : */

/*    FluidPropertyCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ); */

/*    void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ); */

/*    bool operator ()( const OutputPropertyMap::OutputPropertyList& properties,  */
/*                            OutputPropertyMap::PropertyValueList&  propertyValues ); */

/*    bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues ); */

/* private : */

/*    LayerProps* m_formation; */
/*    const Interface::Surface*  m_surface; */
/*    const Interface::Snapshot* m_snapshot; */
/*    bool m_isCalculated; */
/*    /// Default molar masses, kg/mol. */
/*    PVTComponents m_defaultMolarMasses; */

   
/*    OutputPropertyMap* m_pressure; */
/*    OutputPropertyMap* m_temperature; */
/* };  */


/* OutputPropertyMap* allocateFluidPropertyCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot); */

#endif
