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


CrustalThicknessCalculator* CrustalThicknessCalculatorFactory::produceProjectHandle ( database::ProjectFileHandlerPtr database,
                                                                                      const string & name,
                                                                                      const string & accessMode) const
{
   return new CrustalThicknessCalculator ( database, name, accessMode, this );
}

Ctc::CTCPropertyValue * CrustalThicknessCalculatorFactory::producePropertyValue( Interface::ProjectHandle *   projectHandle,
                                                                              database::Record *           record,
                                                                              const string &               name,
                                                                              const Interface::Property *  property,
                                                                              const Interface::Snapshot *  snapshot,
                                                                              const Interface::Reservoir * reservoir,
                                                                              const Interface::Formation * formation,
                                                                              const Interface::Surface *   surface,
                                                                              Interface::PropertyStorage   storage,
                                                                              const std::string &          /*fileName*/) const
{
   return new Ctc::CTCPropertyValue( projectHandle, record, name, property, snapshot, reservoir, formation, surface, storage );
}
