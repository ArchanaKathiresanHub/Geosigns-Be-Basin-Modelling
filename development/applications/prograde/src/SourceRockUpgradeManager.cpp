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
#include <iomanip>
//Prograde
#include "SourceRockUpgradeManager.h"
#include "SourceRockConverter.h"
//Prograde class to update the GridMapIoTbl if any GridMap is removed from any table
#include "GridMapIoTblUpgradeManager.h"
/**Static function named 'Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap()' is defined for the operation
* Overload 1: Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap("tableName"); //clears all the map references ReferredBy the table "tableName" from GridMapIoTbl
* Overload 2: Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap("tableName","mapName"); //clears the map reference of the "mapName" ReferredBy "tableName" from GridMapIoTbl
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
					
//======== THIS IS A TEMPORARY SOLUTION OF PLACING measuredHI of a SINGLE Source Rock in the SourceRockMixingHI field,
#define HiAnamoly 0
//======================================================================================

using namespace mbapi;
using namespace std;

//------------------------------------------------------------//

Prograde::SourceRockUpgradeManager::SourceRockUpgradeManager(Model& model) :
	IUpgradeManager("SourceRock upgrade manager"), m_model(model)
{
	const auto ph = m_model.projectHandle();
	if (ph == nullptr) {
		throw std::invalid_argument(getName() + " cannot retrieve the project handle from Cauldron data model");
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

	// to detect if there is at least one mixed SR case in the scenario
	bool MixingOn = false;

	// The SourceRock layers in the StratIoTbl
	std::vector<size_t> srLayersFromStratIoTbl{};

	// The active SourceRocks in the StratIoTbl
	theValidSrMixList listOfSrInSrLithoIoTblThatMustExist;

	// list of the bpa2 names of SourceRocks with their corresponding Hi values -> SrId from SrLithoIo, and LayId from StratIo
	std::vector<bpa2nameHiPair> theNameHiList;
	std::vector<LayIdStratIoLayNamePair> SnglSrsFromStratIoLayIdNamePairList;

	// list of SourceRock Mixing(HI/HC) Grids which are cleared if the SR is inactive
	std::vector<std::string> srInactiveMixHIGridsCleared, srInactiveMixHCgridsCleared;

	//// === 0. Check and update SourceRockLithoIoTbl for H/C and Preasphaltene activation energy values to be in the respective limits ===========================////
	database::Table* sourceRockLithoIo_tbl = m_ph->getTable("SourceRockLithoIoTbl");
	if (sourceRockLithoIo_tbl->size() != 0) 
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Checking SourceRockLithoIoTbl for the H/C and PreAsphaltStartAct to be in the respective valid limits";
		std::string bpaBaseSR;
		std::string sourceRockType;
		std::string layerName;

		double Hc_original = 0.0;
		double Ea_original = 0.0;
		double Hc_updated = 0.0;
		double Ea_updated = 0.0;
		double ScVre = 0.0;
		int count = 0;
		for (int id = 0; id < sourceRockLithoIo_tbl->size(); ++id)
		{
			database::Record* rec = sourceRockLithoIo_tbl->getRecord(static_cast<int>(id));
			layerName = rec->getValue<std::string>("LayerName");
			sourceRockType = rec->getValue<std::string>("SourceRockType");
			bpaBaseSR = rec->getValue<std::string>("BaseSourceRockType");
			Hc_original = rec->getValue<double>("HcVRe05");
			Ea_original = rec->getValue<double>("PreAsphaltStartAct");
			ScVre = rec->getValue<double>("ScVRe05");
			Hc_updated = Hc_original;
			Ea_updated = Ea_original;

			modelConverter.limitHcEa(bpaBaseSR, Hc_updated, Ea_updated, ScVre);

			if (Hc_updated != Hc_original)
			{
				count++;
				rec->setValue<double>("HcVRe05", Hc_updated);
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> In the layer "<< layerName<<" BPA-legacy source rock "<< sourceRockType<<" has invalid HcVRe05 values for the BPA-legacy base source type "<< bpaBaseSR;
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Info> Hence, the HcVRe05 is updated from " << Hc_original << " to " << Hc_updated;
			}
			if (Ea_updated != Ea_original)
			{
				count++;
				rec->setValue<double>("PreAsphaltStartAct", Ea_updated);
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> In the layer " << layerName << " BPA-legacy source rock " << sourceRockType << " has invalid PreAsphaltStartAct values for the BPA-legacy base source type " << bpaBaseSR;
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Info> Hence, the PreAsphaltStartAct is updated from " << Ea_original << " to " << Ea_updated;
			}
		}
		if (count == 0)
		{
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> H/C and PreAsphaltStartAct for the Source Rocks are within the acceptable limits";
		}
		else LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> Note that BPA-legacy Source rock names will be updated in the project3d file; Please refer the prograde log to find the corresponding BPA-2 names for each of the Source rock";
	}

	//// ============================ 1. Check StratIoTbl ================================================////
	// for all the layers
	std::for_each(StartIoLayIds.begin(), StartIoLayIds.end(),
		[&](size_t const & LayIdFromStratIo) // with layer id
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
			bool isMixEnbl = m_model.stratigraphyManager().isSourceRockMixingEnabled(LayIdFromStratIo);
			// variables to detect invalid mixing cases, numbered with (*SlNo.)
			bool isSulfurous = false;
			bool is2ndSRLiteratureType = false;
			double hi2 = 0.0;
			// case 1: Check Mixing
			if (isMixEnbl)
			{
				// the second source rock type is a sure thing
				auto sr2 = &theSRTypesInStraiIo[1];
				isSulfurous = modelConverter.isBpaSrSulfurous(*sr2);
				// (*4) checking for literature type SR mixing, returns "","" as the names and 0.0 as measured Hi
				theNameHiList.push_back(GetBpa2SourceRockNamesFromBpaNamesOfThisLayer((*sr2), LayIdFromStratIo,
					srId, &modelConverter, isMixEnbl));
				 hi2 = theNameHiList.back().second.first;
				// if there is a VALID SR-mixing name 
				if (theNameHiList.back().first.second.compare(Utilities::Numerical::NoDataStringValue)) {
					listOfSrInSrLithoIoTblThatMustExist.push_back(std::make_pair(srId, true)); // true => mixed SR2
					// setSourceRockTypeName works as: sets 2 sr names iff mixing is enabled, sets 1 SR name is one
					// active sr is present and none if no Sr in this layer
					*sr2 = theNameHiList.back().first.second;
					// \m_model.stratigraphyManager().setSourceRockTypeName(LayIdFromStratIo, theSRTypesInStraiIo);
					// no need to set the 2nd SR here, we have collected the name in theSRTypesInStraiIo, we will set in one go when
					// we set the 1st SR
				}
				else {
					theNameHiList.pop_back()/* is a null string so clear*/;
					// changing the same to new name
					sr2 = nullptr;
					is2ndSRLiteratureType = true;
				}				
				
				MixingOn = true;
			}
			else
			{
				// Try and get the second SR name from this layer in STratIoTbl theSRTypesInStraiIo[1] will not work
				auto srNa2 = m_model.tableValueAsString("StratIoTbl", LayIdFromStratIo, "SourceRockType2");
				// SR is active, EnableMixing is off but still SourceRockType2 is present
				if (srNa2.compare("")) {// if there is a name in SR2... clear it out
					theSRTypesInStraiIo.push_back(""); // expand the vector to 2 elements ... for later use
													   //(*sr2) = ""; // set the sr2 to null
					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP)
						<< "<Basin-Warning> Found 2nd SourceRock in this layer '" << LayerNameInStratIo << "' although mixing flag disabled,"
						<< " clearing mixing parameters and the 2nd source rock <" << srNa2 << "> will be removed!";
#if !HiAnamoly
					clearMixingParams(LayIdFromStratIo);
#endif // !HiAnamoly
				}
			}
			auto sr1 = &theSRTypesInStraiIo[0];
			// case 2: this is a 1st SR in the layer; change it to new name
			if ((*sr1).compare("")) // if there is a name
			{
				bool is1stSrSulfurous = modelConverter.isBpaSrSulfurous(*sr1);
				theNameHiList.push_back(GetBpa2SourceRockNamesFromBpaNamesOfThisLayer((*sr1), LayIdFromStratIo,
					srId, &modelConverter, isMixEnbl));
				// changing the same to new name
				*sr1 = theNameHiList.back().first.second;
				auto hi1 = theNameHiList.back().second.first;
				bool is1stSRLiteratureType = !theNameHiList.back().first.second.compare(Utilities::Numerical::NoDataStringValue);

				if (!isMixEnbl) 
				{
					// This is a single SR
					m_model.stratigraphyManager().setSourceRockTypeName(LayIdFromStratIo, theSRTypesInStraiIo);
					listOfSrInSrLithoIoTblThatMustExist.push_back(std::make_pair(srId, false)); // isMixEnbl=false = non-mixed
#if	HiAnamoly
					SnglSrsFromStratIoLayIdNamePairList.push_back(std::make_pair(LayIdFromStratIo, LayerNameInStratIo));
#endif
				}
				else {
					if (is1stSRLiteratureType && !is2ndSRLiteratureType)listOfSrInSrLithoIoTblThatMustExist.pop_back();
					if (!is1stSRLiteratureType && !is2ndSRLiteratureType) {
						m_model.stratigraphyManager().setSourceRockTypeName(LayIdFromStratIo, theSRTypesInStraiIo);

						// (*1) checking mixing of unrelated end-members w.r.t sulfur
						if (isSulfurous != is1stSrSulfurous && !is2ndSRLiteratureType)
						{
							LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP)
								<< "<Basin-Warning> Mixing of sulfur and non sulfur source rock is identified in source rock mixing in Layer '"
								<< LayerNameInStratIo << "'  and BPA2 does not allow mixing of sulfur and non-sulfur source rocks; Migration from BPA to BPA2-Basin changed this layer as a non-source rock layer...";
							upgradeAsNormalLayer(LayIdFromStratIo);
							listOfSrInSrLithoIoTblThatMustExist.pop_back();
							theNameHiList.pop_back()/* is a improperSR so clear*/;

						}
						// (*2) checking same type SR mixing, sometimes in bpa names might differ but same measured hi can be used
						else if ((!theSRTypesInStraiIo[1].compare(theSRTypesInStraiIo[0]) || hi1 == hi2)
							&& theSRTypesInStraiIo[1] != ""
							&& !is2ndSRLiteratureType)
						{
							LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP)
								<< "<Basin-Warning> Mixing of same Source Rocks (or with identical measured Hi) is detected, <" << theSRTypesInStraiIo[0] << "> and <"
								<< theSRTypesInStraiIo[1] <<">, in source rock mixing in Layer '"
								<< LayerNameInStratIo << "'  and BPA2 does not allow mixing of same source rocks; Migration from BPA to BPA2-Basin changed this layer as a non-source rock layer...";
							upgradeAsNormalLayer(LayIdFromStratIo);
							listOfSrInSrLithoIoTblThatMustExist.pop_back();
							theNameHiList.pop_back()/* is a improperSR so clear*/;

						}
						else 
							listOfSrInSrLithoIoTblThatMustExist.push_back(std::make_pair(srId, true)); // isMixEnbl=true = mixed
					}
					else if(is1stSRLiteratureType || is2ndSRLiteratureType)
						theNameHiList.pop_back();
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
			upgradeAsNormalLayer(LayIdFromStratIo);
			// making a lists of SR mixing grids which have been removed from StratIoTbl if SR is inactive
			srInactiveMixHIGridsCleared.push_back(m_model.tableValueAsString("StratIoTbl", LayIdFromStratIo, "SourceRockMixingHIGrid"));
			srInactiveMixHCgridsCleared.push_back(m_model.tableValueAsString("StratIoTbl", LayIdFromStratIo, "SourceRockMixingHCGrid"));
		}
	}
	);
	
	// If SourceRock is not enabled
	if (srLayersFromStratIoTbl.empty()) {
		if (cleanSourceRockLithoIoTbl() == ErrorHandler::NoError)
		{
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> SourceRock disabled, clearing SourceRockLithoIoTbl! GridMaps ReferredBy SourceRockLithoIoTbl (if any) will be cleared by GridMapIoTbl Upgrade Manager";
			Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap("SourceRockLithoIoTbl");
		}
		else
			throw ErrorHandler::Exception(ErrorHandler::ValidationError) << "Something went wrong in cleaning SourceRockLithoIoTbl";

		return;
	}
	else
	{
		// Check if the HI_mixing ranges remain in valid ranges after resetting to default SRs for mixing cases
		if (MixingOn) {
			auto theFaildSrIdInStratIo = CheckValidHiRangesForMixedSRs(srLayersFromStratIoTbl, theNameHiList);
			//(*3) Hi ranges that fail for mixing cases after user mixed SRs are reset to default Srs in BPA2
			for (auto aFailedSr : theFaildSrIdInStratIo) {
				listOfSrInSrLithoIoTblThatMustExist.erase(
					std::remove_if(	listOfSrInSrLithoIoTblThatMustExist.begin(), 
									listOfSrInSrLithoIoTblThatMustExist.end(),
									[&](pair<size_t, bool>const & element) {
										bool is = (element.first == aFailedSr);
										return is;
									}
								   )
							);
			}
		}
		// clear up strayed SR entries from SourceRockLithoIoTbl that was left behind
		//
		auto err = UpdateOfInconsistentEntriesInSrLithoIoTbl(listOfSrInSrLithoIoTblThatMustExist, &modelConverter);
		if (err != ErrorHandler::NoError) {
			throw ErrorHandler::Exception(ErrorHandler::ValidationError) << "Something went wrong in cleaning SourceRockLithoIoTbl";
		}
		
		// 2. Change SourceRockLithoIoTbl 
		if (SetSourceRockPropertiesForBPA2(&modelConverter, listOfSrInSrLithoIoTblThatMustExist, SnglSrsFromStratIoLayIdNamePairList)
			!= ErrorHandler::NoError) {
			throw ErrorHandler::Exception(ErrorHandler::ValidationError) << "Something went wrong in updating SourceRockLithoIoTbl";
		}
		// ============================== SourceRockLithoIoTbl updated =================================== //

	}

	// \Removing unnecessary map references related to SourceRockLithoIoTbl from GridMapIoTbl 
	
	// Collect remaining GridMap names in SourceRockLithoIoTbl
	std::vector<std::string> srLithoIoTblMaps; // vector containing the list of maps in SourceRockLithoIoTbl; 
	database::Table* srLithoiotbl = m_ph->getTable("SourceRockLithoIoTbl");
	for (size_t id = 0; id < srLithoiotbl->size(); ++id)
	{
		database::Record* rec = srLithoiotbl->getRecord(static_cast<int> (id));
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
			if(srLithoIoTblMaps.size() == 0) // this is when the entire SourceRockLithoIoTbl is cleared
				Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap("SourceRockLithoIoTbl");
			for (int i = 0; i < srLithoIoTblMaps.size(); ++i)
			{
				if (mapNameFromIoTbl == srLithoIoTblMaps[i]) break;
				else if (i == srLithoIoTblMaps.size() - 1)
				{
					Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap("SourceRockLithoIoTbl", mapNameFromIoTbl);
					// this creates a list of maps to be cleared when GidMapIoTbl upgrade Update method is called 
				}
			}
		}
	}

	// Identifying the unnecessary map references in GridMapIoTbl ReferredBy StratIoTbl for inactive SR - SR mixing Grid maps
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
				Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap("StratIoTbl", srMixHIGrid);

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
				Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap("StratIoTbl", srMixHCGrid);

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
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> Updating Source Rocks names and properties in SourceRockLithoIoTbl and StratIoTbl";
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
		auto bpaHiIni = std::floor(m_model.sourceRockManager().hiIni(sourceRockId));
		auto bpaHcIni = m_model.sourceRockManager().hcIni(sourceRockId);
		auto bpaBaseSourceRockType = m_model.sourceRockManager().baseSourceRockType(sourceRockId);
		// return data
		std::string bpa2SourceRockTypeName, bpa2BaseSourceRockType; bool litFlag{};

		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* For the Layer Name : '" << layerName << "',";

		upgradeToBPA2Names(mConvert, bpaSourceRockTypeName,
			bpaBaseSourceRockType, bpa2SourceRockTypeName, bpa2BaseSourceRockType, litFlag, bpaScVre05);
		(*mConvert).upgradeDiffusionEnergy(bpaBaseSourceRockType, bpaScVre05, bpaAsphalteneDE,
			bpaResinDE, bpaC15AroDE, bpaC15SatDE);
		(*mConvert).upgradeVESlimit(legacyVESlimitIndicator, VESlimit);
		(*mConvert).upgradeVREthreshold(legacyVREoptimization, VREthreshold);
		//change mixed SRs to default in SourceRockLithoIoTbl
		if (srRecord.second) {
			double HiRange[2]{};
			auto newVal = mConvert->upgradeEa(bpaSourceRockTypeName, bpaBaseSourceRockType, legacyEa, bpaScVre05);
			err = m_model.sourceRockManager().setPreAsphActEnergy(sourceRockId, newVal);
			newVal = mConvert->upgradeHcVre05(bpaSourceRockTypeName, bpaBaseSourceRockType, bpaHcIni, bpaScVre05, HiRange[0], HiRange[1]);
			err = m_model.sourceRockManager().setHCIni(sourceRockId, newVal);
			newVal = mConvert->upgradeScVre05(bpaSourceRockTypeName, bpaBaseSourceRockType, bpaScVre05);
			err = m_model.sourceRockManager().setSCIni(sourceRockId, newVal);
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> SourceRock " << bpa2SourceRockTypeName
				<< " used in a mixed source rock layer "
				<< layerName << " is reset to default properties";
		}
