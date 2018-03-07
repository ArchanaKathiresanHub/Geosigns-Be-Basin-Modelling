//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ReservoirProperty.h"

AbstractDerivedProperties::ReservoirProperty::ReservoirProperty ( const DataModel::AbstractProperty*  property,
                                                                  const DataModel::AbstractSnapshot*  snapshot,
                                                                  const DataModel::AbstractReservoir* reservoir,
                                                                  const DataModel::AbstractGrid*      grid ) :
   FormationMapProperty ( property, snapshot, reservoir->getFormation (), grid ),
   m_reservoir ( reservoir )
{
   // Should check that they aint null.
}
