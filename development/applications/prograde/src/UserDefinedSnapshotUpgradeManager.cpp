//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "UserDefinedSnapshotUpgradeManager.h"

//utilities
#include "LogHandler.h"

//cmbAPI
#include "cmbAPI.h"

#include <list>

using namespace mbapi;

//------------------------------------------------------------//

Prograde::UserDefinedSnapshotUpgradeManager::UserDefinedSnapshotUpgradeManager(Model& model) :
  IUpgradeManager("User defined snapshot upgrade manager"), m_model(model)
{
  const auto ph = m_model.projectHandle();
  if (ph == nullptr)
  {
    throw std::invalid_argument(getName() + " cannot retrieve the project handle from Cauldron data model");
  }
  m_ph = ph;
}

//------------------------------------------------------------//

void Prograde::UserDefinedSnapshotUpgradeManager::upgrade()
{
  std::list<double> userDefinedAges;

  database::Table* userDefinedSnapshotIoTable = m_ph->getTable("UserDefinedSnapshotIoTbl");
  assert(userDefinedSnapshotIoTable);

  for ( database::Record* record : *userDefinedSnapshotIoTable )
  {
    userDefinedAges.push_back( record->getValue<double>("Time") );
  }
  userDefinedSnapshotIoTable->clear();

  database::Table* snapshotIoTable = m_ph->getTable("SnapshotIoTbl");
  assert(snapshotIoTable);

  for ( database::Record* record : *snapshotIoTable )
  {
    if ( record->getValue<std::string>("TypeOfSnapshot") == "User Defined" )
    {
      userDefinedAges.push_back( record->getValue<double>("Time") );
    }
  }

  userDefinedAges.sort();
  userDefinedAges.unique( DataAccess::Interface::ProjectHandle::isEqualTime );

  for ( const double userDefinedAge : userDefinedAges )
  {
    database::Record* record = userDefinedSnapshotIoTable->createRecord();
    record->setValue<double>( "Time", userDefinedAge );
    LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> User defined snapshot age " << userDefinedAge << " is copied from the SnapshotIoTbl to the UserDefinedSnapshotIoTbl";
  }
}
