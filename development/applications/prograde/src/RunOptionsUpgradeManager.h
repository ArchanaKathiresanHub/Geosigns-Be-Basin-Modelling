//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_RUN_OPTIONS_UPGRADE_MANAGER_H
#define PROGRADE_RUN_OPTIONS_UPGRADE_MANAGER_H

//Prograde
#include "IUpgradeManager.h"

//std
#include <memory>

namespace mbapi {
   class Model;
}

namespace DataAccess {
   namespace Interface {
      class ProjectHandle;
   }
}

namespace Prograde
{
   /// @class RunOptionsUpgradeManager Manager to upgrade the BPA1 options to BPA2 options
   class RunOptionsUpgradeManager : public IUpgradeManager {

   public:
      RunOptionsUpgradeManager() = delete;
      RunOptionsUpgradeManager(const RunOptionsUpgradeManager &) = delete;
      RunOptionsUpgradeManager& operator=(const RunOptionsUpgradeManager &) = delete;

      /// @param[in] model The model on which to perform the RunOptionsIoTbl upgrade
      /// throw std::invalid_argument if the model's project handle is null
      explicit RunOptionsUpgradeManager(mbapi::Model& model);

      ~RunOptionsUpgradeManager() final = default;

      /// @brief Upgrades the model to use BPA2 values for RunOptionsIoTbl items
      void upgrade() final;


   private:

      mbapi::Model& m_model; ///< The model to upgrade
      std::shared_ptr<DataAccess::Interface::ProjectHandle> m_ph; ///< The project handle of the model to upgrade
   };
}

#endif