#if HiAnamoly		// This was a temporary solution solution and now not needed as this has been now taken care in the middle tier in PBI #.
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
	const std::string& bpa1SR, const std::string& bpa1BSR,
	std::string& bpa2SR, std::string& bpa2BSR,
	bool& isLit, bool& isSulfur)
{
	(*mConvert).upgradeSourceRockName(bpa1SR, bpa1BSR, isSulfur, bpa2SR, bpa2BSR, isLit);
}


Prograde::bpa2nameHiPair Prograde::SourceRockUpgradeManager::GetBpa2SourceRockNamesFromBpaNamesOfThisLayer(const std::string& bpaSrName,
	const size_t& LayIdFromStraIOTbl, size_t& srIdFromSrLitoIoTbl, const Prograde::SourceRockConverter* mConvert, bool isMixingEnable)
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
			std::pair < std::string, std::string> theSrNames(Utilities::Numerical::NoDataStringValue, Utilities::Numerical::NoDataStringValue);// BaseSR,SR name
			auto theBpa2Hi = Utilities::Numerical::IbsNoDataValue;
			if (mConvert->isSrFromLiterature(bsr, &bpaSrName) && isMixingEnable) {
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> Literature source is identified in source rock mixing in Layer '"
					<< LyrNameStarIo << "' and BPA2 does not allow literature source rocks in source rock mixing'; Migration from BPA to BPA2-Basin changed this layer as a non-source rock layer...";
				upgradeAsNormalLayer(LayIdFromStraIOTbl);

			}
			else {
				theBpa2Hi = mConvert->GetmeasuredHI(m_model.sourceRockManager().baseSourceRockType(srIdFromSrLitoIoTbl), &bpaSrName);
				theSrNames = std::make_pair(
					mConvert->GetBPA2BaseRockName(m_model.sourceRockManager().baseSourceRockType(srIdFromSrLitoIoTbl),
						&bpaSrName),
					mConvert->GetBPA2RockName(m_model.sourceRockManager().baseSourceRockType(srIdFromSrLitoIoTbl),
						&bpaSrName));
			}

			return std::make_pair(theSrNames, make_pair( theBpa2Hi, make_pair(srIdFromSrLitoIoTbl, LayIdFromStraIOTbl)));
		}
	}

	return std::make_pair(std::make_pair(std::string("Something is fishy BaseSr!"), std::string("Something is fishy Sr!")), make_pair(Utilities::Numerical::IbsNoDataValue, make_pair(Utilities::Numerical::IbsNoDataValue, Utilities::Numerical::IbsNoDataValue)));
}

