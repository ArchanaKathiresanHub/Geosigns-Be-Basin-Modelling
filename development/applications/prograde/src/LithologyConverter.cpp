//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "LithologyConverter.h"

//utilities
#include "LogHandler.h"

//cmbAPI
#include "StratigraphyManager.h"
using namespace mbapi;
#include<map>

std::map<std::string, std::string> Prograde::LithologyConverter::createMapForLithoNames()
{
	std::map<std::string, std::string> m;
	m.insert(std::pair<std::string, std::string>("Diorite/Granodiorite","Diorite/Granodiorite (SI)"));
	m.insert(std::pair<std::string, std::string>("Gabbro/Dry basalt", "Dry Gabbro/Basalt (SI)"));
	m.insert(std::pair<std::string, std::string>("Gabbro/Wet basal", "Granite/Rhyolite (SI)"));
	m.insert(std::pair<std::string, std::string>("Granite/Rhyolite", "Wet Gabbro/Basalt (SI)"));
	m.insert(std::pair<std::string, std::string>("HEAT Chalk", "Chalk, white"));
	m.insert(std::pair<std::string, std::string>("Std. Chalk", "Chalk, white"));
	m.insert(std::pair<std::string, std::string>("HEAT Dolostone", "Grainstone, dolomitic, typical"));
	m.insert(std::pair<std::string, std::string>("HEAT Limestone", "Grainstone, calcitic, typical"));
	m.insert(std::pair<std::string, std::string>("HEAT Sandstone", "Sandstone, typical"));
	m.insert(std::pair<std::string, std::string>("Std. Sandstone", "Sandstone, typical"));
	m.insert(std::pair<std::string, std::string>("SM. Sandstone", "Sandstone, typical"));
	m.insert(std::pair<std::string, std::string>("SM.Mudst.40%Clay", "Mudstone, 40% clay"));
	m.insert(std::pair<std::string, std::string>("SM.Mudst.50%Clay", "Mudstone, 50% clay"));
	m.insert(std::pair<std::string, std::string>("SM.Mudst.60%Clay", "Mudstone, 60% clay"));
	m.insert(std::pair<std::string, std::string>("Std. Shale", "Mudstone, 60% clay"));
	m.insert(std::pair<std::string, std::string>("HEAT Shale", "Mudstone, 60% clay"));
	m.insert(std::pair<std::string, std::string>("Standard Ice", "Ice"));
	m.insert(std::pair<std::string, std::string>("Std. Anhydrite", "Anhydrite"));
	m.insert(std::pair<std::string, std::string>("Std. Basalt", "Basalt, typical"));
	m.insert(std::pair<std::string, std::string>("Std. Coal", "Coal"));
	m.insert(std::pair<std::string, std::string>("Std. Marl", "Marl"));
	m.insert(std::pair<std::string, std::string>("Std. Salt", "Halite"));
	m.insert(std::pair<std::string, std::string>("Std. Siltstone", "Siltstone, 20% clay"));
	m.insert(std::pair<std::string, std::string>("Std.Dolo.Grainstone", "Grainstone, dolomitic, typical"));
	m.insert(std::pair<std::string, std::string>("Std.Dolo.Mudstone", "Lime-Mudstone, dolomitic"));
	m.insert(std::pair<std::string, std::string>("Std.Grainstone", "Grainstone, calcitic, typical"));
	m.insert(std::pair<std::string, std::string>("Std.Lime Mudstone", "Lime-Mudstone"));
	m.insert(std::pair<std::string, std::string>("Sylvite", "Potash-Sylvite"));
	m.insert(std::pair<std::string, std::string>("Astheno. Mantle", "Mantle"));
	m.insert(std::pair<std::string, std::string>("Litho. Mantle", "Mantle"));

	return m;
}
std::map<std::string, std::string> Prograde::LithologyConverter::lithologyNameMaps = createMapForLithoNames();

std::string Prograde::LithologyConverter::upgradeLithologyName(const std::string & legacyLithotypeName)
{
	//std::string newLithoTypeNameAssigned;
	auto lithology = lithologyNameMaps.find(legacyLithotypeName);

	if (lithology != lithologyNameMaps.end())
	{
		return lithology->second;
	}

	return legacyLithotypeName;
}
std::string Prograde::LithologyConverter::upgradeLithologyDescription(std::string & legacyDescription, const int & lithologyFlag, const std::string & legacyParentLithology)
{
	std::string updatedDescription;
	updatedDescription = legacyDescription;
	if (lithologyFlag == 1)
	{
		updatedDescription = legacyDescription + ". Based on legacy BPA " + legacyParentLithology;
	}
	if (updatedDescription.size()>1000)
	{
		if (lithologyFlag == 0)
		{
			updatedDescription.resize(1000);
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* Description: Length of legacy description exceeds the allowed length of BPA2. Resetting the length to 1000 characters.";
		}
		else
		{
			int SizeOfAppendedString = static_cast<int>(legacyParentLithology.size() + 22);
			int allowedLegacyDescriptionSize = 1000 - SizeOfAppendedString;
			legacyDescription.resize(allowedLegacyDescriptionSize);
			updatedDescription = legacyDescription + ". Based on legacy BPA " + legacyParentLithology;
			
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* Description: Length of legacy description exceeds the allowed length of BPA2. Cutting the length of the original description and appended 'Based on legacy BPA <legacyParentLithoName' to it.";
		}
	}
	else
	{
		if (lithologyFlag == 0)
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* Description: No upgrade is done for this standard lithology which will get updated as per the description of the BPA2 mapped lithology while importing into BPA2-Basin.";
		else
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* Description: Updated the description for this userDefined lithology by appending 'Based on legacy BPA <legacyParentLithoName>'";

	}
	return updatedDescription;
}

void Prograde::LithologyConverter::upgradeLithologyAuditInfo(std::string & DefinitionDate, std::string & LastChangedBy, std::string & LastChangedDate, const int & lithologyFlag)
{
	std::string OriginalDefinitionDate = DefinitionDate;
	std::string OriginalLastChangedBy = LastChangedBy;
	std::string OriginalLastChangedDate = LastChangedDate;

	if (lithologyFlag == 0)
	{
		DefinitionDate = "November 21 2019 12:00";
		LastChangedBy = "Lorcan Kenan";
		LastChangedDate = "November 21 2019 12:00";

		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* Audit Info: Updating DefinitionDate from '"<<OriginalDefinitionDate<< "' to '"<<DefinitionDate<<"', LastChangedBy from '"<<OriginalLastChangedBy<<"' to '"<< LastChangedBy<<"' and LastChangedDate from '"<<OriginalLastChangedDate<<"' to '"<< LastChangedDate<<"'.";
	}
	else
	{
		LastChangedBy = "Lorcan Kenan";
		LastChangedDate = "November 21 2019 12:00";
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* Audit Info: Not changing the DefinitionDate but updating LastChangedBy from '" << OriginalLastChangedBy << "' to '" << LastChangedBy << "' and LastChangedDate from '" << OriginalLastChangedDate << "' to '" << LastChangedDate << "'.";
	}
}

