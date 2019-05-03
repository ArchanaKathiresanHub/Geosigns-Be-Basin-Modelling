#ifndef _GENEXSIMULATION_GENEXSIMULATORFACTORY_H_
#define _GENEXSIMULATION_GENEXSIMULATORFACTORY_H_

#include "ProjectFileHandler.h"

#include "GeoPhysicsObjectFactory.h"

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

class GenexSimulatorFactory : public GeoPhysics::ObjectFactory
{
public:
   /// Produce the fastgenex6 specific ProjectHandle
   virtual GeoPhysics::ProjectHandle * produceProjectHandle (database::ProjectFileHandlerPtr database,
                                                             const string & name,
                                                             const string & accessMode) const;

   /// Produce the fastgenex6 specific SourceRock
   virtual DataAccess::Interface::SourceRock * produceSourceRock (DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record) const;

};

}

#endif
