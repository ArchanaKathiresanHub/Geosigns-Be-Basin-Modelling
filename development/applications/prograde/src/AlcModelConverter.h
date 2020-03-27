//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_ALC_MODEL_CONVERTER_H
#define PROGRADE_ALC_MODEL_CONVERTER_H

//cmbAPI
#include "BottomBoundaryManager.h"

namespace Prograde
{
   /// @class AlcModelConverter Converts bottom boundary models as well as crust/mantle property model
   class  AlcModelConverter {

   public:

      AlcModelConverter() = default;
      AlcModelConverter(const AlcModelConverter &) = delete;
      AlcModelConverter& operator=(const AlcModelConverter &) = delete;
      ~AlcModelConverter() = default;

      /// @brief Upgrades the crust property models
      /// @details The crust property models are upgraded to standard conductivity crust model
      mbapi::BottomBoundaryManager::CrustPropertyModel upgradeAlcCrustPropModel(const mbapi::BottomBoundaryManager::CrustPropertyModel) const;

      /// @brief Upgrades the mantle property models
      /// @details The mantle property models are upgraded to high conductivity mantle model
      mbapi::BottomBoundaryManager::MantlePropertyModel upgradeAlcMantlePropModel(const mbapi::BottomBoundaryManager::MantlePropertyModel) const;

	  ///@brief Upgrades the BottomBoundaryModel column of BasementIoTbl
	  ///@details Sets the value to Improved Lithosphere Calculator Linear Element Mode if it is Advanced Lithosphere Calculator
	  std::string updateBottomBoundaryModel(std::string &);

	  ///@brief Sets the limits on TopCrustHeatProd column of BasementIoTbl as [0-1000]
	  ///@details If TopCrustHeatProd column of BasementIoTbl has values crossing the set limits for any record, the values are changed and set to the nearest extremes
	  double updateTopCrustHeatProd(double);

	  ///@brief Sets the limits on InitialLithosphericMantleThickness column of BasementIoTbl as [0- 6300000]
	  ///@details If InitialLithosphericMantleThickness column of BasementIoTbl has values crossing the set limits for any record, the values are changed and set to the nearest extremes
	  double updateInitialLithosphericMantleThickness(double);


   };
}

#endif