std::string Prograde::LithologyConverter::findParentLithology(std::string legacydefinedBy, std::string lithologyDescription, const int lithologyFlag)
{
	string ParentLithoName;
	size_t pos_FirstPipe = legacydefinedBy.find("|");
	size_t pos_SecondPipe = legacydefinedBy.find_last_of("|");
	ParentLithoName = legacydefinedBy.substr((pos_FirstPipe + 1), (pos_SecondPipe - 1) - pos_FirstPipe);// It gives the base litho after the First pipe
	
	if(ParentLithoName!="")
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* Parent Lithology Name:  Parent lithology name is  " << ParentLithoName << " and is derived from original DefinedBy field information.";
	
		//If the parent lithology namme is not available in the "DefinedBy" field then try to extract from the Description field as per the mapping available for some selected lithologies
	else
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* Parent Lithology Name: Parent lithology information is not available in the DefinedBy field of the original project file for this user defined lithotype.";
		if (lithologyDescription.compare("Soil Mechanics Sandstone") == 0)
		{
			ParentLithoName = "SM. Sandstone";
		}
		else if (lithologyDescription.compare("Soil Mechanics Mudstone 50% Clay") == 0)
		{
			ParentLithoName = "SM.Mudst.50%Clay";
		}
		else if (lithologyDescription.compare("Soil Mechanics Mudstone 40% Clay") == 0 || lithologyDescription.compare("SM Mudstone 20% Clay") == 0)
		{
			ParentLithoName = "SM.Mudst.40%Clay";
		}
		else if (lithologyDescription.compare("SGP_Shale") == 0 || lithologyDescription.compare("Standard Shale") == 0)
		{
			ParentLithoName = "Std. Shale";
		}
		else if (lithologyDescription.compare("SM Mudstone 80% Clay") == 0 || lithologyDescription.compare("Soil Mechanics Mudstone 60% Clay") == 0)
		{
			ParentLithoName = "SM.Mudst.60%Clay";
		}
		else if (lithologyDescription.compare("SM Siltstone") == 0 || lithologyDescription.compare("Standard Siltstone") == 0)
		{
			ParentLithoName = "Std. Siltstone";
		}
		else if (lithologyDescription.compare("Standard Coal") == 0)
		{
			ParentLithoName = "Std. Coal";
		}
		else if (lithologyDescription.compare("StaSm Grainstone") == 0)
		{
			ParentLithoName = "Std.Grainstone";
		}
		else if (lithologyDescription.compare("Limestone as used by IBS-HEAT") == 0)
		{
			ParentLithoName = "HEAT Limestone";
		}
		else if (lithologyDescription.compare("New Salt SM GVB") == 0)
		{
			ParentLithoName = "Std. Salt";
		}
		else if (lithologyDescription.compare("StanSM Dolo Grainsto") == 0)
		{
			ParentLithoName = "Std.Dolo.Grainstone";
		}
		else if (lithologyDescription.compare("Standard Sandstone") == 0 || lithologyDescription.compare("silica ooze") == 0)
		{
			ParentLithoName = "Std. Sandstone";
		}



		else if (lithologyDescription.compare("Al Bashair Shale") == 0)
		{
			ParentLithoName = "Std. Shale";
		}
		else if (lithologyDescription.compare("Allow minor porosity and permeability near surface.") == 0)
		{
			ParentLithoName = "Std. Basalt";
		}
		else if (lithologyDescription.compare("Chalk as used by IBS-HEAT") == 0)
		{
			ParentLithoName = "HEAT Chalk";
		}
		else if (lithologyDescription.compare("Cold Shale_1.0") == 0)
		{
			ParentLithoName = "Std. Shale";
		}
		else if (lithologyDescription.compare("DeepCompactedSand") == 0)
		{
			ParentLithoName = "Std. Sandstone";
		}
		else if (lithologyDescription.compare("DeepSh_LP20%") == 0)
		{
			ParentLithoName = "Std. Shale";
		}
		else if (lithologyDescription.compare("DeepShaleLowPerm") == 0)
		{
			ParentLithoName = "Std. Shale";
		}
		else if (lithologyDescription.compare("hot_shale to introduce a great amount of heat from the surface into the model by introducitn a hot Radiogenic heat genration") == 0)
		{
			ParentLithoName = "Std. Shale";
		}
		else if (lithologyDescription.compare("Klaus_Sand") == 0)
		{
			ParentLithoName = "Std. Sandstone";
		}
		else if (lithologyDescription.compare("Klaus_Sand2") == 0)
		{
			ParentLithoName = "Std. Sandstone";
		}
		else if (lithologyDescription.compare("Lower Depositional Perm (value of std shale not std siltstone).\\n\\nLower thermal conductivity (using geometric not arithmetic mean).\\n\\nModified to fit general result from RS initial inversion. Summary much lower CC (why? OP? sandier?).\\n\\nMix Lithology created from: Project Lithology Std. Shale(50%); Project Lithology Std. Sandstone(25%); Project Lithology Std.Lime Mudstone(15%); Project Lithology Std.Dolo.Mudstone(10%); \\n\\nAdjusted to match Phi-VP xlsx") == 0)
		{
			ParentLithoName = "Std. Shale";
		}
		else if (lithologyDescription.compare("Lower thermal conductivity (using geometric not arithmetic mean).\\n\\nModified to fit general result from RS initial inversion. Summary much lower CC (why? OP? sandier?).\\n\\nMix Lithology created from: Project Lithology Std. Shale(50%); Project Lithology Std. Sandstone(25%); Project Lithology Std.Lime Mudstone(15%); Project Lithology Std.Dolo.Mudstone(10%); \\n\\nAdjusted to match Phi-VP xlsx") == 0)
		{
			ParentLithoName = "Std. Shale";
		}
		else if (lithologyDescription.compare("lowporositywadsand") == 0)
		{
			ParentLithoName = "Std. Sandstone";
		}
		else if (lithologyDescription.compare("LSB_Vlieland") == 0)
		{
			ParentLithoName = "Std. Sandstone";
		}
		else if (lithologyDescription.compare("Mod. Basalt") == 0)
		{
			ParentLithoName = "Std. Basalt";
		}
		else if (lithologyDescription.compare("NewAnhydrite") == 0)
		{
			ParentLithoName = "Std. Anhydrite";
		}
		else if (lithologyDescription.compare("Nile Sandstone") == 0)
		{
			ParentLithoName = "Std. Sandstone";
		}
		else if (lithologyDescription.compare("Qatar Anhydrite") == 0)
		{
			ParentLithoName = "Std. Anhydrite";
		}
		else if (lithologyDescription.compare("Qatar Dolo Lime") == 0)
		{
			ParentLithoName = "Std.Dolo.Mudstone";
		}
		else if (lithologyDescription.compare("Qatar Dolomite") == 0)
		{
			ParentLithoName = "Std.Dolo.Mudstone";
		}
		else if (lithologyDescription.compare("Qatar Limestone") == 0)
		{
			ParentLithoName = "HEAT Limestone";
		}
		else if (lithologyDescription.compare("Qatar Shale") == 0)
		{
			ParentLithoName = "Std. Shale";
		}
		else if (lithologyDescription.compare("Shale as used by IBS-HEAT") == 0)
		{
			ParentLithoName = "HEAT Shale";
		}
		else if (lithologyDescription.compare("Standard Basalt") == 0)
		{
			ParentLithoName = "Std. Basalt";
		}
		else if (lithologyDescription.compare("Standard Calcareous Mudstone (Limestone)") == 0)
		{
			ParentLithoName = "Std.Lime Mudstone";
		}
		else if (lithologyDescription.compare("Standard Chalk") == 0)
		{
			ParentLithoName = "Std. Chalk";
		}
		else if (lithologyDescription.compare("Standard Coal") == 0)
		{
			ParentLithoName = "Std. Coal";
		}
		else if (lithologyDescription.compare("Standard Domlomitic Grainstone (Dolostone)") == 0)
		{
			ParentLithoName = "Std.Dolo.Grainstone";
		}
		else if (lithologyDescription.compare("Standard Grainstone (Limestone)") == 0)
		{
			ParentLithoName = "Std.Grainstone";
		}
		else if (lithologyDescription.compare("Standard Marl") == 0)
		{
			ParentLithoName = "Std. Marl";
		}
		else if (lithologyDescription.compare("Standard Salt") == 0)
		{
			ParentLithoName = "Std. Salt";
		}
		else if (lithologyDescription.compare("WAD_chalk") == 0)
		{
			ParentLithoName = "Std. Chalk";
		}
		else if (lithologyDescription.compare("WAD_sandstone") == 0)
		{
			ParentLithoName = "Std. Sandstone";
		}
		else if (lithologyDescription.compare("WAD_shale") == 0)
		{
			ParentLithoName = "Std. Shale";
		}
		else if (lithologyDescription.compare("Mix Lithology created from: Project Lithology Std. Basalt(90%); Project Lithology Std. Sandstone(10%); ") == 0)//check extra space at the end
		{
			ParentLithoName = "Std. Basalt";
		}
		else if (lithologyDescription.compare("Mix Lithology created from: Project Lithology Std. Basalt(90%); Project Lithology Std. Sandstone(10%); \\n\\nSeisVel=6500 instead of 5750") == 0)
		{
			ParentLithoName = "Std. Basalt";
		}
		else if (lithologyDescription.compare("Mix Lithology created from: Project Lithology Std. Basalt(90%); Project Lithology Std. Sandstone(10%); \\n\\nSlightly increased Surface Porosity\\nComp Coeff (Eff Stress) increased\\nMultipoint Perm Model Sm Sandstone\\nSeisVel=6500 instead of 5750") == 0)
		{
			ParentLithoName = "Std. Basalt";
		}
		else if (lithologyDescription.compare("Mix Lithology created from: Project Lithology Std. Basalt(90%); Project Lithology Std. Sandstone(10%); \\n\\nSlightly increased Surface Porosity\\nComp Coeff (Eff Stress) increased\\nMultipoint Perm Model Sm Sandstone\\nSeisVel=6500 instead of 5750\\n\\nHighest thermal conductivity") == 0)
		{
			ParentLithoName = "Std. Basalt";
		}
		else if (lithologyDescription.compare("Mix Lithology created from: Project Lithology Std. Basalt(90%); Project Lithology Std. Sandstone(10%); \\n\\nSlightly increased Surface Porosity\\nComp Coeff (Eff Stress) increased\\nMultipoint Perm Model Sm Sandstone\\nSeisVel=6500 instead of 5750\\n\\nLowest thermal conductivity") == 0)
		{
			ParentLithoName = "Std. Basalt";
		}
		else if (lithologyDescription.compare("Mix Lithology created from: Project Lithology Std. Basalt(90%); Project Lithology Std. Sandstone(10%);\\nAssign density 2830 for consistency with CTC basalt (combines true basalt and mantle depletion uplift effects).") == 0)
		{
			ParentLithoName = "Std. Basalt";
		}
		//Seems to be same record in the spreadsheet...Crossckeck?

		else if (lithologyDescription.compare("Mix Lithology created from: Project Lithology Std. Sandstone(20%); Project Lithology Std. Shale(80%);\\n\\nEst 12%clay equivalent\\nEst 6.46 comp coeff\\nEst Surf Phi 0.656\\nExcel estimates\\nVP solid 5670\\nRho solid 2652\\nThC Geom Mean 1.72") == 0)
		{
			ParentLithoName = "Std. Shale";
		}
		else if (lithologyDescription.compare("Mix Lithology created from: Project Lithology Std. Sandstone(20%); Project Lithology Std. Shale(80%);\\n\\nUse for Orange SPOB12-09 test\\nEdited k = weighted harm mean\\ncomp coeff from spreadsheet") == 0)
		{
			ParentLithoName = "Std. Shale";
		}
		else if (lithologyDescription.compare("Mix Lithology created from: Project Lithology Std. Sandstone(40%); Project Lithology Std. Siltstone(60%); ") == 0)//check extra space at last
		{
			ParentLithoName = "Std. Siltstone";
		}
		else if (lithologyDescription.compare("Mix Lithology created from: Project Lithology Std. Sandstone(75%); Project Lithology Std. Shale(25%);\\nEst 15%clay equivalent\\nEst 4.2325 comp coeff\\nEst Surf Phi 0.535\\nRaise to 80% due to Wyllie prob\\nExcel estimates\\nVP solid 5575\\nRho solid 2652\\nThC Geom Mean 3.02") == 0)
		{
			ParentLithoName = "Std. Sandstone";
		}
		else if (lithologyDescription.compare("Mix Lithology created from: Project Lithology Std. Sandstone(75%); Project Lithology Std. Shale(25%);\\nEst 15%clay equivalent\\nEst 4.2325 comp coeff\\nEst Surf Phi 0.535\\nRaise to 85% due to Wyllie prob\\nExcel estimates\\nVP solid 5575\\nRho solid 2652\\nThC Geom Mean 3.02") == 0)
		{
			ParentLithoName = "Std. Sandstone";
		}
		else if (lithologyDescription.compare("Mix Lithology created from: Project Lithology Std. Sandstone(75%); Project Lithology Std. Shale(25%);\\nEst 30%clay equivalent\\nEst 5.245 comp coeff\\nEst Surf Phi 0.59\\nChanged to 0.65% due to Wyllie problem.\\nExcel estimates\\nVP solid 5150\\nRho solid 2654\\nThC Geom Mean 2.34") == 0)
		{
			ParentLithoName = "Std. Sandstone";
		}
		else if (lithologyDescription.compare("Mix Lithology created from: Project Lithology Std. Shale(25%); Project Lithology Std. Sandstone(75%); ") == 0)
		{
			ParentLithoName = "Std. Sandstone";
		}
		else if (lithologyDescription.compare("Mix Lithology created from: Project Lithology Std. Shale(50%); Project Lithology Std. Sandstone(25%); Project Lithology Std.Lime Mudstone(15%); Project Lithology Std.Dolo.Mudstone(10%); \\n\\nAdjusted to match Phi-VP xlsx") == 0)
		{
			ParentLithoName = "Std. Shale";
		}
		else if (lithologyDescription.compare("Modified to fit general result from RS initial inversion. Summary much lower CC (why? OP? sandier?).\\n\\nMix Lithology created from: Project Lithology Std. Shale(50%); Project Lithology Std. Sandstone(25%); Project Lithology Std.Lime Mudstone(15%); Project Lithology Std.Dolo.Mudstone(10%); \\n\\nAdjusted to match Phi-VP xlsx") == 0)
		{
			ParentLithoName = "Std. Shale";
		}
		//Seems to be same as above..crosscheck
		else if (lithologyDescription.compare("Modified to fit general result from RS initial inversion. Summary much lower CC (why? OP? sandier?).\\n\\nMix Lithology created from: Project Lithology Std. Shale(50%); Project Lithology Std. Sandstone(25%); Project Lithology Std.Lime Mudstone(15%); Project Lithology Std.Dolo.Mudstone(10%); \\n\\nAdjusted to match Phi-VP xlsx") == 0)
		{
			ParentLithoName = "Std. Shale";
		}
		else if (lithologyDescription.compare("Standard Shale\\nFor tests constructed by Philipp in WSB") == 0)
		{
			ParentLithoName = "Std. Shale";
		}

		//check for the null string again
		if (ParentLithoName.compare("") == 0)
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  Error: Cann't update the parent lithology name from the lithology descriptionas as no mapping is available.";
		else
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  Parent lithology is mapped to " << ParentLithoName << " from the legacy lithotype description.";

	}
	return ParentLithoName;
}

