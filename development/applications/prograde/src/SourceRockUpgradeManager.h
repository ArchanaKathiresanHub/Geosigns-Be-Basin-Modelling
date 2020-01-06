//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef PROGRADE_SOURCEROCK_UPGRADE_MANAGER_H
#define PROGRADE_SOURCEROCK_UPGRADE_MANAGER_H

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
	/// @class SourceRockUpgradeManager Manager to upgrade the BPA1 SourceRockLithoIoTbl to BPA2 SourceRockLithoIoTbl standard
	/// @brief Legacy(BPA) projects have TOC dependent adsorption model which are converted to Langmuir Isotherm adsorption model in BPA2
	class SourceRockUpgradeManager : public IUpgradeManager {

	public:
		SourceRockUpgradeManager() = delete;
		SourceRockUpgradeManager(const SourceRockUpgradeManager &) = delete;
		SourceRockUpgradeManager& operator=(const SourceRockUpgradeManager &) = delete;

		/// @param[in] model The model on which to perform the shale gas related fields of SourceRockLithoIoTbl upgrade
		/// throw std::invalid_argument if the model's project handle is null
		explicit SourceRockUpgradeManager(mbapi::Model& model);

		~SourceRockUpgradeManager() final = default;

		/// @brief Upgrades the model to use Langmuir Isotherm adsorption model in BPA2
		void upgrade() final;


	private:

		mbapi::Model& m_model; ///< The model to upgrade
		std::shared_ptr<DataAccess::Interface::ProjectHandle> m_ph; ///< The project handle of the model to upgrade
	};
}

#endif

