//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// std
#include<algorithm>

//Prograde
#include "SourceRockUpgradeManager.h"
#include "SourceRockConverter.h"
//Prograde class to update the GridMapIoTbl if any GridMap is removed from any table
#include "GridMapIoTblUpgradeManager.h"
/**Static function named 'Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNamepReferenceGridMap()' is defined for the operation
* Overload 1: Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNamepReferenceGridMap("tableName"); //clears all the map references ReferredBy the table "tableName" from GridMapIoTbl
* Overload 2: Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNamepReferenceGridMap("tableName","mapName"); //clears the map reference of the "mapName" ReferredBy "tableName" from GridMapIoTbl
*/

//utilities
#include "LogHandler.h"
#include "NumericFunctions.h"
//cmbAPI
#include "cmbAPI.h"
#include "StratigraphyManager.h"
#include "SourceRockManager.h"


//DataAccess
#include "ProjectHandle.h"

using namespace mbapi;

//------------------------------------------------------------//

Prograde::SourceRockUpgradeManager::SourceRockUpgradeManager(Model& model) :
	IUpgradeManager("SourceRock upgrade manager"), m_model(model)
{
	const auto ph = m_model.projectHandle();
	if (ph == nullptr) {
		throw std::invalid_argument(getName() + " cannot retreive the project handle from Cauldron data model");
	}
	m_ph = ph;
}

