#include "Interface/ObjectFactory.h"

namespace DataAccess
{
   namespace Interface
   {
      class ProjectHandle;
      class ObjectFactory;
      class Trap;
      class Reservoir;
   }
}

namespace database
{
   class Record;
   class Database;
}

namespace PersistentTraps
{
   class PersistentTrapFactory : public DataAccess::Interface::ObjectFactory
   {
      virtual DataAccess::Interface::ProjectHandle * produceProjectHandle (database::Database * database,
	    const string & name, const string & accessMode);
      virtual DataAccess::Interface::Trap * produceTrap (DataAccess::Interface::ProjectHandle * projectHandle,
	    database::Record * record);
      virtual DataAccess::Interface::Reservoir * produceReservoir (DataAccess::Interface::ProjectHandle * projectHandle,
	    database::Record * record);
   };
}