void Prograde::LithologyConverter::upgradePermModelForUsrDefLitho(const std::string & permModelName, std::vector<double> & mpPor, std::vector<double> & mpPerm, int & numPts)
{
   if ((permModelName == "None") || (permModelName == "Impermeable"))
   {
      mpPor[0] = 5;
      mpPor[1] = 60;
      mpPerm[0] = -6;
      mpPerm[1] = -6;
   }
   else if (permModelName == "Sands") 
   {
      mpPor[0] = 5;
      mpPor[1] = 60;
      mpPerm[0] = 0.2;
      mpPerm[1] = 4.9;
   }
   else if (permModelName == "Shales") 
   {
      mpPor[0] = 5;
      mpPor[1] = 60;
      mpPerm[0] = -6.4;
      mpPerm[1] = -2.16;
   }
}

ErrorHandler::ReturnCode Prograde::LithologyConverter::PreprocessLithofaciesInputOfStratIoTbl(std::vector<std::string>& lithologyNamesSingleLayer, std::vector<double>& lithoPercntSingleLayer, std::vector<std::string>& lithoPercntGridSingleLayer)
{
	if (lithologyNamesSingleLayer.size() != 3 or lithoPercntSingleLayer.size() != 3 or lithoPercntGridSingleLayer.size() != 2 or 
		( lithologyNamesSingleLayer[0] == "" and lithologyNamesSingleLayer[1] == "" and lithologyNamesSingleLayer[2]=="") )
	{//Added just as a precautionary measure to filter it out erronious scenarios like this from importing in to BPA2-Basin
		return ErrorHandler::ReturnCode::ValidationError;
	}
	else
	{
		for (size_t temp = 0; temp < lithologyNamesSingleLayer.size(); temp++)
		{
			std::string legacyLithoTypeName = lithologyNamesSingleLayer[temp];
			double legacyLithoPerctScalar = lithoPercntSingleLayer[temp];

			//This loop is provided to preprocess the inputs as per the need of Import validation...If the inputs are not as per the BPA2 import validation code structure then modify the inputs
			//	-> Lithology can't be imported if the lithology name is not available but lithology percents are available.
			//		-> If this is the case, then reset the lithology percentages to NO-DATA-VALUE
			//	-> If lithology is available but lithology percentages have both scalar and map specified then 
			//		-> Keep the map info and reset the scalar value to NO-DATA-VALUE
			if (legacyLithoTypeName.compare("") == 0 && legacyLithoPerctScalar != DataAccess::Interface::DefaultUndefinedScalarValue)
			{
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Info> Lithology percent is found whereas the Lithotype" << (temp + 1) << " is not defined. " <<
					"Resetting Percent" << (temp + 1) << " from " << legacyLithoPerctScalar << " to " << DataAccess::Interface::DefaultUndefinedScalarValue;

				lithoPercntSingleLayer[temp] = DataAccess::Interface::DefaultUndefinedScalarValue;
				if (temp < 2)
				{
					std::string legacyLithoPerctMap = lithoPercntGridSingleLayer[temp];
					if (legacyLithoPerctMap != DataAccess::Interface::NullString)
					{
						lithoPercntGridSingleLayer[temp] = DataAccess::Interface::NullString;
						LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Info> Resetting lithology PercentGrid" << (temp + 1) << " from " << legacyLithoPerctMap << " to " << DataAccess::Interface::NullString;
					}
				}

				///Check if lithology==NULL, percent=-9999 but PercentGrid != NULL...Is this a possible use case????
			}
			else
			{
				std::string bpa2LithoName = upgradeLithologyName(legacyLithoTypeName);
				lithologyNamesSingleLayer[temp] = bpa2LithoName;
				if (bpa2LithoName != legacyLithoTypeName)
					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> Lithotype" << (temp + 1) << " name is updated from '" << legacyLithoTypeName << "' to '" << bpa2LithoName << "'";
				if (temp < 2 && lithoPercntSingleLayer[temp] != DataAccess::Interface::DefaultUndefinedScalarValue && lithoPercntGridSingleLayer[temp] != "")
				{
					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Info> Lithotype" << (temp + 1) << " has both scalar and maps. Percent" << (temp + 1) << " value is updated from " << lithoPercntSingleLayer[temp] << " to " << DataAccess::Interface::DefaultUndefinedScalarValue;
					lithoPercntSingleLayer[temp] = DataAccess::Interface::DefaultUndefinedScalarValue;
				}
			}
		}
	}

	return ErrorHandler::ReturnCode::NoError;
}

