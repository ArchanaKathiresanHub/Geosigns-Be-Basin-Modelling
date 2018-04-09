//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "UpgradeManagers.h"

//std
#include <vector>
#include <algorithm>
#include <memory>

//Prograde
#include "AlcUpgradeManager.h"
#include "PermeabilityUpgradeManager.h"

//Prograde
#include "IUpgradeManager.h"

//utilities
#include "LogHandler.h"

void Prograde::UpgradeManagers::runAll() const{
   ///1. Create list of upgrade managers
   std::vector<std::shared_ptr<Prograde::IUpgradeManager>> managers;
   // ALCv1 to ALCv2
   managers.emplace_back(std::unique_ptr<Prograde::AlcUpgradeManager>(new Prograde::AlcUpgradeManager(m_model)));
   // Mudstone and Sandstone permeability models to Multipoint permeability model
   managers.emplace_back(std::unique_ptr<Prograde::PermeabilityUpgradeManager>(new Prograde::PermeabilityUpgradeManager(m_model)));
   // Other managers to be added in the same way

   ///2. Run all upgrade managers
   std::for_each(managers.begin(), managers.end(), [] ( std::shared_ptr<Prograde::IUpgradeManager> manager)
   {
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP ) << "running " + manager->getName();
      manager->upgrade();
   });
}