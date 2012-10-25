#ifndef FASTREQ_OBJECTFACTORY_H_
#define FASTREQ_OBJECTFACTORY_H_

#include "Interface/ObjectFactory.h"

namespace DataAccess
{
   namespace Interface
   {
      class ObjectFactory;
      class ProjectHandle;  
   }
}

namespace database
{
   class Record;
   class Database;
}

namespace FastReq
{

class FastReqObjectFactory : public DataAccess::Interface::ObjectFactory
{
public:
   virtual DataAccess::Interface::ProjectHandle * produceProjectHandle ( database::Database * database,
	                                                                           const string & name, 
                                                                              const string & accessMode);

};
}

#endif
