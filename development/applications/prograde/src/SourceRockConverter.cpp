//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "SourceRockConverter.h"
#include "SourceRockTypeNameMappings.h"
//utilities
#include "LogHandler.h"

//cmbAPI
#include "SourceRockManager.h"
# include "ConstantsNumerical.h"
#include<map>

using namespace mbapi;

//bpaBSr				bpa2BSr					bpa2Sr			
static Prograde::pairMap create_map()
{
	Prograde::pairMap m =
	{
		{"Type_I_CenoMesozoic_Lacustrine_kin",			std::make_pair("Type_I_CenoMesozoic_Lacustrine_Algal_Shale_kin", "Type I CenoMesozoic Lacustrine Algal Shale (Kinetics)")},
		{"Type_I_II_Mesozoic_MarineShale_lit",			std::make_pair("Type_I_II_Mesozoic_Marine_Shale_lit", "Type I/II Mesozoic Marine Shale (Literature)")},
		{"Type_II_Mesozoic_MarineShale_kin",			std::make_pair("Type_II_Mesozoic_Marine_Shale_kin", "Type II Mesozoic Marine Shale (Kinetics)")},
		{"Type_II_Mesozoic_Marl_kin",					std::make_pair("Type_II_Mesozoic_Marine_Marl_kin", "Type II Mesozoic Marine Marl (Kinetics)")},
		{"Type_II_Paleozoic_MarineShale_kin",			std::make_pair("Type_II_Paleozoic_Marine_Shale_kin", "Type II Paleozoic Marine Shale (Kinetics)")},
		{"Type_III_II_Mesozoic_HumicCoal_lit",			std::make_pair("Type_III_II_Mesozoic_Terrestrial_Humic_Coal_lit", "Type III/II Mesozoic Terrestrial Humic Coal (Literature)")},
		{"Type_III_MesoPaleozoic_VitriniticCoal_kin",	std::make_pair("Type_III_Paleozoic_Terrestrial_Vitrinitic_Coals_kin", "Type III Paleozoic Terrestrial Vitrinitic Coals (Kinetics)")},
		// with Sulfur
		{"Type_I_CenoMesozoic_Lacustrine_kin_s",			std::make_pair("Type_I_CenoMesozoic_Lacustrine_Algal_Shale_kin_s", "Type I - Sulfur - CenoMesozoic Lacustrine Algal Shale (Kinetics)")},
		{"Type_I_II_Mesozoic_MarineShale_lit_s",			std::make_pair("Type_I_II_Mesozoic_Marine_Shale_lit_s", "Type I/II - Sulfur - Mesozoic Marine Shale (Literature)")},
		{"Type_II_Mesozoic_MarineShale_kin_s",			std::make_pair("Type_II_Mesozoic_Marine_Shale_kin_s", "Type II - Sulfur - Mesozoic Marine Shale (Kinetics)")},
		{"Type_II_Mesozoic_Marl_kin_s",					std::make_pair("Type_II_Mesozoic_Marine_Marl_kin_s", "Type II - Sulfur - Mesozoic Marine Marl (Kinetics)")},
		{"Type_II_Paleozoic_MarineShale_kin_s",			std::make_pair("Type_II_Paleozoic_Marine_Shale_kin_s", "Type II - Sulfur - Paleozoic Marine Shale (Kinetics)")},
		{"Type_III_II_Mesozoic_HumicCoal_lit_s", 			std::make_pair("Type_II_III_Mesozoic_Terrestrial_Humic_Coal_lit_s", "Type II/III - Sulfur - Mesozoic Terrestrial Humic Coal (Literature)")},
		{"Type_I_II_Cenozoic_Marl_kin_s", /*this was lit in bpa2*/					std::make_pair("Type_I_II_Cenozoic_Marine_Marl_lit_s", "Type I/II - Sulfur - Cenozoic Marine Marl (Literature)")}
	};
	return m;
};

