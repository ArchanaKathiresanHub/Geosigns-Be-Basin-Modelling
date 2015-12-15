#include "database.h"

#include "Interface/ObjectFactory.h"

#include "PersistentTrapFactory.h"
#include "Trap.h"
#include "Reservoir.h"
#include "ProjectHandle.h"

using namespace PersistentTraps;

DataAccess::Interface::ProjectHandle *
PersistentTrapFactory::produceProjectHandle (database::Database * database,
	 const string & name, const string & accessMode)
{
   return new ProjectHandle (database, name, accessMode, this);
}

DataAccess::Interface::Trap *
PersistentTrapFactory::produceTrap (DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record)
{
   return new PersistentTraps::Trap (projectHandle, record);
}

DataAccess::Interface::Reservoir *
PersistentTrapFactory::produceReservoir (DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record)
{
   return new Reservoir (projectHandle, record);
}


