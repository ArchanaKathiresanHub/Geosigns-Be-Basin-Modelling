//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Genex0dSimulatorFactory.h"

#include "Genex0dSimulator.h"

#include "database.h"

namespace genex0d
{

GeoPhysics::ProjectHandle * Genex0dSimulatorFactory::produceProjectHandle (database::ProjectFileHandlerPtr database,
                                                                           const std::string & name,
                                                                           const std::string & accessMode) const
{
  return new Genex0dSimulator(database, name, accessMode, this);
}

} // namespace genex0d
