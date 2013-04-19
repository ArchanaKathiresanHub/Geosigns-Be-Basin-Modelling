#include "Interface/ObjectFactory.h"

namespace DataAccess
{
   namespace Interface
   {
      class ProjectHandle;
      class ObjectFactory;
   }
}

namespace database
{
   class Record;
   class Database;
}

namespace fasttouch
{

   class ObjectFactory : public DataAccess::Interface::ObjectFactory
   {
      virtual DataAccess::Interface::ProjectHandle * produceProjectHandle (database::Database * database,
	    const string & name, const string & accessMode);
   };
}