static Prograde::DefaultValueMap create_map2()
{
	Prograde::DefaultValueMap m =
	{
		// bpaBaseSourceRockNames			//measuredHI	measuredHcVre05	measuredScVre05s	measuredEa	bpaBaseSourceRockAsphalteneDE	bpaBaseSourceRockResinDE	bpaBaseSourceRockC15AroDE	bpaBaseSourceRockC15SatDE
		{"Type_I_CenoMesozoic_Lacustrine_kin",			{747,1.45,0,214,90,83,81,72}},
		{"Type_I_II_Mesozoic_MarineShale_lit",			{656,1.35,0,211,90,87,80,74}},
		{"Type_II_Mesozoic_MarineShale_kin",			{471,1.25,0,210,88,85,80,75}},
		{"Type_II_Mesozoic_Marl_kin",					{471,1.25,0,211,88,85,74,69}},
		{"Type_II_Paleozoic_MarineShale_kin",			{361,1.2,0,212,87,80,77,71}},
		{"Type_III_II_Mesozoic_HumicCoal_lit",			{210,1.03,0,208,90,87,81,76}},
		{"Type_III_MesoPaleozoic_VitriniticCoal_kin",	{94,0.801,0,206,90,87,84,81}},
		// with Sulfur
		{"Type_I_CenoMesozoic_Lacustrine_kin_s",		{823,1.56,0.02,214,90,83,81,72}},
		{"Type_I_II_Mesozoic_MarineShale_lit_s",		{656,1.35,0.02,211,88,85,77,72}},
		{"Type_II_Mesozoic_MarineShale_kin_s",			{444,1.24,0.035,210,88,85,77,72}},
		{"Type_II_Mesozoic_Marl_kin_s",					{444,1.24,0.05,210,87,83,75,71}},
		{"Type_II_Paleozoic_MarineShale_kin_s",			{281,1.13,0.045,210,87,82,78,72}},
		{"Type_III_II_Mesozoic_HumicCoal_lit_s", 		{210,1.03,0.02,208,87,84,79,74}},
		{"Type_I_II_Cenozoic_Marl_kin_s",				{656,1.35,0.05,209,88,85,77,72}}
	};
	return m;
};

Prograde::pairMap const Prograde::SourceRockConverter::SrNameMaps = create_map();

Prograde::DefaultValueMap const Prograde::SourceRockConverter::SrDefValueMap = create_map2();


double Prograde::SourceRockConverter::upgradeHcVre05(const std::string & legacySourceRockType, const std::string & baseSRName, double legacyHcVre05, 
	double legacyScVre05, double & minHI, double & maxHI) const
{
	std::string legacyBaseSourceRockType_{ baseSRName };
	if (legacyScVre05 > 0.0)
		legacyBaseSourceRockType_ = AddTailing_sToSulfurousBpaBaseSrName(baseSRName);

	//auto i = getIndexOfSrMap(legacyBaseSourceRockType_);
	auto measuredHI = GetmeasuredHI(legacyBaseSourceRockType_);
	auto measuredHcVre05 = GetmeasuredHcVre05(legacyBaseSourceRockType_);
	if (minHI > measuredHI) minHI = measuredHI;
	if (maxHI < measuredHI) maxHI = measuredHI;
	if (legacyHcVre05 != measuredHcVre05)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> 'HcVre05' value is identified as " << "'"
			<< legacyHcVre05 << "' which is not equal to measured 'HcVre05' for this base sourcerock."
			<<"Hence, 'HcVre05' value; resetting to measured value of " << measuredHcVre05;
		return measuredHcVre05;
	}
	return legacyHcVre05;
}