//------------------------------------------------------------//
void Prograde::SourceRockUpgradeManager::upgrade() {

	// Initialize the SrConvertor 
	Prograde::SourceRockConverter modelConverter;

	// SourceRocks from SrLithIo
	auto SrLithIoSrIds = m_model.sourceRockManager().sourceRockIDs();
	
	// From StratIotbl get records that are SourceRocks
	auto StartIoLayIds = m_model.stratigraphyManager().layersIDs();

	// to detect if there is atleast one mixed SR case in the scenario
	bool MixingOn = false;

	// The SourceRock layers in the StratIoTbl
	std::vector<size_t> srLayersFromStratIoTbl{};

	// The active SourceRocks in the StratIoTbl
	theValidSrMixList listOfSrInSrLithoIoTblThatMustExist;

	// list of the bpa2 names of SourceRocks with their corresponding Hi values
	std::vector<bpa2nameHiPair> theNameHiList;
	std::vector<LayIdStratIoLayNamePair> SnglSrsFromStratIoLayIdNamePairList;

	// list of SourceRock Mixing(HI/HC) Grids which are cleared if the SR is inactive
	std::vector<std::string> srInactiveMixHIGridsCleared, srInactiveMixHCgridsCleared;

	//// ============================ 1. Check StratIoTbl ================================================////
	// for all the layers
	std::for_each(StartIoLayIds.begin(), StartIoLayIds.end(),
		[&](size_t& LayIdFromStratIo) // with layer id
		{
			std::vector<std::string> theSRTypesInStraiIo{};
			// Get the SourceRockTypes from StratIoTbl; return size()=zero if SR in not active, 
			// returns size()=1 if one SR and size()=2 if mixing is enabled
			theSRTypesInStraiIo = m_model.stratigraphyManager().sourceRockTypeName(LayIdFromStratIo);
			auto LayerNameInStratIo = m_model.stratigraphyManager().layerName(LayIdFromStratIo);

			if (m_model.stratigraphyManager().isSourceRockActive(LayIdFromStratIo))
			{
				size_t srId = 0;
				srLayersFromStratIoTbl.push_back(LayIdFromStratIo);

				// this returns true only if SR is active
				bool isEnbl = m_model.stratigraphyManager().isSourceRockMixingEnabled(LayIdFromStratIo);
				bool isSulfurous = false;

				// case 1: Check Mixing
				if (isEnbl) 
				{
					// the second source rock type is a sure thing
					auto sr2 = &theSRTypesInStraiIo[1];
					isSulfurous = modelConverter.isBpaSrSulfurous(*sr2);
					theNameHiList.push_back(GetBpa2SourceRockNamesFromBpaNamesOfThisLayer((*sr2), LayIdFromStratIo,
						srId, &modelConverter, isEnbl));
					// changeing the same to new name
					*sr2 = theNameHiList.back().first.second;

					listOfSrInSrLithoIoTblThatMustExist.push_back(std::make_pair(srId,true)); // true => mixed SR2
					// setSourceRockTypeName works as: sets 2 sr names iff mixing is enabled, sets 1 SR name is one
					// active sr is present and none if no Sr in this layer
					m_model.stratigraphyManager().setSourceRockTypeName(LayIdFromStratIo, theSRTypesInStraiIo);
					// \m_model.stratigraphyManager().setSourceRockMixingEnabled(LayIdFromStratIo, isEnbl);
					MixingOn = true;
				}
				else
				{
					// Try and get the second SR name from this layer in STratIoTbl
					auto srNa2 = m_model.tableValueAsString("StratIoTbl", LayIdFromStratIo, "SourceRockType2");
					// SR is active, EnableMixing is off but still SourceRockType2 is present
					if (srNa2.compare("")) {// if there is a name in SR2
						theSRTypesInStraiIo.push_back(""); // expand the vector to 2 elements
						//(*sr2) = ""; // null set the sr2
						// \m_model.stratigraphyManager().setSourceRockTypeName(LayIdFromStratIo, theSRTypesInStraiIo);
						// \m_model.stratigraphyManager().setSourceRockMixingEnabled(LayIdFromStratIo, isEnbl);
					}
				}
				auto sr1 = &theSRTypesInStraiIo[0];
				// case 2: this is a 1st SR in the layer; change it to new name
				if ((*sr1).compare("")) // if there is a name
				{
					bool is1stSrSulfurous = modelConverter.isBpaSrSulfurous(*sr1);
					theNameHiList.push_back(GetBpa2SourceRockNamesFromBpaNamesOfThisLayer((*sr1), LayIdFromStratIo,
						srId, &modelConverter, isEnbl));
					// changeing the same to new name
					*sr1 = theNameHiList.back().first.second;
					listOfSrInSrLithoIoTblThatMustExist.push_back(std::make_pair(srId, false)); // false = non-mixed
					m_model.stratigraphyManager().setSourceRockTypeName(LayIdFromStratIo, theSRTypesInStraiIo);
					if (!isEnbl) {
					/* ======== THIS IS A TEMPORARY SOLUTION OF PLACING measuredHI of a SINGLE Source Rock in the SourceRockMixingHI
					=========== field, for import to work =======================================================================*/
						SnglSrsFromStratIoLayIdNamePairList.push_back(std::make_pair(LayIdFromStratIo, LayerNameInStratIo));
					}
					else {
						// checking mixing of unrelated end-members w.r.t sulfur
						listOfSrInSrLithoIoTblThatMustExist.back().second = true;
						if (isSulfurous != is1stSrSulfurous)
						{
							LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS)
								<< "Note: Mixing of sulfur and non sulfur source rock is identified in source rock mixing in Layer '"
								<< LayerNameInStratIo << "'  and BPA2 does not allow mixing of sulfur and non-sulfur source rocks";
							LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Migration from BPA to BPA2 Basin Aborted...";
							exit(1);
						}
					}
				}
				else
					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "SourceRockActive=1 but no SourceRockType1";

			}
			// clear out entries in SRs in STratIoTbl in which SourceRock is disabled
			else/* if(theSRTypesIntStraiIo.size()>0) -> this can not be checked to narrow scope cause
				 theSRTypesIntStraiIo == null when SR inavtive althought names might be preset:
				 see implementation of stratigraphyManager().isSourceRockActive*/
			{
				theSRTypesInStraiIo.resize(2); 
				m_model.stratigraphyManager().setSourceRockTypeName(LayIdFromStratIo,theSRTypesInStraiIo);
				m_model.stratigraphyManager().setSourceRockMixingEnabled(LayIdFromStratIo, false);
				// the above should be the sequence of calls to update SourceRockTypeNames and RockMixingEnabled
				// see implementation of both the methods

				// making a lists of SR mixing grids which have been removed from StratIoTbl if SR is inactive
				srInactiveMixHIGridsCleared.push_back(m_model.tableValueAsString("StratIoTbl", LayIdFromStratIo, "SourceRockMixingHIGrid"));
				srInactiveMixHCgridsCleared.push_back(m_model.tableValueAsString("StratIoTbl", LayIdFromStratIo, "SourceRockMixingHCGrid"));

				m_model.setTableValue("StratIoTbl", LayIdFromStratIo, "SourceRockMixingHI", DataAccess::Interface::DefaultUndefinedScalarValue);// Changed as per the BPA2 standard for no SR layer
				
				m_model.setTableValue("StratIoTbl", LayIdFromStratIo, "SourceRockMixingHIGrid", "");
				/*Note: Not needed to set SourceRockMixingHC/SourceRockMixingHCGrid fields explicitely. 
						Can we remove these two fields from the BasinModellerProperty.Spec file as these two fields are already removed from the downloaded p3d file from BPA2 	*/			
				m_model.setTableValue("StratIoTbl", LayIdFromStratIo, "SourceRockMixingHC", 0.);
				m_model.setTableValue("StratIoTbl", LayIdFromStratIo, "SourceRockMixingHCGrid", "");
			}
		}
	);
	// If SourceRock is not enabled
	if (srLayersFromStratIoTbl.size() == 0) {
		if (cleanSourceRockLithoIoTbl() == ErrorHandler::NoError)
		{
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "SourceRock disabled, clearing SourceRockLithoIoTbl! ";
			Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNamepReferenceGridMap("SourceRockLithoIoTbl");
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "GridMaps ReferredBy SourceRockLithoIoTbl (if any) will be cleared by GridMapIoTbl Upgrade Manager";
		}
		else
			throw ErrorHandler::Exception(ErrorHandler::ValidationError) << "Something went wrong in cleaning SourceRockLithoIoTbl";
		
		return;
	}
	else 
	{
		// clear up strayed SR entries from SourceRockLithoIoTbl that was left behind
		std::sort(listOfSrInSrLithoIoTblThatMustExist.begin(), listOfSrInSrLithoIoTblThatMustExist.end());
		auto err = UpdateOfInconsistentEntriesInSrLithoIoTbl(listOfSrInSrLithoIoTblThatMustExist, &modelConverter);
		if(err == ErrorHandler::NoError)
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Inconsistencies in SourceRockLithoIoTbl addressed! ";
		else
			throw ErrorHandler::Exception(ErrorHandler::ValidationError) << "Something went wrong in cleaning SourceRockLithoIoTbl";
		// Collect consolidated record in a vector srLayersFromStratIoTbl
		if (MixingOn) {
			CheckValidHiRangesForMixedSRs(srLayersFromStratIoTbl,theNameHiList);
		}

		// 2. Change SourceRockLithoIoTbl 
		if (SetSourceRockPropertiesForBPA2(&modelConverter, listOfSrInSrLithoIoTblThatMustExist, SnglSrsFromStratIoLayIdNamePairList)
			== ErrorHandler::NoError)
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "SourceRocks updated In SourceRockLithoIoTbl! ";
		else
			throw ErrorHandler::Exception(ErrorHandler::ValidationError) << "Something went wrong in updating SourceRockLithoIoTbl";

		// ============================== SourceRockLithoIoTbl updated =================================== //
		
	}

	// Removing unnecessary map references related to SourceRockLithoIoTbl from GridMapIoTbl 
	std::vector<std::string> srLithoIoTblMaps; //vector containing the list of maps in SourceRockLithoIoTbl; 
	database::Table * srLithoiotbl = m_ph->getTable("SourceRockLithoIoTbl");
	for (size_t id = 0; id < srLithoiotbl->size(); ++id)
	{
		database::Record * rec = srLithoiotbl->getRecord(static_cast<int> (id));
		std::string TocIniGridMap = rec->getValue<std::string>("TocIniGrid");
		srLithoIoTblMaps.push_back(TocIniGridMap);
	}
	// Identifying the unnecessary map references in GridMapIoTbl ReferredBy SourceRockLithoIoTbl
	database::Table * gridMapiotbl = m_ph->getTable("GridMapIoTbl");
	for (size_t id = 0; id < gridMapiotbl->size(); ++id)
	{
		database::Record * rec = gridMapiotbl->getRecord(static_cast<int> (id));
		std::string referredBy = rec->getValue<std::string>("ReferredBy");
		if (referredBy == "SourceRockLithoIoTbl")
		{
			std::string mapNameFromIoTbl = rec->getValue<std::string>("MapName");
			for (int i = 0; i < srLithoIoTblMaps.size(); ++i)
			{
				if (mapNameFromIoTbl == srLithoIoTblMaps[i]) break;
				else if (i == srLithoIoTblMaps.size()-1)
				{
					Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNamepReferenceGridMap("SourceRockLithoIoTbl", mapNameFromIoTbl);
					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "GridMap " << mapNameFromIoTbl << " ReferredBy SourceRockLithoIoTbl will be cleared by GridMapIoTbl Upgrade Manager as the map has no reference in SourceRockLithoIoTbl";
				}
			}
		}
	}

	//Identifying the unnecessary map references in GridMapIoTbl ReferredBy StratIoTbl for inactive SR - SR mixing Gridmaps
	database::Table * stratioTbl = m_ph->getTable("StratIoTbl");
	// Inactive SR - SourceRockMixingHIGrid cleared
	for (int i = 0; i < srInactiveMixHIGridsCleared.size(); i++)
	{
		for (size_t id = 0; id < stratioTbl->size(); id++)
		{
			database::Record * rec = stratioTbl->getRecord(static_cast<int> (id));
			std::string srMixHIGrid = rec->getValue<std::string>("SourceRockMixingHIGrid");
			if (srInactiveMixHIGridsCleared[id] == srMixHIGrid) break;
			else if (id == stratioTbl->size() - 1)
			{
				Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNamepReferenceGridMap("StratIoTbl", srMixHIGrid);
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "GridMap " << srMixHIGrid << " ReferredBy StratIoTbl will be cleared by GridMapIoTbl Upgrade Manager as the map is no longer referenced in StratIoTbl";
			}
		}
	}
	// Inactive SR - SourceRockMixingHCGrid cleared
	for (int i = 0; i < srInactiveMixHCgridsCleared.size(); i++)
	{
		for (size_t id = 0; id < stratioTbl->size(); ++id)
		{
			database::Record * rec = stratioTbl->getRecord(static_cast<int> (id));
			std::string srMixHCGrid = rec->getValue<std::string>("SourceRockMixingHCGrid");
			if (srInactiveMixHCgridsCleared[id] == srMixHCGrid) break;
			else if (id == stratioTbl->size() - 1)
			{
				Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNamepReferenceGridMap("StratIoTbl", srMixHCGrid);
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "GridMap " << srMixHCGrid << " ReferredBy StratIoTbl will be cleared by GridMapIoTbl Upgrade Manager as the map is no longer referenced in StratIoTbl";
			}
		}
	}
}

