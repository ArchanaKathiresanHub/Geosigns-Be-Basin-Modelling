//
// Copyright (C) 2015-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"

// DataAccess API includes
#include "Snapshot.h"
#include "Reservoir.h"
#include "Trap.h"
#include "Property.h"

// Derived property manager
#include "DerivedPropertyManager.h"
#include "ReservoirProperty.h"

// CBMGenerics
#include "ComponentManager.h"

#include "DataMiningProjectHandle.h"

using namespace CBMGenerics;
using namespace DataAccess;
using namespace Interface;
using namespace Numerics;
using namespace Mining;


namespace DataExtraction
{

class TrapPropertiesManager
{
public:
  explicit TrapPropertiesManager( Mining::ProjectHandle* projectHandle,
                                  DerivedProperties::DerivedPropertyManager& propertyManager );
  virtual ~TrapPropertiesManager() = default;

  double getTrapPropertyValue( const Property  * property,
                               const Snapshot  * snapshot,
                               const Reservoir * reservoir,
                               double x,
                               double y );
private:
  double computeTrapPropertyValue( const Trap      * trap,
                                   const Property  * property,
                                   const Snapshot  * snapshot,
                                   const Reservoir * reservoir,
                                   unsigned int i,
                                   unsigned int j );

  const GridMap* getPropertyGridMap( const Property  * property,
                                     const Snapshot  * snapshot,
                                     const Reservoir * reservoir );

  AbstractDerivedProperties::ReservoirPropertyPtr getReservoirPropertyGridMap( const Property* property,
                                                                               const Snapshot* snapshot,
                                                                               const Reservoir* reservoir );

  bool performPVT( double masses[ComponentManager::NUMBER_OF_SPECIES],
  double temperature,
  double pressure,
  double phaseMasses     [ComponentManager::NUMBER_OF_PHASES][ComponentManager::NUMBER_OF_SPECIES],
  double phaseDensities  [ComponentManager::NUMBER_OF_PHASES],
  double phaseViscosities[ComponentManager::NUMBER_OF_PHASES] );

  double accumulate( double values[], int numberOfValues );
  double computeVolume( double * masses, double density, int numberOfSpecies );

  Mining::ProjectHandle* m_projectHandle;
  DerivedProperties::DerivedPropertyManager& m_propertyManager;
};

} // namespace DataExtraction