double Prograde::SourceRockConverter::upgradeHiVre05(const std::string& legacySourceRockType, const std::string& baseSRName, double legacyHiVre05,
	double legacyScVre05, double& minHI, double& maxHI) const
{
	std::string legacyBaseSourceRockType_{ baseSRName };
	if (legacyScVre05 > 0.0)
		legacyBaseSourceRockType_ = AddTailing_sToSulfurousBpaBaseSrName(baseSRName);

	//auto i = getIndexOfSrMap(legacyBaseSourceRockType_);
	auto measuredHI = GetmeasuredHI(legacyBaseSourceRockType_);
	//auto measuredHcVre05 = GetmeasuredHiVre05(legacyBaseSourceRockType_);
	if (minHI > measuredHI) minHI = measuredHI;
	if (maxHI < measuredHI) maxHI = measuredHI;
	if (legacyHiVre05 != measuredHI)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> 'HiVre05' value is identified as " << "'"
			<< legacyHiVre05 << "' which is not equal to measured 'HiVre05' for this base sourcerock."
			<< "Hence, 'HiVre05' value; resetting to measured value of " << measuredHI;
		return measuredHI;
	}
	return legacyHiVre05;
}



double Prograde::SourceRockConverter::upgradeScVre05(const std::string & legacySourceRockType, 
	const std::string & baseSRName, double legacyScVre05) const
{
	std::string legacyBaseSourceRockType_{ baseSRName };
	if (legacyScVre05 > 0.0)
		legacyBaseSourceRockType_ = AddTailing_sToSulfurousBpaBaseSrName(baseSRName);
	else
		return 0.0;
	//auto i = getIndexOfSrMap(legacyBaseSourceRockType_);
	auto measuredScVre05s = GetmeasuredScVre05s(legacyBaseSourceRockType_);

	if (legacyScVre05 != measuredScVre05s)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> 'ScVre05' value is identified as" << "'"
			<< legacyScVre05 << "' which is not equal to measured 'ScVre05' for this base sourcerock. Hence, 'ScVre05' "<<
			"value is resetting to measured value of " << measuredScVre05s;
		return measuredScVre05s;
	}

	return legacyScVre05;
}
	
double Prograde::SourceRockConverter::upgradeEa(const std::string & legacySourceRockType, 
	const std::string & baseSRName, double legacyEa, double legacyScVre05) const
{
	std::string legacyBaseSourceRockType_{ baseSRName };
	if (legacyScVre05 > 0.0)
		legacyBaseSourceRockType_ = AddTailing_sToSulfurousBpaBaseSrName(baseSRName);

	//auto i = getIndexOfSrMap(legacyBaseSourceRockType_);
	auto measuredEa = GetmeasuredEa(legacyBaseSourceRockType_);
	if (legacyEa != measuredEa)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> 'PreAsphaltStartAct' value is identified as" << "'"
			<< legacyEa << "' which is not equal to measured 'PreAsphaltStartAct'" << 
			"for this base sourcerock. Hence, 'PreAsphaltStartAct' value is resetting to measured value of " << measuredEa;
		return measuredEa;
	}
	return legacyEa;
}

void Prograde::SourceRockConverter::upgradeSourceRockName(const std::string & legacySourceRockType, const std::string & legacyBaseSourceRockType,
	double legacyScVRe05, std::string & bpa2SourceRockType, std::string & bpa2BaseSourceRockType, bool & litFlag)
{
	std::string legacyBaseSourceRockType_{ legacyBaseSourceRockType };
	if (legacyScVRe05 > 0.0)
		legacyBaseSourceRockType_ = AddTailing_sToSulfurousBpaBaseSrName(legacyBaseSourceRockType);
	bpa2BaseSourceRockType = GetBPA2BaseRockName(legacyBaseSourceRockType_);
	bpa2SourceRockType = GetBPA2RockName(legacyBaseSourceRockType_);
	litFlag = isSrFromLiterature(legacyBaseSourceRockType_);

	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Info> Source rock type is prograded from '"<< legacySourceRockType << "' to '" << bpa2SourceRockType << "'";
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Info> Base rock type is prograded from '" << legacyBaseSourceRockType << "' to '" << bpa2BaseSourceRockType << "'";
}
	
