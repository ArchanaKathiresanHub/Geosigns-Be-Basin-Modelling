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
	std::string parentLithologyDetails, legacyParentLithoName, bpa2ParentLithoName, legacyDefinitionDate, legacyLastChangedBy, legacyLastChangedDate, permModelName;
	std::string legacyLithoTypeName, bpa2LithoName, legacyLithoDescription, updatedDescription;
	mbapi::LithologyManager::PorosityModel porModel; // porosity calculation model
	std::vector<double> porModelPrms, updatedPorModelPrms; // to store poro. model parameters, depends on the given model
	mbapi::LithologyManager::PermeabilityModel prmModel; // permeability calculation model
	std::vector<double> mpPor;     // for multi-point perm. model the porosity values vector
	std::vector<double> mpPerm;    // for multi-point perm. model the log. of perm values vector
	int numPts;
	for (size_t lithoId = 0; lithoId < litho_table->size(); ++lithoId)
	{
		m_model.lithologyManager().getUserDefinedFlagForLithology(lithoId, lithologyFlag);
		m_model.lithologyManager().getReferenceLithology(lithoId, parentLithologyDetails);
		legacyLithoTypeName = m_model.lithologyManager().lithologyName(lithoId);//get the lithotype name
		legacyLithoDescription = m_model.tableValueAsString("LithotypeIoTbl", lithoId, "Description");// get legacy description for lith id 
		
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Lithotype encountered in the LithotypeIoTbl is '" << legacyLithoTypeName << "'";

		legacyParentLithoName = modelConverter.findParentLithology(parentLithologyDetails, legacyLithoDescription, lithologyFlag);
		
		//......................................UPGRADING LITHOLOGY NAME and DESCRIPTION.......................................//	
		//Upgrading the lithotype names for BPA1 standard lithotypes
		
		if (lithologyFlag == 0)
		{
			bpa2LithoName = modelConverter.upgradeLithologyName(legacyLithoTypeName);
			m_model.lithologyManager().setLithologyName(lithoId, bpa2LithoName);//Upgrading the lithology names in LithotypeIoTbl
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* Lithology Name: '" << legacyLithoTypeName << "' is a BPA standard lithology. Lithology name is updated from '" << legacyLithoTypeName << "' to '" << bpa2LithoName << "'.";

			if (bpa2LithoName.compare("Mantle") == 0)
			{
				std::string LithoName_Mantle = m_model.tableValueAsString("BasementIoTbl", 0, "MantleLithoName");
				if (LithoName_Mantle.compare("Mantle") != 0)
				{
					m_model.setTableValue("BasementIoTbl", 0, "MantleLithoName", "Mantle");
					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  Updated the 'MantleLithoName' field of BasementIoTbl from '" << legacyLithoTypeName << "' to 'Mantle'.";
				}
			}
			
		}
		//This is added just to check the legacy parent litho names. It further needs to be updated to the corresponding BPA2 litholigy Name
		bpa2ParentLithoName = modelConverter.upgradeLithologyName(legacyParentLithoName); 
		m_model.setTableValue("LithotypeIoTbl", lithoId, "DefinedBy", bpa2ParentLithoName);
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* DefinedBy: Updated from '" << parentLithologyDetails << "' to '" << bpa2ParentLithoName << "'.";
				
		//Checking and updating the Lithology description
		updatedDescription = modelConverter.upgradeLithologyDescription(legacyLithoDescription, lithologyFlag, legacyParentLithoName);
		m_model.setTableValue("LithotypeIoTbl", lithoId, "Description", updatedDescription);//Upgrading the description in LithotypeIoTbl

		//upgrading the audit details for the lithotypes
		legacyDefinitionDate = m_model.tableValueAsString("LithotypeIoTbl", lithoId, "DefinitionDate");// get legacy date of definition for the selected lithology 
		legacyLastChangedBy = m_model.tableValueAsString("LithotypeIoTbl", lithoId, "LastChangedBy");// get legacy user name who has made changes to the selected lithology
		legacyLastChangedDate = m_model.tableValueAsString("LithotypeIoTbl", lithoId, "LastChangedDate");// get legacy date when the last changed has made by the user to the selected lithology 
		modelConverter.upgradeLithologyAuditInfo(legacyDefinitionDate, legacyLastChangedBy, legacyLastChangedDate, lithologyFlag);
		m_model.setTableValue("LithotypeIoTbl", lithoId, "DefinitionDate", legacyDefinitionDate);
		m_model.setTableValue("LithotypeIoTbl", lithoId, "LastChangedBy", legacyLastChangedBy);
		m_model.setTableValue("LithotypeIoTbl", lithoId, "LastChangedDate", legacyLastChangedDate);
		
		//......................................POROSITY MODEL UPGRADATION......................................//	
		//Upgrading the deprecated Soil Mechanics porosity model to Exponential model for user defined lithotypes of BPA1
		m_model.lithologyManager().porosityModel(lithoId, porModel, porModelPrms);
		updatedPorModelPrms.clear();
		modelConverter.computeSingleExpModelParameters(legacyParentLithoName, lithologyFlag, porModel, porModelPrms, updatedPorModelPrms);
		m_model.lithologyManager().setPorosityModel(lithoId, porModel, updatedPorModelPrms);
		m_model.setTableValue("LithotypeIoTbl", lithoId, "CompacCoefESA", Utilities::Numerical::IbsNoDataValue);
		m_model.setTableValue("LithotypeIoTbl", lithoId, "CompacCoefESB", Utilities::Numerical::IbsNoDataValue);
		m_model.setTableValue("LithotypeIoTbl", lithoId, "Compaction_Coefficient_SM", Utilities::Numerical::IbsNoDataValue);
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  " << "Upgrading " << "CompacCoefESA, CompacCoefESB and Compaction_Coefficient_SM values to " << Utilities::Numerical::IbsNoDataValue;
		//......................................PERMEABILITY MODEL UPGRADATION......................................//	
		//Upgrading the permeability parameters for system and user defined lithotypes of BPA1
		m_model.lithologyManager().getPermeabilityModel(lithoId, prmModel, mpPor, mpPerm, numPts);
		if (lithologyFlag) // user defined lithology
		{
         switch (prmModel) {
         case mbapi::LithologyManager::PermSandstone:
         {
            modelConverter.upgradePermModelForUsrDefLitho("Sands", mpPor, mpPerm, numPts);
            m_model.lithologyManager().setPermeabilityModel(lithoId, prmModel, mpPor, mpPerm, numPts, lithologyFlag);
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* Permeability Model: Legacy Sandstone permeability model is detected, upgrading to Multipoint.";
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  upgrading " << "DepoPerm, PermDecrStressCoef and PermIncrRelaxCoef values to " << Utilities::Numerical::IbsNoDataValue;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  upgrading Number_Of_Data_Points to " << numPts;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  upgrading " << "Multipoint_Porosity to " << mpPor[0] << " " << mpPor[1] << " and Multipoint_Permeability to " << mpPerm[0] << " " << mpPerm[1];
            break;
         }
         case mbapi::LithologyManager::PermMudstone:
         {
            modelConverter.upgradePermModelForUsrDefLitho("Shales", mpPor, mpPerm, numPts);
            m_model.lithologyManager().setPermeabilityModel(lithoId, prmModel, mpPor, mpPerm, numPts, lithologyFlag);
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* Permeability Model: Legacy Mudstone permeability model is detected, upgrading to 'Multipoint'.";
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  upgrading " << "DepoPerm, PermDecrStressCoef and PermIncrRelaxCoef values to " << Utilities::Numerical::IbsNoDataValue;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  upgrading Number_Of_Data_Points to " << numPts;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  upgrading " << "Multipoint_Porosity to " << mpPor[0] << " " << mpPor[1] << " and Multipoint_Permeability to " << mpPerm[0] << " " << mpPerm[1];
            break;
         }
         case mbapi::LithologyManager::PermNone:
         {
            modelConverter.upgradePermModelForUsrDefLitho("None", mpPor, mpPerm, numPts);
			double legacyPermAnisotropy = m_model.tableValueAsDouble("LithotypeIoTbl", lithoId, "PermAnisotropy");
            m_model.lithologyManager().setPermeabilityModel(lithoId, prmModel, mpPor, mpPerm, numPts, lithologyFlag);	
			double updatedPermAnisotropy = 1;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* Permeability Model: Legacy None permeability model is detected, upgrading to 'Multipoint'.";
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  upgrading " << "DepoPerm, PermDecrStressCoef and PermIncrRelaxCoef values to " << Utilities::Numerical::IbsNoDataValue;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  upgrading Number_Of_Data_Points to " << numPts;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  upgrading " << "Multipoint_Porosity to " << mpPor[0] << " " << mpPor[1] << " and Multipoint_Permeability to " << mpPerm[0] << " " << mpPerm[1];
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  upgrading PermAnisotropy value from "<< legacyPermAnisotropy <<" to " << updatedPermAnisotropy;
            break;
         }
         case mbapi::LithologyManager::PermImpermeable:
         {
            modelConverter.upgradePermModelForUsrDefLitho("Impermeable", mpPor, mpPerm, numPts);
			double legacyPermAnisotropy = m_model.tableValueAsDouble("LithotypeIoTbl", lithoId, "PermAnisotropy");
            m_model.lithologyManager().setPermeabilityModel(lithoId, prmModel, mpPor, mpPerm, numPts, lithologyFlag);
			double updatedPermAnisotropy = 1;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* Permeability Model: Legacy Impermeable permeability model is detected, upgrading to 'Multipoint'.";
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  upgrading " << "DepoPerm, PermDecrStressCoef and PermIncrRelaxCoef values to " << Utilities::Numerical::IbsNoDataValue;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  upgrading Number_Of_Data_Points to " << numPts;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  upgrading " << "Multipoint_Porosity to " << mpPor[0] << " " << mpPor[1] << " and Multipoint_Permeability to " << mpPerm[0] << " " << mpPerm[1];
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  upgrading PermAnisotropy value from " << legacyPermAnisotropy << " to " << updatedPermAnisotropy;
            break;
         }
         case mbapi::LithologyManager::PermMultipoint:
         {
			 LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* Permeability Model: Multipoint permeability model is detected. No upgrade is needed.";
			 LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  upgrading " << "DepoPerm, PermDecrStressCoef and PermIncrRelaxCoef values to " << Utilities::Numerical::IbsNoDataValue;
            break;
         }
         case mbapi::LithologyManager::PermUnknown:
         default:
            break;
         }
		}
		else // Sytem defined Lithology
		{
         modelConverter.upgradePermModelForSysDefLitho(bpa2LithoName, mpPor, mpPerm, numPts);
		 string permModelName=m_model.tableValueAsString("LithotypeIoTbl", lithoId, "PermMixModel");// get legacy description for lith id 
         m_model.lithologyManager().setPermeabilityModel(lithoId, prmModel, mpPor, mpPerm, numPts, lithologyFlag);
		 LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* Permeability Model: Upgrading the permeability model from '"<< permModelName <<"' to 'Multipoint' with the properties of BPA2 '"<< bpa2LithoName<<"'.";
		 LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  upgrading " << "DepoPerm, PermDecrStressCoef and PermIncrRelaxCoef values to " << Utilities::Numerical::IbsNoDataValue;
		 LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  upgrading Number_Of_Data_Points to " << numPts;
		 LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  upgrading " << "Multipoint_Porosity to " << mpPor[0] << " " << mpPor[1] << " and Multipoint_Permeability to " << mpPerm[0] << " " << mpPerm[1];
		 if(permModelName.compare("None") == 0 || permModelName.compare("Impermeable") == 0)
		 {
			 double legacyPermAnisotropy = m_model.tableValueAsDouble("LithotypeIoTbl", lithoId, "PermAnisotropy");
			 double updatedPermAnisotropy = 1;
			 m_model.setTableValue("LithotypeIoTbl", lithoId, "PermAnisotropy", updatedPermAnisotropy);//upgrading the Permeability Anisotropy value
			 LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  upgrading PermAnisotropy value from " << legacyPermAnisotropy << " to " << updatedPermAnisotropy;
		 }
		}
	}
	