ErrorHandler::ReturnCode Prograde::SourceRockUpgradeManager::cleanSourceRockLithoIoTbl(void) const
{
	return m_model.clearTable("SourceRockLithoIoTbl");
}

ErrorHandler::ReturnCode Prograde::SourceRockUpgradeManager::SetSourceRockPropertiesForBPA2(Prograde::SourceRockConverter* mConvert,
	const theValidSrMixList& ValidSrIds,
	std::vector<LayIdStratIoLayNamePair>& SnglSrsFromStratIoLayIdNamePairList)
{
	ErrorHandler::ReturnCode err = ErrorHandler::NoError;
	auto srLayIds = m_model.sourceRockManager().sourceRockIDs();
	
	for (auto srRecord : ValidSrIds) {
		size_t sourceRockId = srRecord.first;
		auto layerName = m_model.sourceRockManager().layerName(sourceRockId);
		auto bpaSourceRockTypeName = m_model.sourceRockManager().sourceRockType(sourceRockId);
		auto bpaScVre05 = m_model.sourceRockManager().scIni(sourceRockId) > 0;
		auto legacyEa = m_model.sourceRockManager().preAsphActEnergy(sourceRockId);
		auto bpaAsphalteneDE = m_model.sourceRockManager().getAsphalteneDiffusionEnergy(sourceRockId);
		auto bpaResinDE = m_model.sourceRockManager().getResinDiffusionEnergy(sourceRockId);
		auto bpaC15AroDE = m_model.sourceRockManager().getC15AroDiffusionEnergy(sourceRockId);
		auto bpaC15SatDE = m_model.sourceRockManager().getC15SatDiffusionEnergy(sourceRockId);
		auto legacyVESlimitIndicator = m_model.sourceRockManager().getVESlimitIndicator(sourceRockId);
		auto VESlimit = m_model.sourceRockManager().getVESlimit(sourceRockId);
		auto legacyVREoptimization = m_model.sourceRockManager().getVREoptimization(sourceRockId);
		auto VREthreshold = m_model.sourceRockManager().getVREthreshold(sourceRockId);
		// the Hi is rounded to nearest lower integer since its from the Genex6::GenexSourceRock::convertHCtoHI( hcIni )
		auto bpaHiIni = std::floor( m_model.sourceRockManager().hiIni(sourceRockId));
		auto bpaHcIni = m_model.sourceRockManager().hcIni(sourceRockId);
		auto bpaBaseSourceRockType = m_model.sourceRockManager().baseSourceRockType(sourceRockId);
		// return data
		std::string bpa2SourceRockTypeName, bpa2BaseSourceRockType; bool litFlag{};
		upgradeToBPA2Names(mConvert,bpaSourceRockTypeName,
			bpaBaseSourceRockType, bpa2SourceRockTypeName, bpa2BaseSourceRockType, litFlag , bpaScVre05);
		(*mConvert).upgradeDiffusionEnergy(bpaBaseSourceRockType, bpaScVre05, bpaAsphalteneDE, 
			bpaResinDE, bpaC15AroDE, bpaC15SatDE);
		(*mConvert).upgradeVESlimit(legacyVESlimitIndicator, VESlimit);
		(*mConvert).upgradeVREthreshold(legacyVREoptimization, VREthreshold);
		//change mixed SRs to default in SourceRockLithoIoTbl
		if (srRecord.second) {
			double HiRange[2]{};
			auto newVal = mConvert->upgradeEa(bpaSourceRockTypeName, bpaBaseSourceRockType, legacyEa, bpaScVre05);
			err = m_model.sourceRockManager().setPreAsphActEnergy(sourceRockId, newVal);
			newVal = mConvert->upgradeHcVre05(bpaSourceRockTypeName, bpaBaseSourceRockType, bpaHcIni, bpaScVre05,HiRange[0],HiRange[1]);
			err = m_model.sourceRockManager().setHCIni(sourceRockId, newVal);
			newVal = mConvert->upgradeScVre05(bpaSourceRockTypeName, bpaBaseSourceRockType, bpaScVre05);
			err = m_model.sourceRockManager().setSCIni(sourceRockId, newVal);
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "SourceRock " << bpa2SourceRockTypeName
				<<" used in a mixed source rock layer "
				<< layerName << " is reset to default properties";
		}
#if 0		// This was a temporary solution solution and now not needed as this has been now taken care in the middle tier in PBI #.
		else {
			// This is the PART where HI value for Single SR is put into SourceRockMixingHI
			// \ has to be removed later when "middle" tire is corrected
			auto itSrId = std::find_if(SnglSrsFromStratIoLayIdNamePairList.begin(), SnglSrsFromStratIoLayIdNamePairList.end(),

				[&](LayIdStratIoLayNamePair& element) -> bool {
					return !element.second.compare(layerName);
				}

			);
			if (itSrId != SnglSrsFromStratIoLayIdNamePairList.end()) {
				m_model.setTableValue("StratIoTbl", (*itSrId).first, "SourceRockMixingHI", bpaHiIni);
			}
		}
#endif
		err = m_model.sourceRockManager().setSourceRockType(sourceRockId, bpa2SourceRockTypeName);
		err = m_model.sourceRockManager().setBaseSourceRockType(sourceRockId, bpa2BaseSourceRockType);
		err = m_model.sourceRockManager().setAsphalteneDiffusionEnergy(sourceRockId, bpaAsphalteneDE);
		err = m_model.sourceRockManager().setResinDiffusionEnergy(sourceRockId, bpaResinDE);
		err = m_model.sourceRockManager().setC15AroDiffusionEnergy(sourceRockId, bpaC15AroDE);
		err = m_model.sourceRockManager().setC15SatDiffusionEnergy(sourceRockId, bpaC15SatDE);
		err = m_model.sourceRockManager().setVESlimit(sourceRockId, VESlimit);
		err = m_model.sourceRockManager().setVREthreshold(sourceRockId, VREthreshold);
	}
	return err;
}



