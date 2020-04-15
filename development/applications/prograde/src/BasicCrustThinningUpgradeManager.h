//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_BOTTOM_BOUNDARY_UPGRADE_MANAGER_H
#define PROGRADE_BOTTOM_BOUNDARY_UPGRADE_MANAGER_H

//Prograde
#include "IUpgradeManager.h"
#include "ErrorHandler.h"
//std
#include <memory>

namespace mbapi {
   class Model;
   class MapsManager;
}

namespace DataAccess {
   namespace Interface {
      class ProjectHandle;
   }
}

namespace Prograde
{
   /// @class BasicCrustThinningUpgradeManager Manager to upgrade a basic crustal thinning model project to use the new crustal thinning model
   class BasicCrustThinningUpgradeManager : public IUpgradeManager {

   public:
      BasicCrustThinningUpgradeManager() = delete;
      BasicCrustThinningUpgradeManager(const BasicCrustThinningUpgradeManager &) = delete;
      BasicCrustThinningUpgradeManager& operator=(const BasicCrustThinningUpgradeManager &) = delete;

      /// @param[in] model The model on which to perform the basic crustal thinning model upgrade
      /// throw std::invalid_argument if the model's project handle or curst formation is null
      explicit BasicCrustThinningUpgradeManager(mbapi::Model& model);

      ~BasicCrustThinningUpgradeManager() final = default;

      /// @brief Upgrades the basic crustal thinning model to use the new crustal thinning model 
      /// @details If the model is using advanced crust thinning/heat flow model, then does nothing
      void upgrade() final;

   private:
      /// @brief Detects if the project is using the basic crust thinning model for bottom boundary condition 
      /// @return True if the project is using basic crust thinning model, false otherwise
      //bool isBasicCrustThinning() const;

      /// @brief Clean the CrustIoTbl
      void cleanCrustIoTbl() const;

      /// @brief Clean the Continental Crustal thickness IoTbl
      void cleanContCrustIoTbl() const;
      /// @brief Clean the BasaltThicknessIoTbl
      void cleanBasaltThicknessIoTbl() const;

      /// @brief Clean the MntlHeatFlowIoTbl 
      
      /// @brief Interpolate at a given age
      /// @Inputs
      /// @return 
      ErrorHandler::ReturnCode InterpolateIntermidiateValues(size_t mapIdToSave, size_t mapIdmin, size_t mapId, double* ageValuePoints, mbapi::MapsManager* mngr=nullptr);
      
      void cleanMntlHeatFlowIoTbl() const;
      mbapi::Model& m_model; ///< The model to upgrade
      std::shared_ptr<DataAccess::Interface::ProjectHandle> m_ph; ///< The project handle of the model to upgrade
   };
}

#endif