std::vector<size_t> Prograde::SourceRockUpgradeManager::CheckValidHiRangesForMixedSRs(const std::vector<size_t>& theActiveSrs,
	std::vector < bpa2nameHiPair>& SrNameHiList)
{
	std::vector<size_t> thefailedSrList;
	for (auto stratIoSRs : theActiveSrs)
	{
		auto enbl = m_model.stratigraphyManager().isSourceRockMixingEnabled(stratIoSRs);
		auto layName = m_model.stratigraphyManager().layerName(stratIoSRs);
		if (enbl) {
			auto SRs = m_model.stratigraphyManager().sourceRockTypeName(stratIoSRs);
			// \ Only for Scalar Hi entries
			if (!m_model.tableValueAsString("StratIoTbl", stratIoSRs, "SourceRockMixingHIGrid").compare("") && !SRs.empty())
			{
				auto mixHi = m_model.stratigraphyManager().sourceRockMixHI(stratIoSRs);
				double arr[2] = { 0,0 };

				auto it = std::find_if(SrNameHiList.begin(), SrNameHiList.end(),
					[&](Prograde::bpa2nameHiPair& element) ->bool {
					auto is = (element.first.second == SRs[0]);
					if (is)arr[0] = element.second.first;
					return is;
				});

				auto it2 = std::find_if(SrNameHiList.begin(), SrNameHiList.end(),
					[&](Prograde::bpa2nameHiPair& element) ->bool {
					auto is = (element.first.second == SRs[1]);
					if (is)arr[1] = element.second.first;
					return is;
				});
				int n = sizeof(arr) / sizeof(arr[0]);
				std::sort(arr, arr + n);
				if (it != SrNameHiList.end() && it2 != SrNameHiList.end()) {
					if (!NumericFunctions::inRange(mixHi, arr[0], arr[1])) {
						LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) <<
							"<Basin-Warning> Target Source Rock Mixing HI  in Layer '" << layName << "' is: " << mixHi <<

							" which is out of allowed range (" << arr[0] << "," << arr[1] <<

							") for source rock mixing" << "in BPA2-Basin; Migration from BPA to BPA2-Basin changed this layer as a non-source rock layer...";
						upgradeAsNormalLayer(stratIoSRs);
						auto itt = std::find_if(SrNameHiList.begin(), SrNameHiList.end(),
							[&](Prograde::bpa2nameHiPair& element) ->bool {
								auto is = (element.first.second == SRs[0] && element.second.second.second == stratIoSRs);
								if (is)thefailedSrList.push_back(element.second.second.first);
								return is;
							});

						auto itt2 = std::find_if(SrNameHiList.begin(), SrNameHiList.end(),
							[&](Prograde::bpa2nameHiPair& element) ->bool {
								auto is = (element.first.second == SRs[1] && element.second.second.second == stratIoSRs);
								if (is)thefailedSrList.push_back(element.second.second.first);
								return is;
							});
					}
				}
			}
			else {
				m_model.stratigraphyManager().setSourceRockMixHI(stratIoSRs, Utilities::Numerical::IbsNoDataValue);
			}
		}
	}
	return thefailedSrList;
}


