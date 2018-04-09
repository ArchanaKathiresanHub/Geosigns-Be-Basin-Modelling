//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_PERMEABILITY_UPGRADE_MANAGER_H
#define PROGRADE_PERMEABILITY_UPGRADE_MANAGER_H

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
   /// @class PermeabilityUpgradeManager Manager to upgrade a legacy permeability model project (Sandstone or Mudstone model) to use the multipoint permeability model only
   /// @brief Legacy permeability model projects are describe
   class PermeabilityUpgradeManager: public IUpgradeManager {

      public:
         PermeabilityUpgradeManager()                                              = delete;
         PermeabilityUpgradeManager(const PermeabilityUpgradeManager &)            = delete;
         PermeabilityUpgradeManager& operator=(const PermeabilityUpgradeManager &) = delete;

         /// @param[in] model The model on which to perform the permeability model upgrade
         /// throw std::invalid_argument if the model's project handle or curst formation is null
         explicit PermeabilityUpgradeManager( mbapi::Model& model );

         ~PermeabilityUpgradeManager() final = default;

         /// @brief Upgrades the model to use the multipoint permeability model only
         ///    -describe seps
         /// @details If the model is already using multipoint permeability model only, then does nothing
         void upgrade() final;

      private:

         mbapi::Model& m_model; ///< The model to upgrade
         std::shared_ptr<DataAccess::Interface::ProjectHandle> m_ph; ///< The project handle of the model to upgrade
   };
}

#endif