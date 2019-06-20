//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef FASTCAULDRON__REQUIRED_GENEX5_PROPERTY_CALCULATOR_H
#define FASTCAULDRON__REQUIRED_GENEX5_PROPERTY_CALCULATOR_H

#include "GeoPhysicsSourceRock.h"

// CBMGenerics library
#include "ComponentManager.h"

#include "OutputPropertyMap.h"

#include "Formation.h"
#include "Surface.h"
#include "Snapshot.h"


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

   bool m_hasSpeciesMap [ CBMGenerics::ComponentManager::NUMBER_OF_SPECIES ];

   bool m_isCalculated;

};


OutputPropertyMap* allocateRequiredGenex5PropertyCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

#endif // FASTCAULDRON__REQUIRED_GENEX5_PROPERTY_CALCULATOR_H
