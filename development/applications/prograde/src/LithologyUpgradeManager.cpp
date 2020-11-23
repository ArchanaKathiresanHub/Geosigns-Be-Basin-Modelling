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
#include "NumericFunctions.h"

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
		throw std::invalid_argument(getName() + " cannot retrieve the project handle from Cauldron data model");
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
	
	//Deleting the lithotypes ("Astheno. Mantle" and "Std. Basalt") if they are not referred in the StratIoTbl. Also clearing the reference from LitThCondIoTbl and LitHeatCapIoTbl
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
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> "<<lithoName << " is not referred in StratIoTbl for the scenario";
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> Deleted the row from LithotypeIoTbl. Also removed the referrences from LitThCondIoTbl and LitHeatCapIoTbl";
		}
	}

	int lithologyFlag, numPts;
	std::string parentLithologyDetails, legacyParentLithoName, bpa2ParentLithoName, legacyDefinitionDate, legacyLastChangedBy, legacyLastChangedDate, permModelName;
	std::string legacyLithoTypeName, bpa2LithoName, legacyLithoDescription, updatedDescription;
	mbapi::LithologyManager::PorosityModel porModel; // porosity calculation model
	std::vector<double> porModelPrms, updatedPorModelPrms; // to store poro. model parameters, depends on the given model
	mbapi::LithologyManager::PermeabilityModel prmModel; // permeability calculation model
	std::vector<double> mpPor;     // for multi-point perm. model the porosity values vector
	std::vector<double> mpPerm;    // for multi-point perm. model the log. of perm values vector
   double litPropValue;
   std::string lithoDescription;  //Lithology description
	for (size_t lithoId = 0; lithoId < litho_table->size(); ++lithoId)
	{
		m_model.lithologyManager().getUserDefinedFlagForLithology(lithoId, lithologyFlag);
		m_model.lithologyManager().getReferenceLithology(lithoId, parentLithologyDetails);
		legacyLithoTypeName = m_model.lithologyManager().lithologyName(lithoId);//get the lithotype name
		legacyLithoDescription = m_model.tableValueAsString("LithotypeIoTbl", lithoId, "Description");// get legacy description for lith id 
		legacyDefinitionDate = m_model.tableValueAsString("LithotypeIoTbl", lithoId, "DefinitionDate");// get legacy date of definition for the selected lithology 
		legacyLastChangedBy = m_model.tableValueAsString("LithotypeIoTbl", lithoId, "LastChangedBy");// get legacy user name who has made changes to the selected lithology
		legacyLastChangedDate = m_model.tableValueAsString("LithotypeIoTbl", lithoId, "LastChangedDate");// get legacy date when the last changed has made by the user to the selected lithology
		
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Lithotype encountered in the LithotypeIoTbl is '" << legacyLithoTypeName << "'";
		
		//......................................UPGRADING LITHOLOGY NAME .......................................//	
		//Upgrading the lithotype names for BPA1 standard lithotypes
		
		if (lithologyFlag == 0)
		{
			bpa2LithoName = modelConverter.upgradeLithologyName(legacyLithoTypeName);
			m_model.lithologyManager().setLithologyName(lithoId, bpa2LithoName);//Upgrading the lithology names in LithotypeIoTbl
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Info> Lithology Name: '" << legacyLithoTypeName << "' is a BPA standard lithology. Lithology name is updated from '" << legacyLithoTypeName << "' to '" << bpa2LithoName << "'.";

			if (bpa2LithoName.compare("Mantle") == 0)
			{
				std::string LithoName_Mantle = m_model.tableValueAsString("BasementIoTbl", 0, "MantleLithoName");
				if (LithoName_Mantle.compare("Mantle") != 0)
				{
					m_model.setTableValue("BasementIoTbl", 0, "MantleLithoName", "Mantle");
					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Info> Updated the 'MantleLithoName' field of BasementIoTbl from '" << legacyLithoTypeName << "' to 'Mantle'.";
				}
			}
			
		}
		else {
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Info> Lithology Name: '" << legacyLithoTypeName << "' is a BPA userDefined lithology. No upgrade to the lithology name is needed.";
		}
		//.............................Updating the parent lithology name in the "DefinedBy" field of lithotypeIoTbl..........................//
		legacyParentLithoName = modelConverter.findParentLithology(parentLithologyDetails);

		if (legacyParentLithoName != "")
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Info> Parent Lithology Name:  legacy name of Parent lithology is '" << legacyParentLithoName << "'";
		else //If the parentlithology name is not available in the legacy p3d file, then try to get it from the available mapping of lithology description or lithology name
		{
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Info> Parent Lithology Name: Not found in the legacy p3d file; try to find out from the mapping sheet based on Description or Lithology name";
			legacyParentLithoName = modelConverter.findMissingParentLithology(legacyLithoTypeName, legacyLithoDescription);
			if (legacyParentLithoName == "")
			{
				//check the DefinitionDate...the cut-off date is 30th April 2020....If the DefinitonDate is older than the cut-off date, then abort the migration
				bool flag = modelConverter.isDefinedBeforeThanCutOffDate(legacyDefinitionDate);
				if (!flag)
				{
					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Error> Lithology created after the cut-off date of 30th April 2020; Migration from BPA to BPA2 Basin Aborted...";
					exit(41);
				}
				else {
					throw ErrorHandler::Exception(ErrorHandler::NonexistingID) << "<Basin-Info> Not found parent lithology for lithotype '"<< legacyLithoTypeName <<"'...crosscheck the description and/or the date format";
				}
			}
			else
			{
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Info> Parent Lithology Name is derived as '"<< legacyParentLithoName<<"'";
			}
		}	

		bpa2ParentLithoName = modelConverter.upgradeLithologyName(legacyParentLithoName);
		m_model.setTableValue("LithotypeIoTbl", lithoId, "DefinedBy", bpa2ParentLithoName);
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Info> DefinedBy: Updated from '" << parentLithologyDetails << "' to '" << bpa2ParentLithoName << "'.";
				
		//........................Checking and updating the Lithology description.....................
		updatedDescription = modelConverter.upgradeLithologyDescription(legacyLithoDescription, lithologyFlag, legacyParentLithoName);
		m_model.setTableValue("LithotypeIoTbl", lithoId, "Description", updatedDescription);//Upgrading the description in LithotypeIoTbl

		//...........................upgrading the audit details for the lithotypes.......................
		modelConverter.upgradeLithologyAuditInfo(legacyDefinitionDate, legacyLastChangedBy, legacyLastChangedDate, lithologyFlag);
		m_model.setTableValue("LithotypeIoTbl", lithoId, "DefinitionDate", legacyDefinitionDate);
		m_model.setTableValue("LithotypeIoTbl", lithoId, "LastChangedBy", legacyLastChangedBy);
		m_model.setTableValue("LithotypeIoTbl", lithoId, "LastChangedDate", legacyLastChangedDate);
	
		//Check and update lithotype property values are in proposed range in LithotypeIoTbl field
	    //Density
		litPropValue = m_model.lithologyManager().getLitPropDensity(lithoId);
		if (!NumericFunctions::inRange(litPropValue, 500.0, 10000.0))
		{
			double updatedlitPropValue = NumericFunctions::clipValueToRange(litPropValue, 500.0, 10000.0);
			m_model.lithologyManager().setLitPropDensity(lithoId, updatedlitPropValue);
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Density value : " << litPropValue << " is changed to " << updatedlitPropValue << " as the value is not in acceptable limits [500,10000]";
		}
		//Heat Production
		litPropValue = m_model.lithologyManager().getLitPropHeatProduction(lithoId);
		if (!NumericFunctions::inRange(litPropValue, 0.0, 25.0))
		{
			double updatedlitPropValue = NumericFunctions::clipValueToRange(litPropValue, 0.0, 25.0);
			m_model.lithologyManager().setLitPropHeatProduction(lithoId, updatedlitPropValue);
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Heat Production value : " << litPropValue << " is changed to " << updatedlitPropValue << " as the value is not in acceptable limits [0,25]";
		}
		//Thermal Conductivity
		litPropValue = m_model.lithologyManager().getLitPropThrConductivity(lithoId);
		if (!NumericFunctions::inRange(litPropValue, 0.0, 10.0))
		{
			double updatedlitPropValue = NumericFunctions::clipValueToRange(litPropValue, 0.0, 10.0);
			m_model.lithologyManager().setLitPropThrConductivity(lithoId, updatedlitPropValue);
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Thermal conductivity value : " << litPropValue << " is changed to " << updatedlitPropValue << " as the value is not in acceptable limits [0,10]";
		}
		//Thermal Conductivity Anistropy
		litPropValue = m_model.lithologyManager().getLitPropThrCondAnistropy(lithoId);
		if (!NumericFunctions::inRange(litPropValue, 0.0, 10.0))
		{
			double updatedlitPropValue = NumericFunctions::clipValueToRange(litPropValue, 0.0, 10.0);
			m_model.lithologyManager().setLitPropThrCondAnistropy(lithoId, updatedlitPropValue);
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Thermal conductivity Anistropy value : " << litPropValue << " is changed to " << updatedlitPropValue << " as the value is not in acceptable limits [0,10]";
		}
		//Permeability Anistropy
		litPropValue = m_model.lithologyManager().getLitPropPermAnistropy(lithoId);
		if (!NumericFunctions::inRange(litPropValue, 0.0, 100.0))
		{
			double updatedlitPropValue = NumericFunctions::clipValueToRange(litPropValue, 0.0, 100.0);
			m_model.lithologyManager().setLitPropPermAnistropy(lithoId, updatedlitPropValue);
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Permeability Anistropy value : " << litPropValue << " is changed to " << updatedlitPropValue << " as the value is not in acceptable limits [0,100]";
		}
		//Seismic Velocity
		litPropValue = m_model.lithologyManager().getLitPropSeisVelocity(lithoId);
		if (!NumericFunctions::inRange(litPropValue, 1000.0, 9000.0))
		{
			double updatedlitPropValue = NumericFunctions::clipValueToRange(litPropValue, 1000.0, 9000.0);
			m_model.lithologyManager().setLitPropSeisVelocity(lithoId, updatedlitPropValue);
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Seismic Velocity value : " << litPropValue << " is changed to " << updatedlitPropValue << " as the value is not in acceptable limits [1000,9000]";
		}
		//Seismic Velocity Exponent
		litPropValue = m_model.lithologyManager().getLitPropSeisVeloExponent(lithoId);
		if (!NumericFunctions::inRange(litPropValue, -1.0, 1.0))
		{
			double updatedlitPropValue = NumericFunctions::clipValueToRange(litPropValue, -1.0, 1.0);
			m_model.lithologyManager().setLitPropSeisVeloExponent(lithoId, updatedlitPropValue);
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Seismic Velocity Exponent value : " << litPropValue << " is changed to " << updatedlitPropValue << " as the value is not in acceptable limits [-1,1]";
		}
		//Entry Pressure Coefficient 1
		litPropValue = m_model.lithologyManager().getLitPropEntryPresCoeff1(lithoId);
		if (!NumericFunctions::inRange(litPropValue, 0.1, 2.0))
		{
			double updatedlitPropValue = NumericFunctions::clipValueToRange(litPropValue, 0.1, 2.0);
			m_model.lithologyManager().setLitPropEntryPresCoeff1(lithoId, updatedlitPropValue);
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Entry Pressure Coefficient 1 value : " << litPropValue << " is changed to " << updatedlitPropValue << " as the value is not in acceptable limits [0.1,2]";
		}
		//Entry Pressure Coefficient 2
		litPropValue = m_model.lithologyManager().getLitPropEntryPresCoeff2(lithoId);
		if (!NumericFunctions::inRange(litPropValue, -1.5, 1.5))
		{
			double updatedlitPropValue = NumericFunctions::clipValueToRange(litPropValue, -1.5, 1.5);
			m_model.lithologyManager().setLitPropEntryPresCoeff2(lithoId, updatedlitPropValue);
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Entry Pressure Coefficient 2 value : " << litPropValue << " is changed to " << updatedlitPropValue << " as the value is not in acceptable limits [-1.5,1.5]";
		}
		//Hydraulic Fracturing
		litPropValue = m_model.lithologyManager().getLitPropHydFracturing(lithoId);
		if (!NumericFunctions::inRange(litPropValue, 0.0, 100.0))
		{
			double updatedlitPropValue = NumericFunctions::clipValueToRange(litPropValue, 0.0, 100.0);
			m_model.lithologyManager().setLitPropHydFracturing(lithoId, updatedlitPropValue);
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Hydraulic Fracturing value : " << litPropValue << " is changed to " << updatedlitPropValue << " as the value is not in acceptable limits [0.0,100.0]";
		}
		//ReferenceSolidViscosity
		litPropValue = m_model.lithologyManager().getLitPropRefSoldViscosity(lithoId);
		if (!NumericFunctions::inRange(litPropValue, 0.0, 1e18))
		{
			double updatedlitPropValue = NumericFunctions::clipValueToRange(litPropValue, 0.0, 1e18);
			m_model.lithologyManager().setLitPropRefSoldViscosity(lithoId, updatedlitPropValue);
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Reference Solid Viscosity value : " << litPropValue << " is changed to " << updatedlitPropValue << " as the value is not in acceptable limits [0,1e18]";
		}
		//Intrusion Temperature
		litPropValue = m_model.lithologyManager().getLitPropIntrTemperature(lithoId);
		if ((litPropValue != -9999) && (!NumericFunctions::inRange(litPropValue, 600.0, 1650.0)))
		{
			double updatedlitPropValue = NumericFunctions::clipValueToRange(litPropValue, 600.0, 1650.0);
			m_model.lithologyManager().setLitPropIntrTemperature(lithoId, updatedlitPropValue);
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original Hydraulic Fracturing value : " << litPropValue << " is changed to " << updatedlitPropValue << " as the value is not in acceptable limits [600.0,1650.0]";		
		}
	
		//......................................POROSITY MODEL UPGRADATION......................................//	
		//Upgrading the deprecated Soil Mechanics porosity model to Exponential model for user defined lithotypes of BPA1
		m_model.lithologyManager().porosityModel(lithoId, porModel, porModelPrms);
		updatedPorModelPrms.clear();
		updatedPorModelPrms = modelConverter.computeSingleExpModelParameters(legacyParentLithoName, lithologyFlag, porModel, porModelPrms);
		m_model.lithologyManager().setPorosityModel(lithoId, porModel, updatedPorModelPrms);
		m_model.setTableValue("LithotypeIoTbl", lithoId, "CompacCoefESA", Utilities::Numerical::IbsNoDataValue);
		m_model.setTableValue("LithotypeIoTbl", lithoId, "CompacCoefESB", Utilities::Numerical::IbsNoDataValue);
		m_model.setTableValue("LithotypeIoTbl", lithoId, "Compaction_Coefficient_SM", Utilities::Numerical::IbsNoDataValue);
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  " << "<Basin-Warning> Upgrading " << "CompacCoefESA, CompacCoefESB and Compaction_Coefficient_SM values to " << Utilities::Numerical::IbsNoDataValue;
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
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Info> Permeability Model: Legacy Sandstone permeability model is detected, upgrading to Multipoint.";
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> upgrading " << "DepoPerm, PermDecrStressCoef and PermIncrRelaxCoef values to " << Utilities::Numerical::IbsNoDataValue;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> upgrading Number_Of_Data_Points to " << numPts;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> upgrading " << "Multipoint_Porosity to " << mpPor[0] << " " << mpPor[1] << " and Multipoint_Permeability to " << mpPerm[0] << " " << mpPerm[1];
            break;
         }
         case mbapi::LithologyManager::PermMudstone:
         {
            modelConverter.upgradePermModelForUsrDefLitho("Shales", mpPor, mpPerm, numPts);
            m_model.lithologyManager().setPermeabilityModel(lithoId, prmModel, mpPor, mpPerm, numPts, lithologyFlag);
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Info> Permeability Model: Legacy Mudstone permeability model is detected, upgrading to 'Multipoint'.";
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> upgrading " << "DepoPerm, PermDecrStressCoef and PermIncrRelaxCoef values to " << Utilities::Numerical::IbsNoDataValue;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> upgrading Number_Of_Data_Points to " << numPts;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> upgrading " << "Multipoint_Porosity to " << mpPor[0] << " " << mpPor[1] << " and Multipoint_Permeability to " << mpPerm[0] << " " << mpPerm[1];
            break;
         }
         case mbapi::LithologyManager::PermNone:
         {
            modelConverter.upgradePermModelForUsrDefLitho("None", mpPor, mpPerm, numPts);
			double legacyPermAnisotropy = m_model.tableValueAsDouble("LithotypeIoTbl", lithoId, "PermAnisotropy");
            m_model.lithologyManager().setPermeabilityModel(lithoId, prmModel, mpPor, mpPerm, numPts, lithologyFlag);	
			double updatedPermAnisotropy = 1;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Info> Permeability Model: Legacy None permeability model is detected, upgrading to 'Multipoint'.";
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> upgrading " << "DepoPerm, PermDecrStressCoef and PermIncrRelaxCoef values to " << Utilities::Numerical::IbsNoDataValue;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> upgrading Number_Of_Data_Points to " << numPts;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> upgrading " << "Multipoint_Porosity to " << mpPor[0] << " " << mpPor[1] << " and Multipoint_Permeability to " << mpPerm[0] << " " << mpPerm[1];
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> upgrading PermAnisotropy value from "<< legacyPermAnisotropy <<" to " << updatedPermAnisotropy;
            break;
         }
         case mbapi::LithologyManager::PermImpermeable:
         {
            modelConverter.upgradePermModelForUsrDefLitho("Impermeable", mpPor, mpPerm, numPts);
			double legacyPermAnisotropy = m_model.tableValueAsDouble("LithotypeIoTbl", lithoId, "PermAnisotropy");
            m_model.lithologyManager().setPermeabilityModel(lithoId, prmModel, mpPor, mpPerm, numPts, lithologyFlag);
			double updatedPermAnisotropy = 1;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Info> Permeability Model: Legacy Impermeable permeability model is detected, upgrading to 'Multipoint'.";
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> upgrading " << "DepoPerm, PermDecrStressCoef and PermIncrRelaxCoef values to " << Utilities::Numerical::IbsNoDataValue;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> upgrading Number_Of_Data_Points to " << numPts;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> upgrading " << "Multipoint_Porosity to " << mpPor[0] << " " << mpPor[1] << " and Multipoint_Permeability to " << mpPerm[0] << " " << mpPerm[1];
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> upgrading PermAnisotropy value from " << legacyPermAnisotropy << " to " << updatedPermAnisotropy;
            break;
         }
         case mbapi::LithologyManager::PermMultipoint:
         {
			 LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Info> Permeability Model: Multipoint permeability model is detected. No upgrade is needed.";
			 LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> upgrading " << "DepoPerm, PermDecrStressCoef and PermIncrRelaxCoef values to " << Utilities::Numerical::IbsNoDataValue;
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
		 LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Info> Permeability Model: Upgrading the permeability model from '"<< permModelName <<"' to 'Multipoint' with the properties of BPA2 '"<< bpa2LithoName<<"'.";
		 LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> upgrading " << "DepoPerm, PermDecrStressCoef and PermIncrRelaxCoef values to " << Utilities::Numerical::IbsNoDataValue;
		 LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> upgrading Number_Of_Data_Points to " << numPts;
		 LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> upgrading " << "Multipoint_Porosity to " << mpPor[0] << " " << mpPor[1] << " and Multipoint_Permeability to " << mpPerm[0] << " " << mpPerm[1];
		 if(permModelName.compare("None") == 0 || permModelName.compare("Impermeable") == 0)
		 {
			 double legacyPermAnisotropy = m_model.tableValueAsDouble("LithotypeIoTbl", lithoId, "PermAnisotropy");
			 double updatedPermAnisotropy = 1;
			 m_model.setTableValue("LithotypeIoTbl", lithoId, "PermAnisotropy", updatedPermAnisotropy);//upgrading the Permeability Anisotropy value
			 LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> upgrading PermAnisotropy value from " << legacyPermAnisotropy << " to " << updatedPermAnisotropy;
		 }
		}
	}
	
