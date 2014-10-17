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

   class ObjectFactory : public GeoPhysics::ObjectFactory
   {

      virtual DataAccess::Interface::ProjectHandle * produceProjectHandle (database::Database * database,
                                                                           const string & name, const string & accessMode);

      virtual DataAccess::Interface::Reservoir * produceReservoir (DataAccess::Interface::ProjectHandle * projectHandle,
                                                                   database::Record * record);

      virtual DataAccess::Interface::Formation * produceFormation (DataAccess::Interface::ProjectHandle * projectHandle,
                                                                   database::Record * record);

      virtual DataAccess::Interface::Surface * produceSurface (DataAccess::Interface::ProjectHandle * projectHandle,
                                                               database::Record * record);
   };
}




#endif // FASTMIG__OBJECT_FACTORY_H
