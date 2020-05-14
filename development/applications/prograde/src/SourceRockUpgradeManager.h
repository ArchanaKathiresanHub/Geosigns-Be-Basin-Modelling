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
// errors
#include "ErrorHandler.h"
//std
#include <memory>
#include <vector>
#include<string>
namespace mbapi {
	class Model;
}

namespace DataAccess {
	namespace Interface {
		class ProjectHandle;
	}
}
// forward declare
namespace Prograde {
	class SourceRockConverter;
}

namespace database
{
	class Record;
}

namespace Prograde
{
	// @brief the pair of <bpa2BaseSR,bpa2SR> &            the bpa2 Hi
	typedef std::pair<std::pair<std::string, std::string>, double> bpa2nameHiPair;
	typedef std::vector<std::pair<size_t, bool>> theValidSrMixList;
	typedef std::pair<size_t, std::string> LayIdStratIoLayNamePair;
	/// @class SourceRockUpgradeManager Manager to upgrade the BPA1 SourceRockLithoIoTbl to BPA2 SourceRockLithoIoTbl standard
	/// @brief 
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

		/// @brief
		/// this is to clear-out the SourceRockLithoIoTbl if there are no active source rocks in the startigraphy 
		ErrorHandler::ReturnCode cleanSourceRockLithoIoTbl(void) const;
	protected:
		/// @brief method for Updating SourceRockLithoIoTbl to BPA2 standards
		ErrorHandler::ReturnCode SetSourceRockPropertiesForBPA2(Prograde::SourceRockConverter* mConvert
			,const theValidSrMixList& ValidSrIds, std::vector<LayIdStratIoLayNamePair>& SnglSrsFromStratIoLayIdNamePairList);
		/// @brief method for Updating source rock names to BPA2 standards
		void upgradeToBPA2Names(Prograde::SourceRockConverter* mConvert, const std::string& bpa1SR, const std::string& bpa1BSR, std::string& bpa2SR, std::string& bpa2BSR, bool &isLit, bool &isSulfur);
		/// @brief method for getting source rock names as BPA2 standards
		bpa2nameHiPair GetBpa2SourceRockNamesFromBpaNamesOfThisLayer(const std::string &bpaSrName,
		const size_t & LayIdFromStraIOTbl, size_t& srIdFromSrLitoIoTbl, const Prograde::SourceRockConverter* mConvert=nullptr
		,bool isMixingEnable=false
		);
		/// @brief method for Updating SourceRockLithoIoTbl to BPA2 standards for mixed Source rocks
		void CheckValidHiRangesForMixedSRs(const std::vector<size_t>& theActiveSrs, std::vector < bpa2nameHiPair>& SrNameHiList);
		ErrorHandler::ReturnCode UpdateOfInconsistentEntriesInSrLithoIoTbl(theValidSrMixList& ValidSrIds,
			const Prograde::SourceRockConverter* mConvert);

	private:
		mbapi::Model& m_model; ///< The model to upgrade
		std::shared_ptr<DataAccess::Interface::ProjectHandle> m_ph; ///< The project handle of the model to upgrade
	};
}

#endif