ErrorHandler::ReturnCode Prograde::SourceRockUpgradeManager::UpdateOfInconsistentEntriesInSrLithoIoTbl(theValidSrMixList& ValidSrIds,
	const Prograde::SourceRockConverter* mConvert)
{
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Updating SourceRockLithoIoTbl - removing the invalid source rocks";
	ErrorHandler::ReturnCode err = ErrorHandler::NoError;
	// SourceRocks from SrLithIo
	auto SrLithIoSrIds = m_model.sourceRockManager().sourceRockIDs();
	int countOfDelResId = 0; int originalResIDPosition = -1;
	int renumerSrId = 0;

	std::for_each(SrLithIoSrIds.begin(), SrLithIoSrIds.end(),
		[&](size_t & SrId) {
		originalResIDPosition++;
		auto it = std::find_if(ValidSrIds.begin(), ValidSrIds.end(),
			[&](pair<size_t, bool>& element) {
			bool is = (element.first == SrId);
			return is;
		});

		if (it == ValidSrIds.end()) {
			// not found in "Exists" list
			countOfDelResId++;

			auto idRem = (originalResIDPosition + 1) - countOfDelResId;

			if (m_model.tableSize("SourceRockLithoIoTbl") != 0) {
				auto srRem = m_model.sourceRockManager().sourceRockType(idRem);
				auto layername = m_model.sourceRockManager().layerName(idRem);
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> For the layer '" << layername << "', invalid SourceRock '" << srRem << "' with the following properties is removed";
				PrintInvalidSrProperties(idRem);
				err = m_model.removeRecordFromTable("SourceRockLithoIoTbl", idRem);
			}
		}
		else {
			// Re-order the srId after deleting SourceRockLithoIoTbl rows
			(*it).first = renumerSrId; renumerSrId++;
		}
	}
	);
	return err;
}

