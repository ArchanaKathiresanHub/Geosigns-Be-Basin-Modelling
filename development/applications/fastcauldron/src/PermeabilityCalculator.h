#ifndef _FASTCAULDRON_PERMEABILITY_CALCULATOR_H_
#define _FASTCAULDRON_PERMEABILITY_CALCULATOR_H_

#include "layer.h"
#include "CompoundLithologyArray.h"
#include "Property.h"
#include "PropertyValue.h"
#include "OutputPropertyMap.h"
#include "timefilter.h"

#include "Surface.h"
#include "Snapshot.h"
#include "GridMap.h"

class PermeabilityCalculator {

public :

   PermeabilityCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;
   const Interface::Surface* m_surface;
   const Interface::Snapshot* m_snapshot;

   /// Property dependencies.
   OutputPropertyMap* m_ves;
   OutputPropertyMap* m_maxVes;
   OutputPropertyMap* m_chemicalCompaction;

   bool m_chemicalCompactionRequired;
   bool m_isCalculated;

   /// Other dependencies.
   const CompoundLithologyArray* m_lithologies;

};

class PermeabilityVolumeCalculator {

public :

   PermeabilityVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot );

   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;
   const Interface::Snapshot* m_snapshot;

   /// Property dependencies.
   OutputPropertyMap* m_ves;
   OutputPropertyMap* m_maxVes;
   OutputPropertyMap* m_chemicalCompaction;

   bool m_chemicalCompactionRequired;
   bool m_isCalculated;

   /// Other dependencies.
   const CompoundLithologyArray* m_lithologies;

};


OutputPropertyMap* allocatePermeabilityCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocatePermeabilityVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot );


#endif // _FASTCAULDRON_PERMEABILITY_CALCULATOR_H_
