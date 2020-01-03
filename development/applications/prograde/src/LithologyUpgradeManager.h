//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_LITHOLOGY_UPGRADE_MANAGER_H
#define PROGRADE_LITHOLOGYE_UPGRADE_MANAGER_H

//Prograde
#include "IUpgradeManager.h"

//std
#include <memory>
#include <vector>

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
	/// @class LithologyUpgradeManager Manager to upgrade the deprecated Constant model to modified B&W model for density and seismic velocity calculations of LITHOLOGYes
	/// @brief Legacy projects have two models for calculating density and seismic velocity of LITHOLOGYes, namesly costant and modified B&W model.
	///        The constant model has been deprecated in BPA2. Upgrade manager is created to convert constant model specification to modified B&W. 
	class LithologyUpgradeManager : public IUpgradeManager {

	public:
		LithologyUpgradeManager() = delete;
		LithologyUpgradeManager(const LithologyUpgradeManager &) = delete;
		LithologyUpgradeManager& operator=(const LithologyUpgradeManager &) = delete;

		/// @param[in] model The model on which to perform the LITHOLOGYe model upgrade
		/// throw std::invalid_argument if the model's project handle is null
		explicit LithologyUpgradeManager(mbapi::Model& model);

		~LithologyUpgradeManager() final = default;

		/// @brief Upgrades the model to use new BPA2 lothology names, descriptions, parent lithology, Porosity/Permeability models of BPA2 for all the BPA1 lithologies
		/// @details If the model is user defined lithology, then do nothing
		void upgrade() final;

	private:

		mbapi::Model& m_model; ///< The model to upgrade
		std::shared_ptr<DataAccess::Interface::ProjectHandle> m_ph; ///< The project handle of the model to upgrade
	};
}

#endif
