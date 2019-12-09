//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "database.h"

#include "ObjectFactory.h"

#include "PersistentTrapFactory.h"
#include "TrackTrap.h"
#include "TrackReservoir.h"
#include "TrackProjectHandle.h"

using namespace PersistentTraps;

DataAccess::Interface::ProjectHandle *
PersistentTrapFactory::produceProjectHandle (database::ProjectFileHandlerPtr database,
                                             const string & name) const
{
   return new TrackProjectHandle (database, name, this);
}

DataAccess::Interface::Trap *
PersistentTrapFactory::produceTrap (DataAccess::Interface::ProjectHandle& projectHandle, database::Record * record) const
{
   return new PersistentTraps::TrackTrap (projectHandle, record);
}

DataAccess::Interface::Reservoir *
PersistentTrapFactory::produceReservoir (DataAccess::Interface::ProjectHandle& projectHandle, database::Record * record) const
{
   return new TrackReservoir (projectHandle, record);
}
