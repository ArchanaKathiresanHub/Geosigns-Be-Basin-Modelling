//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_FAULTCUT_UPGRADE_MANAGER_H
#define PROGRADE_FAULTCUT_UPGRADE_MANAGER_H

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
	/// @class FaultCutUpgradeManager is used to upgrade the SurfaceName and BottomFormationName columns of PalinspasticIoTbl by removing the special characters from them as in BPA2. It also updates the FaultCutIoTbl by removing the rows for which the Age is greater than or equal to the Basement age.
	/// @brief Legacy projects can have special characters in the names of layers, surfaces and fluidtypes. However, only alphanumeric characters are permitted in BPA2. 
	class FaultCutUpgradeManager : public IUpgradeManager {

	public:
		FaultCutUpgradeManager() = delete;
		FaultCutUpgradeManager(const FaultCutUpgradeManager &) = delete;
		FaultCutUpgradeManager& operator=(const FaultCutUpgradeManager &) = delete;

		/// @param[in] model The model on which to perform the stratigraphy model upgrade
		/// throw std::invalid_argument if the model's project handle is null
		explicit FaultCutUpgradeManager(mbapi::Model& model);

		~FaultCutUpgradeManager() final = default;

		/// @brief Upgrades the model to the BPA-2 standards
		/// @details Upgrades only if necessary
		void upgrade() final;

		// @brief Gets the LayerName for a given SurfaceName from StratIoTbl
		std::string getLayerNameFromStratIoTbl(const std::string&);
	private:

		mbapi::Model& m_model; ///< The model to upgrade
		std::shared_ptr<DataAccess::Interface::ProjectHandle> m_ph; ///< The project handle of the model to upgrade
	};
}

#endif

