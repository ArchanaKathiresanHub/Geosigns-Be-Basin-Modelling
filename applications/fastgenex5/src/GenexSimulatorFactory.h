#ifndef _GENEXSIMULATION_GENEXSIMULATORFACTORY_H_
#define _GENEXSIMULATION_GENEXSIMULATORFACTORY_H_

#include "Interface/ObjectFactory.h"

namespace DataAccess
{
   namespace Interface
   {
      class ObjectFactory;
      class ProjectHandle;  
      class SourceRock;
   }
}

namespace database
{
   class Record;
   class Database;
}

namespace GenexSimulation
{

class GenexSimulatorFactory : public DataAccess::Interface::ObjectFactory
{
public:
   /// Produce the fastgenex5 specific ProjectHandle
   virtual DataAccess::Interface::ProjectHandle * produceProjectHandle (database::Database * database, const string & name, const string & accessMode);

   /// Produce the fastgenex5 specific SourceRock
   virtual DataAccess::Interface::SourceRock * produceSourceRock (DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record);
	                                                                   
};

}

#endif