void Prograde::SourceRockUpgradeManager::upgradeToBPA2Names(Prograde::SourceRockConverter* mConvert, 
	const std::string& bpa1SR,	const std::string& bpa1BSR,
	std::string& bpa2SR, std::string& bpa2BSR,
	bool& isLit, bool& isSulfur)
{
	(*mConvert).upgradeSourceRockName(bpa1SR, bpa1BSR, isSulfur, bpa2SR, bpa2BSR, isLit);
}


Prograde::bpa2nameHiPair Prograde::SourceRockUpgradeManager::GetBpa2SourceRockNamesFromBpaNamesOfThisLayer(const std::string& bpaSrName,
	const size_t& LayIdFromStraIOTbl, size_t& srIdFromSrLitoIoTbl,const Prograde::SourceRockConverter* mConvert, bool isMixingEnable)
{
	// SourcRocks from SrLithIo
	auto LyrNameStarIo = m_model.stratigraphyManager().layerName(LayIdFromStraIOTbl);
	auto SrLithIoSrIds = m_model.sourceRockManager().sourceRockIDs();
	for (auto it = SrLithIoSrIds.begin(); it != SrLithIoSrIds.end(); ++it)
	{
		auto sr = m_model.sourceRockManager().sourceRockType(*it);
		auto lr = m_model.sourceRockManager().layerName(*it);

		if ((!sr.compare(bpaSrName)) && (!lr.compare(LyrNameStarIo))) {
			auto bsr = m_model.sourceRockManager().baseSourceRockType(*it);
			srIdFromSrLitoIoTbl = *it; 
			if (mConvert->isSrFromLiterature(bsr, &bpaSrName) && isMixingEnable) {
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Note: Literature source is identified in source rock mixing in Layer '"
					<< LyrNameStarIo << "' and BPA2 does not allow literature source rocks in source rock mixing'";
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Migration from BPA to BPA2 Basin Aborted...";
				exit(1);
			}
			auto theBpa2Hi = mConvert->GetmeasuredHI(m_model.sourceRockManager().baseSourceRockType(srIdFromSrLitoIoTbl), &bpaSrName);
			auto theSrNames = std::make_pair(
				mConvert->GetBPA2BaseRockName(m_model.sourceRockManager().baseSourceRockType(srIdFromSrLitoIoTbl),
				&bpaSrName), 
				mConvert->GetBPA2RockName(m_model.sourceRockManager().baseSourceRockType(srIdFromSrLitoIoTbl),
				&bpaSrName));

			return std::make_pair(theSrNames, theBpa2Hi);
		}
	}

	return std::make_pair(std::make_pair(std::string("Something is fishy BaseSr!"), std::string("Something is fishy Sr!")),Utilities::Numerical::IbsNoDataValue);
}

