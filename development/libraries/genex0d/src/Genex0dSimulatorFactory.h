//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Genex0dSimulatorFactory class:

#pragma once

#include "GeoPhysicsObjectFactory.h"

namespace DataAccess
{
namespace Interface
{
class ObjectFactory;
class ProjectHandle;
} // namespace Interface
} // namespace DataAccess

namespace database
{
class Record;
class Database;
} // namespace database

namespace Genex0d
{

class Genex0dSimulatorFactory : public GeoPhysics::ObjectFactory
{
public:
  GeoPhysics::ProjectHandle * produceProjectHandle (database::ProjectFileHandlerPtr database,
                                                    const std::string & name) const final;
};

} // namespace genex0d