void Prograde::SourceRockConverter::upgradeDiffusionEnergy(const std::string & bpaBaseSourceRockType, double ScVre05, double & asphalteneDE, double & resinDE, double & C15AroDE, double & C15SatDE)
{
	std::string legacyBaseSourceRockType_{ bpaBaseSourceRockType };
	if (ScVre05 > 0.0)
		legacyBaseSourceRockType_ = AddTailing_sToSulfurousBpaBaseSrName(bpaBaseSourceRockType);
	
	auto i = getIndexOfSrMap(legacyBaseSourceRockType_);

	auto bpaBaseSourceRockAsphalteneDE = GetbpaBaseSourceRockAsphalteneDE(legacyBaseSourceRockType_);
		//bpaBaseSourceRockAsphalteneDE
	auto bpaBaseSourceRockResinDE = GetbpaBaseSourceRockResinDE(legacyBaseSourceRockType_);
	auto bpaBaseSourceRockC15AroDE = GetbpaBaseSourceRockC15AroDE(legacyBaseSourceRockType_);
	auto bpaBaseSourceRockC15SatDE = GetbpaBaseSourceRockC15SatDE(legacyBaseSourceRockType_);

	if (asphalteneDE != bpaBaseSourceRockAsphalteneDE)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) <<
			"  <Basin-Warning> Asphaltene Diffusion energy field is prograded from " << asphalteneDE << " to " << bpaBaseSourceRockAsphalteneDE;
		asphalteneDE = bpaBaseSourceRockAsphalteneDE;
	}
	if (resinDE != bpaBaseSourceRockResinDE)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) <<
			"  <Basin-Warning> Resin Diffusion energy field is prograded from " << resinDE << " to " << bpaBaseSourceRockResinDE;
		resinDE = bpaBaseSourceRockResinDE;
	}
	if (C15AroDE != bpaBaseSourceRockC15AroDE)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) <<
			"  <Basin-Warning> C15Aro Diffusion energy field is prograded from " << C15AroDE << " to " << bpaBaseSourceRockC15AroDE;
		C15AroDE = bpaBaseSourceRockC15AroDE;
	}
	if (C15SatDE != bpaBaseSourceRockC15SatDE)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) <<
			"  <Basin-Warning> C15Sat Diffusion energy field is prograded from " << C15SatDE << " to " << bpaBaseSourceRockC15SatDE;
		C15SatDE = bpaBaseSourceRockC15SatDE;
	}
	
}

void Prograde::SourceRockConverter::upgradeVESlimit(const std::string & legacyVESlimitIndicator, double & VESlimit)
{
	if (legacyVESlimitIndicator.compare("No") == 0)
	{
		if (VESlimit == Utilities::Numerical::IbsNoDataValue)
		{			
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Info> BPA Cauldron VES limit indicator identified as '" << legacyVESlimitIndicator << "' and VES Limit value is '" << VESlimit << "'";
			VESlimit = 50;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> BPA2-Basin VES limit indicator remain as '" << legacyVESlimitIndicator << "' and VES Limit is set as '" << VESlimit << "'";
		}		
	}
	else if (VESlimit < 0.0)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Info> BPA Cauldron VES limit indicator identified as '" << legacyVESlimitIndicator << "' and VES Limit value is '" << VESlimit << "'";
		VESlimit = 0.0;
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> BPA2-Basin VES limit indicator remain as '" << legacyVESlimitIndicator << "' and VES Limit is set as '" << VESlimit << "'";
	}
	else if (VESlimit > 70.0)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Info> BPA Cauldron VES limit indicator identified as '" << legacyVESlimitIndicator << "' and VES Limit value is '" << VESlimit << "'";
		VESlimit = 70.0;
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> BPA2-Basin VES limit indicator remain as '" << legacyVESlimitIndicator << "' and VES Limit is set as '" << VESlimit << "'";
	} // For all other cases VESlimit will remain same
}

