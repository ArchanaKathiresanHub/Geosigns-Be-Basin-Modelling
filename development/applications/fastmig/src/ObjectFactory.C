#include "Reservoir.h"
#include "Formation.h"
#include "Surface.h"
#include "Migrator.h"
#include "ObjectFactory.h"

#include "database.h"

using namespace migration;

DataAccess::Interface::ProjectHandle *
ObjectFactory::produceProjectHandle (database::Database * database,
	 const string & name, const string & accessMode)
{
   return new Migrator (database, name, accessMode, this);
}

DataAccess::Interface::Reservoir *
ObjectFactory::produceReservoir (DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record)
{
   return new Reservoir (projectHandle, record);
}

DataAccess::Interface::Formation *
ObjectFactory::produceFormation (DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record)
{
   return new Formation (projectHandle, record);
}

DataAccess::Interface::Surface *
ObjectFactory::produceSurface (DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record)
{
   return new Surface (projectHandle, record);
}


