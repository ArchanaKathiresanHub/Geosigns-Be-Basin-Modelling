//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_BASIC_CRUST_THINNING_MODEL_CONVERTER_H
#define PROGRADE_BASIC_CRUST_THINNING_MODEL_CONVERTER_H

//cmbAPI
#include "BottomBoundaryManager.h"

namespace Prograde
{
   /// @class BasicCrustThinningModelConverter Converts bottom boundary models as well as crust/mantle property model
   class  BasicCrustThinningModelConverter {

   public:

      BasicCrustThinningModelConverter() = default;
      BasicCrustThinningModelConverter(const BasicCrustThinningModelConverter &) = delete;
      BasicCrustThinningModelConverter& operator=(const BasicCrustThinningModelConverter &) = delete;
      ~BasicCrustThinningModelConverter() = default;


      /// @brief Upgrades the bottom boundary models
      /// @details The basic crustal thinning model are upgraded to new crustal thinning model
      mbapi::BottomBoundaryManager::BottomBoundaryModel upgradeBotBoundModel(const mbapi::BottomBoundaryManager::BottomBoundaryModel);

      /// @brief Upgrades the crust property models
      /// @details The crust property models are upgraded to standard conductivity crust model
      mbapi::BottomBoundaryManager::CrustPropertyModel upgradeCrustPropModel(const mbapi::BottomBoundaryManager::CrustPropertyModel) const;

      /// @brief Upgrades the mantle property models
      /// @details The mantle property models are upgraded to high conductivity mantle model
      mbapi::BottomBoundaryManager::MantlePropertyModel upgradeMantlePropModel(const mbapi::BottomBoundaryManager::MantlePropertyModel) const;

      /// @brief Upgrades the grid map Io table
      /// @details Change the "CrustIoTbl" of GridMapIoTbl to "ContCrustalThicknessIoTbl"
      std::string upgradeGridMapTable(const std::string &);


   };
}

#endif

 