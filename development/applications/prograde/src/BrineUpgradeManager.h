//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_BRINE_UPGRADE_MANAGER_H
#define PROGRADE_BRINE_UPGRADE_MANAGER_H

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
	/// @class BrineUpgradeManager Manager to upgrade the deprecated Constant model to modified B&W model for density and seismic velocity calculations of Brines
	/// @brief Legacy projects have two models for calculating density and seismic velocity of brines, namesly costant and modified B&W model.
	///        The constant model has been deprecated in BPA2. Upgrade manager is created to convert constant model specification to modified B&W. 
	class BrineUpgradeManager : public IUpgradeManager {

	public:
		BrineUpgradeManager() = delete;
		BrineUpgradeManager(const BrineUpgradeManager &) = delete;
		BrineUpgradeManager& operator=(const BrineUpgradeManager &) = delete;

		/// @param[in] model The model on which to perform the brine model upgrade
		/// throw std::invalid_argument if the model's project handle is null
		explicit BrineUpgradeManager(mbapi::Model& model);

		~BrineUpgradeManager() final = default;

		/// @brief Upgrades the model to use the modified B&W model only
		/// @details If the model is already using modified B&W model, then do nothing
		void upgrade() final;


	private:

		mbapi::Model& m_model; ///< The model to upgrade
		std::shared_ptr<DataAccess::Interface::ProjectHandle> m_ph; ///< The project handle of the model to upgrade
	};
}

#endif