void Prograde::SourceRockUpgradeManager::CheckValidHiRangesForMixedSRs(const std::vector<size_t>& theActiveSrs, 
	std::vector < bpa2nameHiPair>& SrNameHiList)
{
	for (auto stratIoSRs : theActiveSrs)
	{
		auto enbl = m_model.stratigraphyManager().isSourceRockMixingEnabled(stratIoSRs);
		auto layName = m_model.stratigraphyManager().layerName(stratIoSRs);
		if (enbl) {
			auto SRs = m_model.stratigraphyManager().sourceRockTypeName(stratIoSRs);
			// \ Only for Scalar Hi enties
			if (!m_model.tableValueAsString("StratIoTbl", stratIoSRs, "SourceRockMixingHIGrid").compare(""))
			{
				auto mixHi = m_model.stratigraphyManager().sourceRockMixHI(stratIoSRs);
				double arr[2] = { 0,0 };

				auto it = std::find_if(SrNameHiList.begin(), SrNameHiList.end(),
					[&](Prograde::bpa2nameHiPair& element) ->bool {
						auto is = (element.first.second == SRs[0]);
						if (is)arr[0] = element.second;
						return is;
					});

				auto it2 = std::find_if(SrNameHiList.begin(), SrNameHiList.end(),
					[&](Prograde::bpa2nameHiPair& element) ->bool {
						auto is = (element.first.second == SRs[1]);
						if (is)arr[1] = element.second;
						return is;
					});
				int n = sizeof(arr) / sizeof(arr[0]);
				std::sort(arr, arr + n);
				if (it != SrNameHiList.end() && it2 != SrNameHiList.end()) {
					if (!NumericFunctions::inRange(mixHi, arr[0], arr[1])) {
						LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) <<
							"Note: Target Source Rock Mixing HI  in Layer '"
							<< layName << "' is: " << mixHi <<
							" which is out of allowed range (" << arr[0] << "," << arr[1] <<
							") for source rock mixing" << "in BPA2 Basin";
						LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Migration from BPA to BPA2 Basin Aborted...";
						exit(1);
					}
				}
			}
			else {
				m_model.stratigraphyManager().setSourceRockMixHI(stratIoSRs, Utilities::Numerical::IbsNoDataValue);
			}
		}
	}
}


