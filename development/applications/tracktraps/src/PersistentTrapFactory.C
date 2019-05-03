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

#include "Interface/ObjectFactory.h"

#include "PersistentTrapFactory.h"
#include "Trap.h"
#include "Reservoir.h"
#include "ProjectHandle.h"

using namespace PersistentTraps;

DataAccess::Interface::ProjectHandle *
PersistentTrapFactory::produceProjectHandle (database::ProjectFileHandlerPtr database,
                                             const string & name, const string & accessMode) const
{
   return new ProjectHandle (database, name, accessMode, this);
}

DataAccess::Interface::Trap *
PersistentTrapFactory::produceTrap (DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record) const
{
   return new PersistentTraps::Trap (projectHandle, record);
}

DataAccess::Interface::Reservoir *
PersistentTrapFactory::produceReservoir (DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record) const
{
   return new Reservoir (projectHandle, record);
}
