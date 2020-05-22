//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_STRATIGRAPHY_UPGRADE_MANAGER_H
#define PROGRADE_STRATIGRAPHY_UPGRADE_MANAGER_H

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
	/// @class StratigraphyUpgradeManager is used to upgrade the SurfaceName, LayerName and Fluidtype columns of StratIoTbl by removing the special characters from them as in BPA2. This upgrade manager is also used to upgrade inputs-> the age limits, lithology mixing modes, depth and thickness values and chemical compaction models to inputs of BPA2 mathematical models.
	/// @brief Legacy projects can have special characters in the names of layers, surfaces and fluidtypes. However, only alphanumeric characters are permitted in BPA2. The scalar limits of depth, thicknesses and DepoAge are different in BPA2 than  BPA legacy. Moreover, ChemicalCompaction and Lithology MixModels LayeringIndex have different inputs than BPA legacy
	class StratigraphyUpgradeManager : public IUpgradeManager {

	public:
		StratigraphyUpgradeManager() = delete;
		StratigraphyUpgradeManager(const StratigraphyUpgradeManager &) = delete;
		StratigraphyUpgradeManager& operator=(const StratigraphyUpgradeManager &) = delete;

		/// @param[in] model The model on which to perform the stratigraphy model upgrade
		/// throw std::invalid_argument if the model's project handle is null
		explicit StratigraphyUpgradeManager(mbapi::Model& model);

		~StratigraphyUpgradeManager() final = default;

		/// @brief Upgrades the model to the BPA-2 standards
		/// @details Upgrades only if necessary
		void upgrade() final;
	private:

		mbapi::Model& m_model; ///< The model to upgrade
		std::shared_ptr<DataAccess::Interface::ProjectHandle> m_ph; ///< The project handle of the model to upgrade
	};
}

#endif