ErrorHandler::ReturnCode Prograde::SourceRockUpgradeManager::UpdateOfInconsistentEntriesInSrLithoIoTbl(theValidSrMixList& ValidSrIds,
	const Prograde::SourceRockConverter* mConvert)
{
	ErrorHandler::ReturnCode err = ErrorHandler::NoError;
	// SourceRocks from SrLithIo
	auto SrLithIoSrIds = m_model.sourceRockManager().sourceRockIDs();
	int countOfDelResId = 0; int originalResIDPosition = -1;
	int renumerSrId = 0;
	
	std::for_each(SrLithIoSrIds.begin(), SrLithIoSrIds.end(),
		[&](size_t & SrId) {
			originalResIDPosition++;
			auto it = std::find_if(ValidSrIds.begin(), ValidSrIds.end(),
				[&](pair<size_t, bool>& element){
					bool is = (element.first == SrId);
					return is;
				});

			if (it == ValidSrIds.end()) {
				// not found in "Exists" list
				countOfDelResId++;
				
				auto idRem = (originalResIDPosition + 1) - countOfDelResId;

				auto t = m_model.tableSize("SourceRockLithoIoTbl");
				auto srRem = m_model.sourceRockManager().sourceRockType(idRem);
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Inactive SourceRock " <<
					srRem <<
					" removed from SourceRockLithoIoTbl";

				err = m_model.removeRecordFromTable("SourceRockLithoIoTbl", idRem); 
			}
			else {
				// Re-order the srId after deleting SourceRockLithoIoTbl rows
				(*it).first = renumerSrId; renumerSrId++;
			}
		}
	);
	return err;
}