//......................................UPGRADING LITHOLOGY inputs of stratIoTbl......................................//	
	database::Table * strat_table = m_ph->getTable("StratIoTbl");
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Updating lithotype inputs of StratIoTbl";
	mbapi::StratigraphyManager & stratMgrLocal = m_model.stratigraphyManager();
	for (size_t stratId = 0; stratId < strat_table->size()-1; ++stratId)
	{
		std::string stratLayerName = m_model.tableValueAsString("StratIoTbl", stratId, "LayerName");
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Info> For stratigraphylayer with LayerName: "<< stratLayerName;
		lithoNamesLst.clear();
		lithoPerct.clear();
		percMaps.clear();
		ErrorHandler::ReturnCode err = m_model.stratigraphyManager().layerLithologiesList(stratId, lithoNamesLst, lithoPerct, percMaps);
		if (err != ErrorHandler::ReturnCode::NoError){
			throw ErrorHandler::Exception(stratMgrLocal.errorCode()) << stratMgrLocal.errorMessage();
		}
		
		//Processing lithotype inputs of StratIoTbl before updating
		err = modelConverter.PreprocessLithofaciesInputOfStratIoTbl(lithoNamesLst, lithoPerct, percMaps);
		if (err != ErrorHandler::ReturnCode::NoError) {
			throw ErrorHandler::Exception(ErrorHandler::ReturnCode::ValidationError) << "Lithology inputs of stratigraphy layer : '"<< stratLayerName<<"' is not correct; No lithotype name is found for this layer";
		}
		err = m_model.stratigraphyManager().setLayerLithologiesList(stratId, lithoNamesLst, lithoPerct, percMaps);
		if (err != ErrorHandler::ReturnCode::NoError){
			throw ErrorHandler::Exception(stratMgrLocal.errorCode()) << stratMgrLocal.errorMessage();
		}
	}

