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
//#include "IUpgradeManager.h"
#include "BottomBoundaryModelUpgradeManager.h"
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
   class BasicCrustThinningUpgradeManager : public BottomBoundaryModelUpgradeManager {

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

      /// @brief Clean the CrustIoTbl
      void cleanCrustIoTbl() const;

      /// @brief Clean the Continental Crustal thickness IoTbl
      void cleanContCrustIoTbl() const;
      /// @brief Clean the BasaltThicknessIoTbl
      void cleanBasaltThicknessIoTbl() const;

      /// @brief Clean the MntlHeatFlowIoTbl 
      void cleanMntlHeatFlowIoTbl() const;
	  //@brief Generate a new map at basinAge by lineraly interpolating the values defined at interpolatingLowerAge and interpolatingHigherAge 
	  DataAccess::Interface::GridMap* generateInterpolatedMapAtAge(std::string bottomBoundaryModel, bool needInterpolation, const double basinAge, double& interpolatingLowerAge, double& interpolatingHigherAge);
      //mbapi::Model& m_model; ///< The model to upgrade
      //std::shared_ptr<DataAccess::Interface::ProjectHandle> m_ph; ///< The project handle of the model to upgrade
   };
}

#endif