void Prograde::SourceRockConverter::upgradeVREthreshold(const std::string & legacyVREoptimization, double & VREthreshold)
{
	if (legacyVREoptimization.compare("No") == 0)
	{
		if (VREthreshold == Utilities::Numerical::IbsNoDataValue)
		{
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Info> BPA Cauldron VRE optimization identified as '" << legacyVREoptimization << "' and VRE threshold value is '" << VREthreshold << "'";
			VREthreshold = 0.5;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "  <Basin-Warning> BPA2-Basin VRE optimization remain as '" << legacyVREoptimization << "' and VRE threshold is set as '" << VREthreshold << "'";
		}
	}	
}

std::string Prograde::SourceRockConverter::GetBPA2BaseRockName(const std::string& bpaBaseSourceRockName, const std::string* bpaSourceRockName) const
{
	auto bpaBaseSourceRockName_ = bpaBaseSourceRockName;
	if (bpaSourceRockName) {
		if (isBpaSrSulfurous(*bpaSourceRockName)) {
			bpaBaseSourceRockName_ = AddTailing_sToSulfurousBpaBaseSrName(bpaBaseSourceRockName);
		}
	}
	
	auto res = SrNameMaps.find(bpaBaseSourceRockName_);

	if (res != SrNameMaps.end())
	{
		return res->second.first;
	}
	return std::string("Opps you made a mistake! check the name "+ bpaBaseSourceRockName);
}

std::string Prograde::SourceRockConverter::GetBPA2RockName(const std::string& bpaBaseSourceRockName, const std::string* bpaSourceRockName) const
{
	auto bpaBaseSourceRockName_ = bpaBaseSourceRockName;
	if (bpaSourceRockName) {
		if (isBpaSrSulfurous(*bpaSourceRockName)) {
			bpaBaseSourceRockName_ = AddTailing_sToSulfurousBpaBaseSrName(bpaBaseSourceRockName);
		}
	}

	auto res = SrNameMaps.find(bpaBaseSourceRockName_);
	if (res != SrNameMaps.end())
	{
		return res->second.second;
	}
	return std::string("Opps you made a mistake! check the name " + bpaBaseSourceRockName);
}

bool Prograde::SourceRockConverter::isBpaSrSulfurous(const std::string& bpaSourceRockName) const
{
	return !bpaSourceRockName.substr(bpaSourceRockName.find(":") + 1).compare("WithSulphur");
}

bool Prograde::SourceRockConverter::isSrFromLiterature(const std::string& bpaBaseSourceRockName,
	const std::string* bpaSourceRockName) const
{
	auto bpaBaseSourceRockName_ = bpaBaseSourceRockName;
	// this name was wrong in BPA it is supposed to be literature type
	if (!bpaBaseSourceRockName_.compare("Type_I_II_Cenozoic_Marl_kin")) {
		bpaBaseSourceRockName_ = "Type_I_II_Cenozoic_Marl_lit";
	}
	if (bpaSourceRockName) {
		if (isBpaSrSulfurous(*bpaSourceRockName)) {
			bpaBaseSourceRockName_ = AddTailing_sToSulfurousBpaBaseSrName(bpaBaseSourceRockName);
		}
	}
	
	return (!Genex6::tail(bpaBaseSourceRockName_,3).compare("lit")
		|| 
		!Genex6::tail(bpaBaseSourceRockName_, 5).compare("lit_s")
		);
}

bool Prograde::SourceRockConverter::isSrFromLiterature2ndImplementation(const std::string* bpaSourceRockName) const
{
	auto  bpaSr = *bpaSourceRockName;
	auto pos = bpaSr.find("(Kinetics)");

	return(pos == std::string::npos);
}