//......................................UPGRADING LITHOLOGY NAME in AllochthonLithoIoTbl......................................//	
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Updated the names for the system defined lithotypes as per the mapping in AllochthonLithoIoTbl";
	database::Table * alochthon_table = m_ph->getTable("AllochthonLithoIoTbl");
	for (size_t alocthonId = 0; alocthonId < alochthon_table->size(); ++alocthonId)
	{
		legacyLithoTypeName = m_model.lithologyManager().allochtonLithology(alocthonId);
		bpa2LithoName = modelConverter.upgradeLithologyName(legacyLithoTypeName);
		m_model.lithologyManager().setAllochtonLithology(alocthonId, bpa2LithoName);
	}

//......................................UPGRADING LITHOLOGY NAME in LitThCondIoTbl......................................//	
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Updated the names for the system defined lithotypes as per the mapping in LitThCondIoTbl";
	database::Table * lithoThCond_table = m_ph->getTable("LitThCondIoTbl");
	 
	for (size_t lithThCondId = 0; lithThCondId < lithoThCond_table->size(); ++lithThCondId)
	{
		m_model.lithologyManager().getThermCondTableLithoName(lithThCondId, legacyLithoTypeName);
		bpa2LithoName = modelConverter.upgradeLithologyName(legacyLithoTypeName);
		m_model.lithologyManager().setThermCondTableLithoName(lithThCondId, bpa2LithoName);
	}

