//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef FASTMIG__OBJECT_FACTORY_H
#define FASTMIG__OBJECT_FACTORY_H

#include "GeoPhysicsObjectFactory.h"

namespace DataAccess
{
   namespace Interface
   {
      class ProjectHandle;
      class ObjectFactory;
      class Trap;
      class Reservoir;
      class MigrationFormation;
      class Surface;
   }
}

namespace database
{
   class Record;
   class Database;
}

namespace migration
{
   class Migrator;

   class ObjectFactory : public GeoPhysics::ObjectFactory
   {
   public:

      ObjectFactory (Migrator * migrator) :
         m_migrator (migrator)
      {
      }

      virtual DataAccess::Interface::Reservoir * produceReservoir (DataAccess::Interface::ProjectHandle& projectHandle,
         database::Record * record) const;

      virtual DataAccess::Interface::Formation * produceFormation (DataAccess::Interface::ProjectHandle& projectHandle,
         database::Record * record) const;

      virtual DataAccess::Interface::Surface * produceSurface (DataAccess::Interface::ProjectHandle& projectHandle,
         database::Record * record) const;

   private:

      DataAccess::Interface::Reservoir * produceMigrationReservoir (DataAccess::Interface::ProjectHandle& projectHandle,
         database::Record * record) const;

      DataAccess::Interface::Formation * produceMigrationFormation (DataAccess::Interface::ProjectHandle& projectHandle,
         database::Record * record) const;

      DataAccess::Interface::Surface * produceMigrationSurface (DataAccess::Interface::ProjectHandle& projectHandle,
         database::Record * record) const;

      Migrator * const m_migrator;
   };
}




#endif // FASTMIG__OBJECT_FACTORY_H
