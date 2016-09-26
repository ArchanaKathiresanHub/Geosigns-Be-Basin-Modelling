//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Reservoir.h"
#include "Formation.h"
#include "Surface.h"
#include "Migrator.h"
#include "ObjectFactory.h"

#include "database.h"

using namespace migration;

DataAccess::Interface::Reservoir *
ObjectFactory::produceReservoir (DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record)
{
   return produceMigrationReservoir (projectHandle, record);
}

DataAccess::Interface::Formation *
ObjectFactory::produceFormation (DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record)
{
   return produceMigrationFormation (projectHandle, record);
}

DataAccess::Interface::Surface *
ObjectFactory::produceSurface (DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record)
{
   return produceMigrationSurface (projectHandle, record);
}


// Private methods using the Migrator pointer for production

DataAccess::Interface::Reservoir *
ObjectFactory::produceMigrationReservoir (DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record)
{
   return new Reservoir (projectHandle, m_migrator, record);
}

DataAccess::Interface::Formation *
ObjectFactory::produceMigrationFormation (DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record)
{
   return new Formation (projectHandle, m_migrator, record);
}

DataAccess::Interface::Surface *
ObjectFactory::produceMigrationSurface (DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record)
{
   return new Surface (projectHandle, m_migrator, record);
}

