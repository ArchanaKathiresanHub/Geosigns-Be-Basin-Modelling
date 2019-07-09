//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_TOP_BOUNDARY_UPGRADE_MANAGER_H
#define PROGRADE_TOP_BOUNDARY_UPGRADE_MANAGER_H

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
	/// @class TopBoundaryUpgradeManager Manager to upgrade the BPA1 surfaceDepthIoTbl and surfaceTempIoTbl
	/// to BPA2 surfaceDepthIoTbl and surfaceTempIoTbl
	/// @brief Legacy(BPA1) projects use layerwise parameters whereas BPA2 uses globle parameters.
	class TopBoundaryUpgradeManager : public IUpgradeManager {

	public:
		TopBoundaryUpgradeManager() = delete;
		TopBoundaryUpgradeManager(const TopBoundaryUpgradeManager &) = delete;
		TopBoundaryUpgradeManager& operator=(const TopBoundaryUpgradeManager &) = delete;

		/// @param[in] model The model on which to perform the surfaceDepthIoTbl and surfaceTempIoTbl upgrade
		/// throw std::invalid_argument if the model's project handle is null
		explicit TopBoundaryUpgradeManager(mbapi::Model& model);
		

		~TopBoundaryUpgradeManager() final = default;

		/// @brief Upgrades the table values  in surfaceDepthIoTbl and create surfaceTempIoTbl
		void upgrade() final;

		/// @breif modifies the ages by clipping anything above 999 to between 998 and 999, 
		/// depending on the number of sufrace ages above 999
		static bool clipAndBufferSurfaceAges(std::vector<double>& surfaceAgeList);


	private:

		mbapi::Model& m_model; ///< The model to upgrade
		std::shared_ptr<DataAccess::Interface::ProjectHandle> m_ph; ///< The project handle of the model to upgrade

		void upgradeSurfaceDepthIoTable();
		void upgradeSurfaceTempIoTable();
	};
}

#endif