//......................................UPGRADING LITHOLOGY NAME in stratIoTbl......................................//	
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

//......................................UPGRADING LITHOLOGY NAME in AllochthonLithoIoTbl......................................//	
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the lithotype names in AllochthonLithoIoTbl";
	database::Table * alochthon_table = m_ph->getTable("AllochthonLithoIoTbl");
	for (size_t alocthonId = 0; alocthonId < alochthon_table->size(); ++alocthonId)
	{
		legacyLithoTypeName = m_model.lithologyManager().allochtonLithology(alocthonId);
		bpa2LithoName = modelConverter.upgradeLithologyName(legacyLithoTypeName);
		m_model.lithologyManager().setAllochtonLithology(alocthonId, bpa2LithoName);
	}

//......................................UPGRADING LITHOLOGY NAME in LitThCondIoTbl......................................//	
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the lithotype names in LitThCondIoTbl";
	database::Table * lithoThCond_table = m_ph->getTable("LitThCondIoTbl");
	 
	for (size_t lithThCondId = 0; lithThCondId < lithoThCond_table->size(); ++lithThCondId)
	{
		m_model.lithologyManager().getThermCondTableLithoName(lithThCondId, legacyLithoTypeName);
		bpa2LithoName = modelConverter.upgradeLithologyName(legacyLithoTypeName);
		m_model.lithologyManager().setThermCondTableLithoName(lithThCondId, bpa2LithoName);
	}

