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
      class Formation;
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

      ObjectFactory(Migrator * migrator):
         m_migrator(migrator)
      {}

         virtual DataAccess::Interface::Reservoir * produceReservoir (DataAccess::Interface::ProjectHandle * projectHandle,
                                                                      database::Record * record);

         virtual DataAccess::Interface::Formation * produceFormation (DataAccess::Interface::ProjectHandle * projectHandle,
                                                                      database::Record * record);

         virtual DataAccess::Interface::Surface * produceSurface (DataAccess::Interface::ProjectHandle * projectHandle,
                                                                  database::Record * record);

   private:

         DataAccess::Interface::Reservoir * produceMigrationReservoir (DataAccess::Interface::ProjectHandle * projectHandle,
                                                                       database::Record * record);

         DataAccess::Interface::Formation * produceMigrationFormation (DataAccess::Interface::ProjectHandle * projectHandle,
                                                                       database::Record * record);

         DataAccess::Interface::Surface * produceMigrationSurface (DataAccess::Interface::ProjectHandle * projectHandle,
                                                                   database::Record * record);

         Migrator * const m_migrator;
   };
}




#endif // FASTMIG__OBJECT_FACTORY_H
