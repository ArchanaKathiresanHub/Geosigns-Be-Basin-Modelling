//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_CTC_UPGRADE_MANAGER_H
#define PROGRADE_CTC_UPGRADE_MANAGER_H

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
   /// @class CtcUpgradeManager Manager to upgrade a basic crustal thinning model project to use the new crustal thinning model
   class CtcUpgradeManager : public IUpgradeManager {

   public:
      CtcUpgradeManager() = delete;
      CtcUpgradeManager(const CtcUpgradeManager &) = delete;
      CtcUpgradeManager& operator=(const CtcUpgradeManager &) = delete;

      /// @param[in] model The model on which to perform the basic crustal thinning model upgrade
      /// throw std::invalid_argument if the model's project handle or curst formation is null
      explicit CtcUpgradeManager(mbapi::Model& model);

      ~CtcUpgradeManager() final = default;

      /// @brief Upgrades the basic crustal thinning model to use the new crustal thinning model 
      /// @details If the model is using advanced crust thinning/heat flow model, then does nothing
      void upgrade() final;

   private:
     
      mbapi::Model& m_model; ///< The model to upgrade
      std::shared_ptr<DataAccess::Interface::ProjectHandle> m_ph; ///< The project handle of the model to upgrade
   };
}

#endif