void Prograde::LithologyConverter::upgradePermModelForSysDefLitho(const std::string & bpa2LithoName, std::vector<double> & mpPor, std::vector<double> & mpPerm, int & numPts)
{
   numPts = 2;
   mpPor.resize(numPts);
   mpPerm.resize(numPts);
   if ((bpa2LithoName.compare("Mantle") == 0) || (bpa2LithoName.compare("Crust") == 0) || 
      (bpa2LithoName.compare("Diorite/Granodiorite (SI)") == 0) || (bpa2LithoName.compare("Dry Gabbro/Basalt (SI)") == 0) ||
      (bpa2LithoName.compare("Wet Gabbro/Basalt (SI)") == 0) || (bpa2LithoName.compare("Granite/Rhyolite (SI)") == 0) ||
      (bpa2LithoName.compare("Basalt, typical") == 0))
      {
         mpPor[0] = 5;
         mpPor[1] = 60;
         mpPerm[0] = -4;
         mpPerm[1] = -4;
      }
   else if (bpa2LithoName.compare("Chalk, white") == 0)
      {
         mpPor[0] = 5;
         mpPor[1] = 60;
         mpPerm[0] = -5.45;
         mpPerm[1] = 1.8;
      }
   else if (bpa2LithoName.compare("Grainstone, dolomitic, typical") == 0)
      {
         mpPor[0] = 5;
         mpPor[1] = 60;
         mpPerm[0] = 1.6;
         mpPerm[1] = 6.4;
      }
   else if (bpa2LithoName.compare("Grainstone, calcitic, typical") == 0)
      {
         mpPor[0] = 5;
         mpPor[1] = 60;
         mpPerm[0] = 1.6;
         mpPerm[1] = 6.4;
      }
   else if (bpa2LithoName.compare("Sandstone, typical") == 0)
      {
         mpPor[0] = 5;
         mpPor[1] = 60;
         mpPerm[0] = 0.2;
         mpPerm[1] = 4.9;
      }
   else if (bpa2LithoName.compare("Mudstone, 60% clay") == 0)
      {
         mpPor[0] = 5;
         mpPor[1] = 60;
         mpPerm[0] = -6.4;
         mpPerm[1] = -2.16;
      }
   else if (bpa2LithoName.compare("Mudstone, 50% clay") == 0)
      {
         mpPor[0] = 5;
         mpPor[1] = 60;
         mpPerm[0] = -6.19;
         mpPerm[1] = -1.28;
      }
   else if (bpa2LithoName.compare("Mudstone, 40% clay") == 0)
      {
         mpPor[0] = 5;
         mpPor[1] = 60;
         mpPerm[0] = -5.98;
         mpPerm[1] = -0.39;
      }
   else if ((bpa2LithoName.compare("Ice") == 0) || (bpa2LithoName.compare("Anhydrite") == 0) ||
      (bpa2LithoName.compare("Coal") == 0) || (bpa2LithoName.compare("Halite") == 0) ||
      (bpa2LithoName.compare("Potash-Sylvite") == 0))
      {
         mpPor[0] = 5;
         mpPor[1] = 60;
         mpPerm[0] = -6;
         mpPerm[1] = -6;
      }
   else if (bpa2LithoName.compare("Marl") == 0)
      {
         mpPor[0] = 5;
         mpPor[1] = 60;
         mpPerm[0] = -5.98;
         mpPerm[1] = -0.39;
      }
   else if (bpa2LithoName.compare("Siltstone, 20% clay") == 0)
      {
         mpPor[0] = 5;
         mpPor[1] = 60;
         mpPerm[0] = -5.55;
         mpPerm[1] = 1.37;
      }
   else if (bpa2LithoName.compare("Lime-Mudstone, dolomitic") == 0)
      {
         mpPor[0] = 5;
         mpPor[1] = 60;
         mpPerm[0] = -6.19;
         mpPerm[1] = -1.28;
      }
   else if (bpa2LithoName.compare("Lime-Mudstone") == 0)
      {
         mpPor[0] = 5;
         mpPor[1] = 60;
         mpPerm[0] = -6.19;
         mpPerm[1] = -1.28;
      }
   else
   {
      //
   }
}

