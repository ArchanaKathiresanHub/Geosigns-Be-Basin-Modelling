//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "CrustalThicknessCalculatorFactory.h"

// mpi 3rd party
#include "mpi.h"

// fastctc application
#include "CrustalThicknessCalculator.h"

// CrustalThickness library
#include "InterfaceInput.h"

// TableIO library
#include "database.h"

using namespace DataAccess;


DataAccess::Interface::ProjectHandle *
CrustalThicknessCalculatorFactory::produceProjectHandle ( database::Database * database, const string & name,
                                                          const string & accessMode) {
   return new CrustalThicknessCalculator ( database, name, accessMode, this );
}

DataAccess::Interface::CrustalThicknessData *
CrustalThicknessCalculatorFactory::produceCrustalThicknessData (DataAccess::Interface::ProjectHandle * projectHandle, 
                                                                database::Record * record)
{
   return new InterfaceInput(projectHandle, record);
}
