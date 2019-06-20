//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "MigrationReservoir.h"
#include "Formation.h"
#include "MigrationSurface.h"
#include "Migrator.h"
#include "ObjectFactory.h"

#include "database.h"

using namespace migration;

DataAccess::Interface::Reservoir *
ObjectFactory::produceReservoir (DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record) const
{
   return produceMigrationReservoir (projectHandle, record);
}

DataAccess::Interface::Formation *
ObjectFactory::produceFormation (DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record) const
{
   return produceMigrationFormation (projectHandle, record);
}

DataAccess::Interface::Surface *
ObjectFactory::produceSurface (DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record) const
{
   return produceMigrationSurface (projectHandle, record);
}


// Private methods using the Migrator pointer for production

DataAccess::Interface::Reservoir *
ObjectFactory::produceMigrationReservoir (DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record) const
{
   return new MigrationReservoir (projectHandle, m_migrator, record);
}

DataAccess::Interface::Formation *
ObjectFactory::produceMigrationFormation (DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record) const
{
   return new MigrationFormation (projectHandle, m_migrator, record);
}

DataAccess::Interface::Surface *
ObjectFactory::produceMigrationSurface (DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record) const
{
   return new MigrationSurface (projectHandle, m_migrator, record);
}