void Prograde::LithologyConverter::computeSingleExpModelParameters(const std::string legacyParentLithoName, const int lithologyFlag, mbapi::LithologyManager::PorosityModel &porModel, std::vector<double> & originalPorModelParam, std::vector<double> & newPorModelParam)
{
	string baseLithologyType = legacyParentLithoName;

	double newSurfacePorosity;
	double newMinMechanicalPorosity;
	double newExponentialCompCoeff;

	if (lithologyFlag == 1 && porModel == mbapi::LithologyManager::PorosityModel::PorSoilMechanics)
	{
		if ((baseLithologyType.compare("HEAT Sandstone") == 0) || (baseLithologyType.compare("SM. Sandstone") == 0) || (baseLithologyType.compare("Std. Sandstone") == 0))
		{
			porModel = mbapi::LithologyManager::PorosityModel::PorExponential;//upgrading the porosity model to "Exponential"
			newSurfacePorosity = 45.0;
			newExponentialCompCoeff = 3.25;
			newMinMechanicalPorosity = 5.0;

			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* Deprecated 'Soil Mechanics' model is detected for this lithotype which is based on legacy " << baseLithologyType << ". Porosity model is upgraded to 'Exponential' model with default values of BPA2 'Sandstone,typical'. ";
			newPorModelParam.push_back(newSurfacePorosity);
			newPorModelParam.push_back(newExponentialCompCoeff);
			newPorModelParam.push_back(newMinMechanicalPorosity);
		}
		else if (baseLithologyType.compare("Std. Siltstone") == 0)
		{
			porModel = mbapi::LithologyManager::PorosityModel::PorExponential;//upgrading the porosity model to "Exponential"
			newSurfacePorosity = 35.0;
			newExponentialCompCoeff = 5.5;
			newMinMechanicalPorosity = 2.0;

			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* Deprecated 'Soil Mechanics' model is detected for this lithotype which is based on legacy " << baseLithologyType << ". Porosity model is upgraded to 'Exponential' model with default values of BPA2 'Siltstone, clean'. ";
			newPorModelParam.push_back(newSurfacePorosity);
			newPorModelParam.push_back(newExponentialCompCoeff);
			newPorModelParam.push_back(newMinMechanicalPorosity);
		}
		else if (baseLithologyType.compare("Std. Shale") == 0 || baseLithologyType.compare("HEAT Shale") == 0 ||
			baseLithologyType.compare("SM.Mudst.40%Clay") == 0 || baseLithologyType.compare("SM.Mudst.50%Clay") == 0 ||
			baseLithologyType.compare("SM.Mudst.60%Clay") == 0)
		{
			porModel = mbapi::LithologyManager::PorosityModel::PorExponential;//upgrading the porosity model to "Exponential"

			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* Deprecated 'Soil Mechanics' model is detected for this lithotype which is based on legacy " << baseLithologyType << ". Porosity model is upgraded to 'Exponential' model with model parameters calculated from the imperical formula. ";

			newSurfacePorosity = (0.0173*originalPorModelParam[0] * originalPorModelParam[0]) - (0.6878*originalPorModelParam[0]) + 27.73;
			//Newly calculated SurfacePorosity values must be in the range [0.1,100]. Resetting it's value if it is outside this range
			if (newSurfacePorosity < 0.1) {
				newSurfacePorosity = 0.1;
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  Newly calculated SurfacePorosity is less than the minimum allowed value of 0.1. Resetting its value to 0.1.";
			}
			if (newSurfacePorosity > 100.0) {
				newSurfacePorosity = 100.0;
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  Newly calculated SurfacePorosity is greater than the maximum allowed value of 100. Resetting its value to 100.";
			}

			newExponentialCompCoeff = (-1.508*originalPorModelParam[1] * originalPorModelParam[1]) + (2.526*originalPorModelParam[1]) + 5.05;
			//Newly calculated compaction coefficient values must be in the range [0,25]. Resetting it's value if it is outside this range
			if (newExponentialCompCoeff < 0.0) {
				newExponentialCompCoeff = 0.0;
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  Newly calculated CompactionCoeff. is less than the minimum allowed value of 0. Resetting its value to 0.";
			}
			if (newExponentialCompCoeff > 25.0) {
				newExponentialCompCoeff = 25.0;//Checking for the allowed range specified for BPA2
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  Newly calculated CompactionCoeff. is greater than the maximum allowed value of 25. Resetting its value to 25.";
			}

			newMinMechanicalPorosity = (0.00123*originalPorModelParam[0] * originalPorModelParam[0]) - (0.04913*originalPorModelParam[0]) + 4.34;
			//Newly calculated Min. Porosity values must be in the range [0.1,SurfacePorosity]. Resetting it's value if it is outside this range
			if (newMinMechanicalPorosity < 0.1) {
				newMinMechanicalPorosity = 0.1;
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  Newly calculated MinPorosity is less than the minimum allowed value of 0.1. Resetting its value to 0.1.";
			}
			if (newMinMechanicalPorosity > newSurfacePorosity) {
				newMinMechanicalPorosity = newSurfacePorosity;
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  Newly calculated MinPorosity is greater than the value of SurfacePorosity. Resetting its value to SurfacePorosity.";
			}

			newPorModelParam.push_back(newSurfacePorosity);
			newPorModelParam.push_back(newExponentialCompCoeff);
			newPorModelParam.push_back(newMinMechanicalPorosity);
		}
		else if (baseLithologyType.compare("Std.Dolo.Grainstone") == 0)
		{
			porModel = mbapi::LithologyManager::PorosityModel::PorExponential;//upgrading the porosity model to "Exponential"
			newSurfacePorosity = 35.0;
			newExponentialCompCoeff = 8.0;
			newMinMechanicalPorosity = 5.0;

			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* Deprecated 'Soil Mechanics' model is detected for this lithotype which is based on legacy " << baseLithologyType << ". Porosity model is upgraded to 'Exponential' model with default values of BPA2 'Grainstone, dolomitic, typical'. ";
			newPorModelParam.push_back(newSurfacePorosity);
			newPorModelParam.push_back(newExponentialCompCoeff);
			newPorModelParam.push_back(newMinMechanicalPorosity);
		}
		else if (baseLithologyType.compare("Std.Grainstone") == 0)
		{
			porModel = mbapi::LithologyManager::PorosityModel::PorExponential;//upgrading the porosity model to "Exponential"
			newSurfacePorosity = 35.0;
			newExponentialCompCoeff = 8.0;
			newMinMechanicalPorosity = 5.0;

			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* Deprecated 'Soil Mechanics' model is detected for this lithotype which is based on legacy " << baseLithologyType << ". Porosity model is upgraded to 'Exponential' model with default values of BPA2 'Grainstone, calcitic, typical'. ";
			newPorModelParam.push_back(newSurfacePorosity);
			newPorModelParam.push_back(newExponentialCompCoeff);
			newPorModelParam.push_back(newMinMechanicalPorosity);
		}
		else if (baseLithologyType.compare("Std.Lime Mudstone") == 0)
		{
			porModel = mbapi::LithologyManager::PorosityModel::PorExponential;//upgrading the porosity model to "Exponential"
			newSurfacePorosity = 35.0;
			newExponentialCompCoeff = 6.0;
			newMinMechanicalPorosity = 3.0;

			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* Deprecated 'Soil Mechanics' model is detected for this lithotype which is based on legacy " << baseLithologyType << ". Porosity model is upgraded to 'Exponential' model with default values of BPA2 'Lime-Mudstone'. ";
			newPorModelParam.push_back(newSurfacePorosity);
			newPorModelParam.push_back(newExponentialCompCoeff);
			newPorModelParam.push_back(newMinMechanicalPorosity);
		}
		else if ((baseLithologyType.compare("Std.Dolo.Mudstone")) == 0)
		{
			porModel = mbapi::LithologyManager::PorosityModel::PorExponential;//upgrading the porosity model to "Exponential"
			newSurfacePorosity = 35.0;
			newExponentialCompCoeff = 6.0;
			newMinMechanicalPorosity = 3.0;

			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* Deprecated 'Soil Mechanics' model is detected for this lithotype which is based on legacy " << baseLithologyType << ". Porosity model is upgraded to 'Exponential' model with default values of BPA2 'Lime-Mudstone, dolomitic'. ";
			newPorModelParam.push_back(newSurfacePorosity);
			newPorModelParam.push_back(newExponentialCompCoeff);
			newPorModelParam.push_back(newMinMechanicalPorosity);
		}
		else if (baseLithologyType.compare("Std. Marl") == 0)
		{
			porModel = mbapi::LithologyManager::PorosityModel::PorExponential;//upgrading the porosity model to "Exponential"
			newSurfacePorosity = 50.0;
			newExponentialCompCoeff = 5.0;
			newMinMechanicalPorosity = 3.0;

			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* Deprecated 'Soil Mechanics' model is detected for this lithotype which is based on legacy " << baseLithologyType << ". Porosity model is upgraded to 'Exponential' model with default values of BPA2 'Marl'. ";
			newPorModelParam.push_back(newSurfacePorosity);
			newPorModelParam.push_back(newExponentialCompCoeff);
			newPorModelParam.push_back(newMinMechanicalPorosity);
		}
		else if (baseLithologyType.compare("HEAT Limestone") == 0)
		{
			porModel = mbapi::LithologyManager::PorosityModel::PorExponential;//upgrading the porosity model to "Exponential"
			newSurfacePorosity = 35.0;
			newExponentialCompCoeff = 8.0;
			newMinMechanicalPorosity = 5.0;

			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* Deprecated 'Soil Mechanics' model is detected for this lithotype which is based on legacy " << baseLithologyType << ". Porosity model is upgraded to 'Exponential' model with default values of BPA2 'Graistone, calcitic, typical'. ";
			newPorModelParam.push_back(newSurfacePorosity);
			newPorModelParam.push_back(newExponentialCompCoeff);
			newPorModelParam.push_back(newMinMechanicalPorosity);
		}
		else if (baseLithologyType.compare("Std. Salt") == 0)
		{
			porModel = mbapi::LithologyManager::PorosityModel::PorExponential;//upgrading the porosity model to "Exponential"
			newSurfacePorosity = 0.1;
			newExponentialCompCoeff = 0.0;
			newMinMechanicalPorosity = 0.1;

			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* Deprecated 'Soil Mechanics' model is detected for this lithotype which is based on legacy " << baseLithologyType << ". Porosity model is upgraded to 'Exponential' model with default values of BPA2 'Halite'. ";
			newPorModelParam.push_back(newSurfacePorosity);
			newPorModelParam.push_back(newExponentialCompCoeff);
			newPorModelParam.push_back(newMinMechanicalPorosity);
		}
		else if (baseLithologyType.compare("Std. Coal") == 0)
		{
			porModel = mbapi::LithologyManager::PorosityModel::PorExponential;//upgrading the porosity model to "Exponential"
			newSurfacePorosity = 75.0;
			newExponentialCompCoeff = 15;
			newMinMechanicalPorosity = 1.1;

			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* Deprecated 'Soil Mechanics' model is detected for this lithotype which is based on legacy " << baseLithologyType << ". Porosity model is upgraded to 'Exponential' model with default values of BPA2 'Coal'. ";
			newPorModelParam.push_back(newSurfacePorosity);
			newPorModelParam.push_back(newExponentialCompCoeff);
			newPorModelParam.push_back(newMinMechanicalPorosity);
		}
		else {
			newPorModelParam = originalPorModelParam;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* ERROR: Deprecated 'Soil Mechanics' model is detected for this lithotype which is based on legacy " << baseLithologyType << ". Porosity model is not upgraded to 'Exponential' as no mapping is available for this parent lithology. ";
		}
	}
	else if (lithologyFlag == 0 && porModel == mbapi::LithologyManager::PorosityModel::PorSoilMechanics) {
		newPorModelParam = originalPorModelParam;
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* Deprecated 'SoilMechanics' model is detected for this standard lithotype. No upgradation of the porosity model is done via PROGRADE which will be updated from the BPA2 reference catalog while importing the scenario in BPA2. ";
	}
	else if (lithologyFlag == 0 && porModel == mbapi::LithologyManager::PorosityModel::PorExponential) {
		newPorModelParam = originalPorModelParam;
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* Latest porosity model is detected for this standard lithotype. No upgradation of the porosity model parameters are done via PROGRADE which will be updated from the BPA2 reference catalog while importing the scenario in BPA2. ";
	}
	else //For all legacy user defined defined lithologies with Expoenential porosity model. 
		 //No upgradation of porosity model is done, but checking for the out-of-range values only and handled them 
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* Latest porosity model is detected for this lithotype. No upgradation of the porosity model is needed. ";

		//SurfacePorosity values must be in the range [0.1,100]. Resetting it's value if it is outside this range
		if (originalPorModelParam[0] < 0.1) {
			originalPorModelParam[0] = 0.1;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  Legacy SurfacePorosity is less than the minimum allowed value of 0.1. Resetting its value to 0.1.";
		}
		if (originalPorModelParam[0] > 100.0) {
			originalPorModelParam[0] = 100.0;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  Legacy SurfacePorosity is greater than the maximum allowed value of 100. Resetting its value to 100.";
		}

		//Compaction coefficient values must be in the range [0,25]. Resetting it's value if it is outside this range
		if (originalPorModelParam[1] < 0.0) {
			originalPorModelParam[1] = 0.0;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  Legacy CompactionCoeff. is less than the minimum allowed value of 0. Resetting its value to 0.";
		}
		if (originalPorModelParam[1] > 25.0) {
			originalPorModelParam[1] = 25.0;//Checking for the allowed range specified for BPA2
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  Legacy CompactionCoeff. is greater than the maximum allowed value of 25. Resetting its value to 25.";
		}
		//Newly calculated Min. Porosity values must be in the range [0.1,SurfacePorosity]. Resetting it's value if it is outside this range
		if (originalPorModelParam[2] < 0.1) {
			originalPorModelParam[2] = 0.1;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  Legacy MinPorosity is less than the minimum allowed value of 0.1. Resetting its value to 0.1.";
		}
		if (originalPorModelParam[2] > originalPorModelParam[0]) {
			originalPorModelParam[2] = originalPorModelParam[0];
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  Legacy MinPorosity is greater than the value of SurfacePorosity. Resetting its value to SurfacePorosity.";
		}
		newPorModelParam = originalPorModelParam;
	}

	//return newPorModelParam;

}

