//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "LithologyUpgradeManager.h"
#include "LithologyConverter.h"
#include <assert.h>

//utilities
#include "LogHandler.h"

//cmbAPI
#include "cmbAPI.h"
#include "LithologyManager.h"
#include "StratigraphyManager.h"


//DataAccess
#include "ProjectHandle.h"
#include "RunParameters.h"

#include <algorithm>

using namespace mbapi;

//------------------------------------------------------------//

Prograde::LithologyUpgradeManager::LithologyUpgradeManager(Model& model) :
	IUpgradeManager("lithology upgrade manager"), m_model(model)
{
	const auto ph = m_model.projectHandle();
	if (ph == nullptr) {
		throw std::invalid_argument(getName() + " cannot retreive the project handle from Cauldron data model");
	}
	m_ph = ph;
}

//------------------------------------------------------------//

void Prograde::LithologyUpgradeManager::upgrade() {
	Prograde::LithologyConverter modelConverter;
	
	std::vector<std::string> lithoNamesLst, newlithoNamesLst;
	std::vector<double>      lithoPerct;
	std::vector<std::string> percMaps;
	std::string legacyLithoTypeName, bpa2LithoName;

	database::Table * litho_table = m_ph->getTable("LithotypeIoTbl");

	size_t table_size1 = litho_table->size();
	
	//Deleting the litholytes ("Astheno. Mantle" and "Std. Basalt") if they are not referred in the StratIoTbl. Also clearing the reference from LitThCondIoTbl and LitHeatCapIoTbl
	for (size_t LithId = 0; LithId < litho_table->size(); ++LithId)
	{
		
		std::string lithoName = m_model.lithologyManager().lithologyName(LithId);
		//count++;
		
		if (lithoName == "Astheno. Mantle" || lithoName == "Std. Basalt")
		{
			m_model.lithologyManager().deleteLithology(LithId);
            
		}
		database::Table * litho_table1 = m_ph->getTable("LithotypeIoTbl");
		size_t table_size2 = litho_table1->size();
		if (table_size2 != table_size1)
		{	
			LithId--;
			table_size1 = table_size2;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << lithoName << " is not referred in StratIoTbl for the scenario";
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Deleted the row from LithotypeIoTbl. Also removed the referrences from LitThCondIoTbl and LitHeatCapIoTbl";
		}
	}

	int lithologyFlag;
	std::string parentLithology;
	mbapi::LithologyManager::PorosityModel porModel; // porosity calculation model
	std::vector<double> porModelPrms, updatedPorModelPrms; // poro. model parameters, depends on the given model
	for (size_t lithoId = 0; lithoId < litho_table->size(); ++lithoId)
	{
		m_model.lithologyManager().getUserDefinedFlagForLithology(lithoId, lithologyFlag);
		m_model.lithologyManager().getReferenceLithology(lithoId, parentLithology);

		//Upgrading the lithotype names for BPA1 standard lithotypes
		legacyLithoTypeName = m_model.lithologyManager().lithologyName(lithoId);//get the lithotype 
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Lithotype encountered in the LithotypeIoTbl is '"<<legacyLithoTypeName<<"'";
		bpa2LithoName = modelConverter.upgradeLithologyName(legacyLithoTypeName);
		m_model.lithologyManager().setLithologyName(lithoId, bpa2LithoName);

		if (lithologyFlag == 0)
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << legacyLithoTypeName << " is a standard lithology. Lithology name is updated from '" << legacyLithoTypeName << "' to '" << bpa2LithoName << "'.";
		else
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << legacyLithoTypeName << " is a user defined lithology. No upgrade is done for the lithology name.";
		
		//The porosity model upgradation needs further modifications as the requirement got changed during its implementation which will be taken in the next sprint......hence commented for the time being
#if 0 
		//Upgrading the porosity model for user defined lithotypes of BPA1
		m_model.lithologyManager().porosityModel(lithoId, porModel, porModelPrms); 
		updatedPorModelPrms.clear();
		modelConverter.computeSingleExpModelParameters(parentLithology, lithologyFlag, porModel, porModelPrms, updatedPorModelPrms);
		m_model.lithologyManager().setPorosityModel(lithoId, porModel, updatedPorModelPrms);
#endif
	}
	//updating the lithology names for the Lithotype1, Lithotype2 and Lithotype3 of stratIoTbl 
	database::Table * strat_table = m_ph->getTable("StratIoTbl");
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the lithotype names in StratIoTbl";
	for (size_t stratId = 0; stratId < strat_table->size(); ++stratId)
	{
		newlithoNamesLst.clear();
		m_model.stratigraphyManager().layerLithologiesList(stratId, lithoNamesLst, lithoPerct, percMaps);
		for (size_t temp = 0; temp<lithoNamesLst.size(); temp++)
		{
			legacyLithoTypeName = lithoNamesLst[temp];
			bpa2LithoName = modelConverter.upgradeLithologyName(legacyLithoTypeName);
			newlithoNamesLst.push_back(bpa2LithoName);
		}
		m_model.stratigraphyManager().setLayerLithologiesList(stratId, newlithoNamesLst, lithoPerct);
	}
	//updating the lithology names in AllochthonLithoIoTbl 
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the lithotype names in AllochthonLithoIoTbl";
	database::Table * alochthon_table = m_ph->getTable("AllochthonLithoIoTbl");
	for (size_t alocthonId = 0; alocthonId < alochthon_table->size(); ++alocthonId)
	{
		legacyLithoTypeName = m_model.lithologyManager().allochtonLithology(alocthonId);
		bpa2LithoName = modelConverter.upgradeLithologyName(legacyLithoTypeName);
		m_model.lithologyManager().setAllochtonLithology(alocthonId, bpa2LithoName);
	}
	//updating the lithology names in LitThCondIoTbl
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the lithotype names in LitThCondIoTbl";
	database::Table * lithoThCond_table = m_ph->getTable("LitThCondIoTbl");
	 
	for (size_t lithThCondId = 0; lithThCondId < lithoThCond_table->size(); ++lithThCondId)
	{
		m_model.lithologyManager().getThermCondTableLithoName(lithThCondId, legacyLithoTypeName);
		bpa2LithoName = modelConverter.upgradeLithologyName(legacyLithoTypeName);
		m_model.lithologyManager().setThermCondTableLithoName(lithThCondId, bpa2LithoName);
	}
	//updating the lithology names in LitHeatCapIoTbl
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the lithotype names in LitHeatCapIoTbl";
	database::Table * lithoHeatCap_table = m_ph->getTable("LitHeatCapIoTbl");
	for (size_t htCaphId = 0; htCaphId < lithoHeatCap_table->size(); ++htCaphId)
	{
		m_model.lithologyManager().getHeatCapTableLithoName(htCaphId, legacyLithoTypeName);
		bpa2LithoName = modelConverter.upgradeLithologyName(legacyLithoTypeName);
		m_model.lithologyManager().setHeatCapTableLithoName(htCaphId, bpa2LithoName);
	}	
	//updating the lithology names in PressureFaultcutIoTbl
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the lithotype names in PressureFaultcutIoTbl";
	database::Table * fltCut_table = m_ph->getTable("PressureFaultcutIoTbl");
	for (size_t Id = 0; Id < fltCut_table->size(); ++Id)
	{
		m_model.faultcutManager().getFaultLithoName(Id, legacyLithoTypeName);
		bpa2LithoName = modelConverter.upgradeLithologyName(legacyLithoTypeName);
		m_model.faultcutManager().setFaultLithoName(Id, bpa2LithoName);
	}
	
}