//......................................UPGRADING LITHOLOGY NAME in LitHeatCapIoTbl......................................//	
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the lithotype names in LitHeatCapIoTbl";
	database::Table * lithoHeatCap_table = m_ph->getTable("LitHeatCapIoTbl");
	for (size_t htCaphId = 0; htCaphId < lithoHeatCap_table->size(); ++htCaphId)
	{
		m_model.lithologyManager().getHeatCapTableLithoName(htCaphId, legacyLithoTypeName);
		bpa2LithoName = modelConverter.upgradeLithologyName(legacyLithoTypeName);
		m_model.lithologyManager().setHeatCapTableLithoName(htCaphId, bpa2LithoName);
	}

//......................................UPGRADING LITHOLOGY NAME in PressureFaultcutIoTbl......................................//	
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the lithotype names in PressureFaultcutIoTbl";
	database::Table * fltCut_table = m_ph->getTable("PressureFaultcutIoTbl");
	for (size_t Id = 0; Id < fltCut_table->size(); ++Id)
	{
		m_model.faultcutManager().getFaultLithoName(Id, legacyLithoTypeName);
		bpa2LithoName = modelConverter.upgradeLithologyName(legacyLithoTypeName);
		m_model.faultcutManager().setFaultLithoName(Id, bpa2LithoName);
	}
	
}