void Prograde::LithologyConverter::upgradeLitPropDensity(double & value)
{
   double lwrLimit = 500;
   double uprLimit = 10000;
   if (value < lwrLimit)
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Original Density value '" << value << "' is less than the minimum acceptable value of BPA2 and updating its value to '" << lwrLimit << "'";
      value = lwrLimit;
   }
   else if (value > uprLimit)
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Original Density value '" << value << "' is larger than the acceptable value of BPA2 and updating its value to '" << uprLimit << "'";
      value = uprLimit;
   }

}
void Prograde::LithologyConverter::upgradeLitPropHeatProduction(double & value)
{
   double lwrLimit = 0;
   double uprLimit = 25;
   if (value < lwrLimit)
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Original Heat Production value '" << value << "' is less than the minimum acceptable value of BPA2 and updating its value to '" << lwrLimit << "'";
      value = lwrLimit;
   }
   else if (value > uprLimit)
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Original Heat Production value '" << value << "' is larger than the acceptable value of BPA2 and updating its value to '" << uprLimit << "'";
      value = uprLimit;
   }

}
void Prograde::LithologyConverter::upgradeLitPropThrConductivity(double & value)
{
   double lwrLimit = 0;
   double uprLimit = 10;
   if (value < lwrLimit)
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Original Thermal Conductivity value '" << value << "' is less than the minimum acceptable value of BPA2 and updating its value to '" << lwrLimit << "'";
      value = lwrLimit;
   }
   else if (value > uprLimit)
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Original Thermal Conductivity value '" << value << "' is larger than the acceptable value of BPA2 and updating its value to '" << uprLimit << "'";
      value = uprLimit;
   }

}
void Prograde::LithologyConverter::upgradeLitPropThrCondAnistropy(double & value)
{
   double lwrLimit = 0;
   double uprLimit = 10;
   if (value < lwrLimit)
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Original Thermal Conductivity Anistropy value '" << value << "' is less than the minimum acceptable value of BPA2 and updating its value to '" << lwrLimit << "'";
      value = lwrLimit;
   }
   else if (value > uprLimit)
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Original Thermal Conductivity Anistropy value '" << value << "' is larger than the acceptable value of BPA2 and updating its value to '" << uprLimit << "'";
      value = uprLimit;
   }

}
void Prograde::LithologyConverter::upgradeLitPropPermAnistropy(double & value)
{
   double lwrLimit = 0;
   double uprLimit = 100;
   if (value < lwrLimit)
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Original Permeability Anistropy value '" << value << "' is less than the minimum acceptable value of BPA2 and updating its value to '" << lwrLimit << "'";
      value = lwrLimit;
   }
   else if (value > uprLimit)
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Original Permeability Anistropy value '" << value << "' is larger than the acceptable value of BPA2 and updating its value to '" << uprLimit << "'";
      value = uprLimit;
   }

}
void Prograde::LithologyConverter::upgradeLitPropSeisVelocity(double & value)
{
   double lwrLimit = 1000;
   double uprLimit = 9000;
   if (value < lwrLimit)
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Original Seismic Velocity value '" << value << "' is less than the minimum acceptable value of BPA2 and updating its value to '" << lwrLimit << "'";
      value = lwrLimit;
   }
   else if (value > uprLimit)
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Original Seismic Velocity value '" << value << "' is larger than the acceptable value of BPA2 and updating its value to '" << uprLimit << "'";
      value = uprLimit;
   }

}
void Prograde::LithologyConverter::upgradeLitPropSeisVeloExponent(double & value)
{
   double lwrLimit = -1;
   double uprLimit = 1;
   if (value < lwrLimit)
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Original Seismic Velocity Exponent value '" << value << "' is less than the minimum acceptable value of BPA2 and updating its value to '" << lwrLimit << "'";
      value = lwrLimit;
   }
   else if (value > uprLimit)
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Original Seismic Velocity Exponent value '" << value << "' is larger than the acceptable value of BPA2 and updating its value to '" << uprLimit << "'";
      value = uprLimit;
   }

}
void Prograde::LithologyConverter::upgradeLitPropEntryPresCoeff1(double & value)
{
   double lwrLimit = 0.1;
   double uprLimit = 2;
   if (value < lwrLimit)
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Original Entry Pressure Coefficient 1 value '" << value << "' is less than the minimum acceptable value of BPA2 and updating its value to '" << lwrLimit << "'";
      value = lwrLimit;
   }
   else if (value > uprLimit)
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Original Entry Pressure Coefficient 1 value '" << value << "' is larger than the acceptable value of BPA2 and updating its value to '" << uprLimit << "'";
      value = uprLimit;
   }

}
void Prograde::LithologyConverter::upgradeLitPropEntryPresCoeff2(double & value)
{
   double lwrLimit = -1.5;
   double uprLimit = 1.5;
   if (value < lwrLimit)
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Original Entry Pressure Coefficient 2 value '" << value << "' is less than the minimum acceptable value of BPA2 and updating its value to '" << lwrLimit << "'";
      value = lwrLimit;
   }
   else if (value > uprLimit)
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Original Entry Pressure Coefficient 2 value '" << value << "' is larger than the acceptable value of BPA2 and updating its value to '" << uprLimit << "'";
      value = uprLimit;
   }

}
void Prograde::LithologyConverter::upgradeLitPropHydFracturing(double & value)
{
   double lwrLimit = 0;
   double uprLimit = 100;
   if (value < lwrLimit)
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Original Hydraulic Fracturing value '" << value << "' is less than the minimum acceptable value of BPA2 and updating its value to '" << lwrLimit << "'";
      value = lwrLimit;
   }
   else if (value > uprLimit)
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Original Hydraulic Fracturing value '" << value << "' is larger than the acceptable value of BPA2 and updating its value to '" << uprLimit << "'";
      value = uprLimit;
   }

}
void Prograde::LithologyConverter::upgradeLitPropRefSoldViscosity(double & value)
{
   double lwrLimit = 0;
   double uprLimit = 1e18;
   if (value < lwrLimit)
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Original ReferenceSolidViscosity value '" << value << "' is less than the minimum acceptable value of BPA2 and updating its value to '" << lwrLimit << "'";
      value = lwrLimit;
   }
   else if (value > uprLimit)
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Original ReferenceSolidViscosity value '" << value << "' is larger than the acceptable value of BPA2 and updating its value to '" << uprLimit << "'";
      value = uprLimit;
   }

}
void Prograde::LithologyConverter::upgradeLitPropIntrTemperature(double & value)
{
   double lwrLimit = 600;
   double uprLimit = 1650;
   if (value == -9999)
      return;
   if (value < lwrLimit)
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Original Intrusion Temperature value '" << value << "' is less than the minimum acceptable value of BPA2 and updating its value to '" << lwrLimit << "'";
      value = lwrLimit;
   }
   else if (value > uprLimit)
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Original Intrusion Temperature value '" << value << "' is larger than the acceptable value of BPA2 and updating its value to '" << uprLimit << "'";
      value = uprLimit;
   }

}

	

	



