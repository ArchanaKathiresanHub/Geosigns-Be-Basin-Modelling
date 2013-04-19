#ifndef _FASTCAULDRON__REQUIRED_GENEX5_PROPERTY_CALCULATOR_H_
#define _FASTCAULDRON__REQUIRED_GENEX5_PROPERTY_CALCULATOR_H_

#include "GeoPhysicsSourceRock.h"
#include "ComponentManager.h"

#include "OutputPropertyMap.h"

#include "Interface/Formation.h"
#include "Interface/Surface.h"
#include "Interface/Snapshot.h"


class RequiredGenex5PropertyCalculator {


public :

   RequiredGenex5PropertyCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;
   const Interface::Surface* m_surface;
   const Interface::Snapshot* m_snapshot;

   const GeoPhysics::GeoPhysicsSourceRock* m_sourceRock;

   bool m_hasSpeciesMap [ CBMGenerics::ComponentManager::NumberOfSpecies ];

   bool m_isCalculated;

};


OutputPropertyMap* allocateRequiredGenex5PropertyCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

#endif // _FASTCAULDRON__REQUIRED_GENEX5_PROPERTY_CALCULATOR_H_
