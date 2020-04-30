//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_HEAT_FLOW_MODE_UPGRADE_MANAGER_H
#define PROGRADE_HEAT_FLOW_MODE_UPGRADE_MANAGER_H

#include "BottomBoundaryModelUpgradeManager.h"

namespace Prograde
{
	/// @class HeatFlowModeUpgradeManager Manager to upgrade the legacy HeatFlow bottom boundary model related IoTbls
	/// @brief Legacy projects having HeatFlow bottom boundary model is upgraded as per the BPA2 format.
	
	class HeatFlowModeUpgradeManager : public BottomBoundaryModelUpgradeManager {

	public:
		HeatFlowModeUpgradeManager() = delete;
		HeatFlowModeUpgradeManager(const HeatFlowModeUpgradeManager &) = delete;
		HeatFlowModeUpgradeManager& operator=(const HeatFlowModeUpgradeManager &) = delete;

		/// @param[in] model The model on which to perform the HeatFlow model upgrade
		explicit HeatFlowModeUpgradeManager(mbapi::Model& model);

		~HeatFlowModeUpgradeManager() final = default;

		/// @brief Upgrades the model to as per the BPA2 standard of heat flow inputs
		/// @details checks for the values for all the related tables for its valid ranges. Also the BPA2 default fields are also checked for its value in the legacy file.
		//			 Updates the MntlHeatFlowIoTbl by providing with the heat flow history at the basement age history predefined to the simulator. 
		void upgrade() final;

		void upgradeBasementIoTbl();
		void upgradeMantleHeatFlowIoTbl();

		//@brief update/reset the value of the propName of the tableName to a specific value. Eg, for HeatFlow mode we need to reset InitialLithosphericMantleThickness with 0 regardless of the legacy value .
		void upgradeBasementRelatedProperty(const std::string & tableName, const std::string & propName, double value);

		//@brief update the crust and mantle property models depending on the bottomBoundaryModelName
		void upgradeMantlePropertyModel(const std::string & bottomBoundaryModelName, const std::string & tableName, const std::string & propName);
		void upgradeCrustPropertyModel(const std::string & bottomBoundaryModelName, const std::string & tableName, const std::string & propName);
	
	};
}

#endif
