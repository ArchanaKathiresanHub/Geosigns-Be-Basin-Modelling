//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_RESERVOIR_UPGRADE_MANAGER_H
#define PROGRADE_RESERVOIR_UPGRADE_MANAGER_H

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
   /// @class ReservoirUpgradeManager Manager to upgrade the BPA1 reservokirIoTbl to BPA2 ReservoirIoTbl and ReservoirOptionsIOTbl
   /// @brief Legacy(BPA1) projects use layerwise parameters whereas BPA2 uses globle parameters.
   class ReservoirUpgradeManager : public IUpgradeManager {

   public:
      ReservoirUpgradeManager() = delete;
      ReservoirUpgradeManager(const ReservoirUpgradeManager &) = delete;
      ReservoirUpgradeManager& operator=(const ReservoirUpgradeManager &) = delete;

      /// @param[in] model The model on which to perform the ReservoirIoTbl and ReservoirOptionsIoTbl upgrade
      /// throw std::invalid_argument if the model's project handle is null
      explicit ReservoirUpgradeManager(mbapi::Model& model);

      ~ReservoirUpgradeManager() final = default;

      /// @brief Upgrades the model to use globle field values for all reservoirs in ReservoirIoTbl and create ReservoirOptionsIoTbl
      void upgrade() final;


   private:

      mbapi::Model& m_model; ///< The model to upgrade
      std::shared_ptr<DataAccess::Interface::ProjectHandle> m_ph; ///< The project handle of the model to upgrade
   };
}

#endif
