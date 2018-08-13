//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_POROSITY_UPGRADE_MANAGER_H
#define PROGRADE_POROSITY_UPGRADE_MANAGER_H

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
   /// @class PorosityUpgradeManager Manager to upgrade a Soil mechanics compaction model project to use the double exponential compaction model
   class PorosityUpgradeManager: public IUpgradeManager {

      public:
         PorosityUpgradeManager()                                          = delete;
         PorosityUpgradeManager(const PorosityUpgradeManager &)            = delete;
         PorosityUpgradeManager& operator=(const PorosityUpgradeManager &) = delete;

         /// @param[in] model The model on which to perform the porosity model upgrade
         /// throw std::invalid_argument if the model's project handle or curst formation is null
         explicit PorosityUpgradeManager( mbapi::Model& model );

         ~PorosityUpgradeManager() final = default;

         /// @brief Upgrades the deprecated Soil mechanics model to use the double exponential model 
         ///    -describe seps
         /// @details If the model is using exponential/double exponential model, then does nothing
         void upgrade() final;

      private:

         mbapi::Model& m_model; ///< The model to upgrade
         std::shared_ptr<DataAccess::Interface::ProjectHandle> m_ph; ///< The project handle of the model to upgrade
   };
}

#endif