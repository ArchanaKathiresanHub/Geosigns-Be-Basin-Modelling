#include <mpi.h>

#include "SourceRock.h"
#include "GenexSimulator.h"
#include "GenexSimulatorFactory.h"

#include "database.h"

#include <iostream>
using namespace std;

using namespace GenexSimulation;

GeoPhysics::ProjectHandle *
GenexSimulatorFactory::produceProjectHandle (database::ProjectFileHandlerPtr database, const string & name,
                                             const string & accessMode) const
{
   return new GenexSimulator (database, name, accessMode, this);
}

DataAccess::Interface::SourceRock *
GenexSimulatorFactory::produceSourceRock (DataAccess::Interface::ProjectHandle * projectHandle,
                                          database::Record * record) const
{
   return new Genex6::SourceRock(projectHandle, record);
}
