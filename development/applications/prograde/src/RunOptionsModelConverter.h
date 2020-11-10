//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef RUN_OPTIONS_CONVERTER_H
#define RUN_OPTIONS_CONVERTER_H

//cmbAPI
#include "RunOptionsManager.h"

namespace Prograde
{
   /// @class RunOptionsConverter Converts BPA1  run options into BPA2 run options
   class RunOptionsModelConverter {

   public:

      RunOptionsModelConverter() = default;
      RunOptionsModelConverter(const RunOptionsModelConverter &) = delete;
      RunOptionsModelConverter& operator=(const RunOptionsModelConverter &) = delete;
      ~RunOptionsModelConverter() = default;

      /// @brief Upgrades the corresponding RunOptionsIoTbl   

	  // @breif upgrades "Gardner's Velocity-Density" and "Wyllie's Time-Average" to "Kennan's Velocity-Porosity" model
	  std::string upgradeVelAlgorithm(std::string velAlgorithm);

	  /// @breif upgrade vre algorithm to Burnham & Sweeney
	  std::string upgradeVreAlgorithm(std::string vreAlgorithm);

	  /// @breif upgrades LooselyCoupled mode to ItCoupled mode and updates the relevant flag
	  // NOTE: variables are set by reference here
	  void upgradePTCouplingMode(std::string& PTCouplingMode, int& coupledMode);

	  /// @breif upgrades chemical compaction model to from Schneider to Walderhaug
	  std::string upgradeChemicalCompactionAlgorithm(std::string chemCompactionAlgo);

	  /// @breif upgrades optimisation Levels 1,2,3 to Level 4
	  std::string upgradeOptimisationLevel(std::string optimisationLevel);

	  /// @breif temperature gradient ranges are between [0, 1e5] Celcius/Km
	  // UNIT : Celcius/Km
	  double upgradeTemperatureRange(double temperature, std::string fieldName);

	  /// @breif pressure ranges are between [0, 1e5] MPa
	  // UNIT : MPa (Mega Pascal)
	  double upgradePressureRange(double pressure, std::string fieldName);

	  /// @breif updates the Legacy column in RunOptionsIoTbl
	  // Updated from 1 to 0
	  int upgradeLegacyFlag(int legacy);
   };
}

#endif
