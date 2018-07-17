//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_BIODEGARDE_UPGRADE_MANAGER_H
#define PROGRADE_BIODEGARDE_UPGRADE_MANAGER_H

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
   /// @class 
   /// @brief 
   ///        
   class BiodegradeUpgradeManager : public IUpgradeManager {

   public:

      BiodegradeUpgradeManager() = delete;
      BiodegradeUpgradeManager(const BiodegradeUpgradeManager &) = delete;
      BiodegradeUpgradeManager& operator=(const BiodegradeUpgradeManager &) = delete;

      /// @param[in] model The model on which to perform the biodegradation upgrade
      /// throw std::invalid_argument if the model's project handle is null
      explicit BiodegradeUpgradeManager(mbapi::Model& model);

      ~BiodegradeUpgradeManager() final = default;

      /// @brief upgrade the biodegradation factors in project3d file (BioDegradIoTbl)
      /// @details check biodegradation factors in BioDegradIoTbl with default value and also range 
      /// @details if biodegradation factors deviate from default/range then update biodegradation factors
      void upgrade() final;


   private:

      mbapi::Model& m_model; ///< The model to upgrade
      std::shared_ptr<DataAccess::Interface::ProjectHandle> m_ph; ///< The project handle of the model to upgrade
   };
}

#endif
