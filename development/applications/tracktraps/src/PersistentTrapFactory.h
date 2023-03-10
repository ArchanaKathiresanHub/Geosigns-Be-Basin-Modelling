//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "ProjectFileHandler.h"
#include "ObjectFactory.h"

namespace DataAccess
{
   namespace Interface
   {
      class ProjectHandle;
      class ObjectFactory;
      class Trap;
      class Reservoir;
   }
}

namespace database
{
   class Record;
   class Database;
}

namespace PersistentTraps
{
   class PersistentTrapFactory : public DataAccess::Interface::ObjectFactory
   {
      virtual DataAccess::Interface::ProjectHandle * produceProjectHandle (database::ProjectFileHandlerPtr database,
                                                                           const std::string & name) const;
      virtual DataAccess::Interface::Trap * produceTrap (DataAccess::Interface::ProjectHandle& projectHandle,
                                                         database::Record * record) const;
      virtual DataAccess::Interface::Reservoir * produceReservoir (DataAccess::Interface::ProjectHandle& projectHandle,
                                                                   database::Record * record) const;
   };
}
