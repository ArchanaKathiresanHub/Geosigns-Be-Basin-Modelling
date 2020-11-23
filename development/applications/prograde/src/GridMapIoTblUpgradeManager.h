//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_GRIDMAPIOTBL_UPGRADE_MANAGER_H
#define PROGRADE_GRIDMAPIOTBL_UPGRADE_MANAGER_H

//Prograde
#include "IUpgradeManager.h"

//std
#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <map>

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
	/// @class 
	/// @brief 
	class GridMapIoTblUpgradeManager : public IUpgradeManager {

	public:
		GridMapIoTblUpgradeManager() = delete;
		GridMapIoTblUpgradeManager(const GridMapIoTblUpgradeManager &) = delete;
		GridMapIoTblUpgradeManager& operator=(const GridMapIoTblUpgradeManager &) = delete;

		/// @param[in] model The model on which to perform the stratigraphy model upgrade
		/// throw std::invalid_argument if the model's project handle is null
		explicit GridMapIoTblUpgradeManager(mbapi::Model& model);

		~GridMapIoTblUpgradeManager() final = default;

		/// @brief Upgrades the model to the BPA-2 standards
		/// @details Upgrades only if necessary
		void upgrade() final;

		/// @brief Static member method which fills the static member map mapNameTblName 
		/// @details The list of the maps and their reference tables removed contained in mapNameTblName, reference of those maps is removed from GridMapIoTbl when update() is called
		static void clearTblNameMapNameReferenceGridMap(const std::string&, const std::string&);

		/// @brief Overload of the function clearMapNameTblNamepReferenceGridMap
		/// @details If we want to clear all the maps referred by a table in GridMapIoTbl, pass the name of the table to the method
		static void clearTblNameMapNameReferenceGridMap(const std::string&);

	private:
		mbapi::Model& m_model; ///< The model to upgrade
		std::shared_ptr<DataAccess::Interface::ProjectHandle> m_ph; ///< The project handle of the model to upgrade
		static std::vector<std::pair<std::string, std::string>> tblNameMapName;///< Static member map which contains list of maps to be cleared from GridMapioTbl
	};
}
#endif