int Prograde::SourceRockConverter::getIndexOfSrMap(const std::string& bpaBaseSourceRockName, const std::string* bpaSourceRockName)
{
	auto bpaBaseSourceRockName_ = bpaBaseSourceRockName;
	if (bpaSourceRockName) {
		if (isBpaSrSulfurous(*bpaSourceRockName)) {
			bpaBaseSourceRockName_ = AddTailing_sToSulfurousBpaBaseSrName(bpaBaseSourceRockName);
		}
	}
	auto  iter = SrNameMaps.find(bpaBaseSourceRockName_);
	int index = std::distance(SrNameMaps.begin(), iter);
	return index;
}

std::string Prograde::SourceRockConverter::AddTailing_sToSulfurousBpaBaseSrName(const std::string& bpaBaseSourceRockName) const
{
	if (Genex6::tail(bpaBaseSourceRockName, 2).compare("_s")) {
		return (bpaBaseSourceRockName + "_s");
	}
	return bpaBaseSourceRockName;
}

double Prograde::SourceRockConverter::GetmeasuredHI(const std::string& bpaBaseSourceRockName, const std::string* bpaSourceRockName ) const
{
	auto bpaBaseSourceRockName_ = bpaBaseSourceRockName;
	if (bpaSourceRockName) {
		if (isBpaSrSulfurous(*bpaSourceRockName)) {
			bpaBaseSourceRockName_ = AddTailing_sToSulfurousBpaBaseSrName(bpaBaseSourceRockName);
		}
	}

	auto res = SrDefValueMap.find(bpaBaseSourceRockName_);

	if (res != SrDefValueMap.end())
	{
		return res->second[0];
	}
	return 0.0;
}

double Prograde::SourceRockConverter::GetmeasuredHcVre05(const std::string& bpaBaseSourceRockName, const std::string* bpaSourceRockName) const

{
	auto res = SrDefValueMap.find(bpaBaseSourceRockName);

	if (res != SrDefValueMap.end())
	{
		return res->second[1];
	}
	return 0.0;
}

double Prograde::SourceRockConverter::GetmeasuredScVre05s(const std::string& bpaBaseSourceRockName) const
{
	auto res = SrDefValueMap.find(bpaBaseSourceRockName);

	if (res != SrDefValueMap.end())
	{
		return res->second[2];
	}
	return 0.0;
}

double Prograde::SourceRockConverter::GetmeasuredEa(const std::string& bpaBaseSourceRockName, const std::string* bpaSourceRockName) const
{
	auto res = SrDefValueMap.find(bpaBaseSourceRockName);

	if (res != SrDefValueMap.end())
	{
		return res->second[3];
	}
	return 0.0;
}

double Prograde::SourceRockConverter::GetbpaBaseSourceRockAsphalteneDE(const std::string& bpaBaseSourceRockName)
{
	auto res = SrDefValueMap.find(bpaBaseSourceRockName);

	if (res != SrDefValueMap.end())
	{
		return res->second[4];
	}
	return 0.0;
}

double Prograde::SourceRockConverter::GetbpaBaseSourceRockResinDE(const std::string& bpaBaseSourceRockName)
{
	auto res = SrDefValueMap.find(bpaBaseSourceRockName);

	if (res != SrDefValueMap.end())
	{
		return res->second[5];
	}
	return 0.0;;
}

double Prograde::SourceRockConverter::GetbpaBaseSourceRockC15AroDE(const std::string& bpaBaseSourceRockName)
{
	auto res = SrDefValueMap.find(bpaBaseSourceRockName);

	if (res != SrDefValueMap.end())
	{
		return res->second[6];
	}
	return 0.0;
}

double Prograde::SourceRockConverter::GetbpaBaseSourceRockC15SatDE(const std::string& bpaBaseSourceRockName)
{
	auto res = SrDefValueMap.find(bpaBaseSourceRockName);

	if (res != SrDefValueMap.end())
	{
		return res->second[7];
	}
	return 0.0;
}
