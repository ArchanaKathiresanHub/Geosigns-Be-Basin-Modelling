#include "mpi.h"
#include "CrustalThicknessCalculatorFactory.h"
#include "CrustalThicknessCalculator.h"
#include "InterfaceInput.h"

#include "database.h"

using namespace DataAccess;


DataAccess::Interface::ProjectHandle*
CrustalThicknessCalculatorFactory::produceProjectHandle ( database::ProjectFileHandlerPtr database, const string & name,
                                                          const string & accessMode) {
   return new CrustalThicknessCalculator ( database, name, accessMode, this );
}

DataAccess::Interface::CrustalThicknessData *
CrustalThicknessCalculatorFactory::produceCrustalThicknessData (DataAccess::Interface::ProjectHandle * projectHandle,
                                                                database::Record * record)
{
   return new InterfaceInput(projectHandle, record);
}