ErrorHandler::ReturnCode Prograde::SourceRockUpgradeManager::upgradeAsNormalLayer(size_t LayId)
{
	std::vector<std::string> SRinStratIo(2, "");
	auto err = m_model.stratigraphyManager().setSourceRockTypeName(LayId, SRinStratIo);
	
	err = m_model.stratigraphyManager().setSourceRockMixingEnabled(LayId, false);
	// the above should be the sequence of calls to update SourceRockTypeNames and RockMixingEnabled
	// see implementation of both the methods

	err = clearMixingParams(LayId);

	SRinStratIo.clear();// this is a trick to clear the "SourceRock" flag
	err = m_model.stratigraphyManager().setSourceRockTypeName(LayId, SRinStratIo);

	return err;
}

ErrorHandler::ReturnCode Prograde::SourceRockUpgradeManager::clearMixingParams(size_t LayId)
{
	auto err = m_model.setTableValue("StratIoTbl", LayId, "SourceRockMixingHI", DataAccess::Interface::DefaultUndefinedScalarValue);// Changed as per the BPA2 standard for no SR layer

	err = m_model.setTableValue("StratIoTbl", LayId, "SourceRockMixingHIGrid", "");

	err = m_model.setTableValue("StratIoTbl", LayId, "SourceRockMixingHC", 0.);

	err = m_model.setTableValue("StratIoTbl", LayId, "SourceRockMixingHCGrid", "");
	return err;
}