//......................................UPGRADING LITHOLOGY NAME in LitHeatCapIoTbl......................................//	
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Updated the names for the system defined lithotypes as per the mapping in LitHeatCapIoTbl";
	database::Table * lithoHeatCap_table = m_ph->getTable("LitHeatCapIoTbl");
	for (size_t htCaphId = 0; htCaphId < lithoHeatCap_table->size(); ++htCaphId)
	{
		m_model.lithologyManager().getHeatCapTableLithoName(htCaphId, legacyLithoTypeName);
		bpa2LithoName = modelConverter.upgradeLithologyName(legacyLithoTypeName);
		m_model.lithologyManager().setHeatCapTableLithoName(htCaphId, bpa2LithoName);
	}

//......................................UPGRADING LITHOLOGY NAME in PressureFaultcutIoTbl......................................//	
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Updated the names for the system defined lithotypes as per the mapping in PressureFaultcutIoTbl";
	database::Table * fltCut_table = m_ph->getTable("PressureFaultcutIoTbl");
	for (size_t Id = 0; Id < fltCut_table->size(); ++Id)
	{
		m_model.faultcutManager().getFaultLithoName(Id, legacyLithoTypeName);
		bpa2LithoName = modelConverter.upgradeLithologyName(legacyLithoTypeName);
		m_model.faultcutManager().setFaultLithoName(Id, bpa2LithoName);
	}
	
}