ErrorHandler::ReturnCode Prograde::SourceRockUpgradeManager::PrintInvalidSrProperties(size_t srIdFromSrLithoIoTbl)
{
	std::vector<std::string> head = {
		"TocIni/TocIniGrid",
		"HcVRe05",
		"HiVRe05(approx)",
		"PreAsphaltStartAct",
		"ScVRe05",
		"AsphalteneDiffusionEnergy",
		"ResinDiffusionEnergy",
		"C15AroDiffusionEnergy",
		"C15SatDiffusionEnergy",
		"VREoptimization",
		"VREthreshold",
		"VESLimitIndicator",
		"VESLimit"
	};
	std::vector<int> widths;
	std::for_each(head.begin(), head.end(),
		[&](std::string const &  s ) {
			widths.push_back(5 * (s.length() / 5 + 1));
		}
	);

	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << 
		std::setw(widths[0]) << head[0] << 
		std::setw(widths[1]) << head[1] <<
		std::setw(widths[2]) << head[2] <<
		std::setw(widths[3]) << head[3] <<
		std::setw(widths[4]) << head[4] <<
		std::setw(widths[5]) << head[5] <<
		std::setw(widths[6]) << head[6] <<
		std::setw(widths[7]) << head[7] <<
		std::setw(widths[8]) << head[8] <<
		std::setw(widths[9]) << head[9] <<
		std::setw(widths[10]) << head[10] <<
		std::setw(widths[11]) << head[11] <<
		std::setw(widths[12]) << head[12] <<'\n';




	auto tocIniMap = m_model.sourceRockManager().tocInitMapName(srIdFromSrLithoIoTbl);

	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) <<
		std::setw(widths[0]) << (!tocIniMap.compare("") ?  to_string(m_model.sourceRockManager().tocIni(srIdFromSrLithoIoTbl)) : tocIniMap) <<
		std::setw(widths[1]) << m_model.sourceRockManager().hcIni(srIdFromSrLithoIoTbl) <<
		std::setw(widths[2]) << std::floor(m_model.sourceRockManager().hiIni(srIdFromSrLithoIoTbl)) <<
		std::setw(widths[3]) << m_model.sourceRockManager().preAsphActEnergy(srIdFromSrLithoIoTbl) <<
		std::setw(widths[4]) << m_model.sourceRockManager().scIni(srIdFromSrLithoIoTbl) <<
		std::setw(widths[5]) << m_model.sourceRockManager().getAsphalteneDiffusionEnergy(srIdFromSrLithoIoTbl) <<
		std::setw(widths[6]) << m_model.sourceRockManager().getResinDiffusionEnergy(srIdFromSrLithoIoTbl) <<
		std::setw(widths[7]) << m_model.sourceRockManager().getC15AroDiffusionEnergy(srIdFromSrLithoIoTbl) <<
		std::setw(widths[8]) << m_model.sourceRockManager().getC15SatDiffusionEnergy(srIdFromSrLithoIoTbl) <<
		std::setw(widths[9]) << m_model.sourceRockManager().getVREoptimization(srIdFromSrLithoIoTbl) <<
		std::setw(widths[10]) << m_model.sourceRockManager().getVREthreshold(srIdFromSrLithoIoTbl) <<
		std::setw(widths[11]) << m_model.sourceRockManager().getVESlimitIndicator(srIdFromSrLithoIoTbl) <<
		std::setw(widths[12]) << m_model.sourceRockManager().getVESlimit(srIdFromSrLithoIoTbl) << '\n';
	// Will have to figure out How to use the error code here, hence retaining it for now
	return ErrorHandler::ReturnCode(ErrorHandler::NoError);
}
