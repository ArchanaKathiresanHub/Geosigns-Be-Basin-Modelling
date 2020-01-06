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

//utilities
#include "LogHandler.h"

//cmbAPI
#include "SourceRockManager.h"

#include<map>

using namespace mbapi;

std::vector<std::string>bpaBaseSourceRockWithoutSulphur{
	"Type_I_CenoMesozoic_Lacustrine_kin" ,
	"Type_I_II_Mesozoic_MarineShale_lit" ,
	"Type_II_Mesozoic_MarineShale_kin" ,
	"Type_II_Mesozoic_Marl_kin" ,
	"Type_II_Paleozoic_MarineShale_kin" ,
	"Type_III_II_Mesozoic_HumicCoal_lit" ,
	"Type_III_MesoPaleozoic_VitriniticCoal_kin" };
std::vector<std::string>bpaBaseSourceRockWithSulphur{
	"Type_I_CenoMesozoic_Lacustrine_kin" ,
	"Type_I_II_Mesozoic_MarineShale_lit" ,
	"Type_II_Mesozoic_MarineShale_kin" ,
	"Type_II_Mesozoic_Marl_kin" ,
	"Type_II_Paleozoic_MarineShale_kin" ,
	"Type_III_II_Mesozoic_HumicCoal_lit" ,
	"Type_I_II_Cenozoic_Marl_kin" };

double Prograde::SourceRockConverter::upgradeHcVre05(const std::string & legacySourceRockType, const std::string & baseSRName, double legacyHcVre05, double legacyScVre05)
{
	std::vector<double>measuredHcVre05{ 1.45, 1.35, 1.25, 1.25, 1.2, 1.03, 0.801 };
	std::vector<double>measuredHcVre05s{ 1.56, 1.35, 1.24, 1.24, 1.13, 1.03, 1.35 };

	if (legacyScVre05 > 0.0)
	{
		for (size_t i = 0; i < bpaBaseSourceRockWithSulphur.size(); ++i)
		{

			if (baseSRName.compare(bpaBaseSourceRockWithSulphur[i]) == 0 && legacyHcVre05 != measuredHcVre05s[i])
			{
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "'HcVre05' value is identified as" << "'"
					<< legacyHcVre05 << "' which is not equal to measured 'HcVre05' for this base sourcerock. Hence, 'HcVre05' value is resetting to measured value of " << measuredHcVre05s[i];
				return measuredHcVre05s[i];
			}

		}
	}
	else
	{
		for (size_t i = 0; i < bpaBaseSourceRockWithoutSulphur.size(); ++i)
		{

			if (baseSRName.compare(bpaBaseSourceRockWithoutSulphur[i]) == 0 && legacyHcVre05 != measuredHcVre05[i])
			{
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "'HcVre05' value is identified as" << "'"
					<< legacyHcVre05 << "' which is not equal to measured 'HcVre05' for this base sourcerock. Hence, 'HcVre05' value is resetting to measured value of " << measuredHcVre05[i];
				return measuredHcVre05[i];
			}

		}
	}

	return legacyHcVre05;
}

double Prograde::SourceRockConverter::upgradeScVre05(const std::string & legacySourceRockType, const std::string & baseSRName, double legacyScVre05)
{
	std::vector<double>measuredScVre05s{ 0.02, 0.02, 0.035, 0.05, 0.045, 0.02, 0.05};
	
	if (legacyScVre05 > 0)
	{
		for (size_t i = 0; i < bpaBaseSourceRockWithSulphur.size(); ++i)
		{
			if (baseSRName.compare(bpaBaseSourceRockWithSulphur[i]) == 0 && legacyScVre05 != measuredScVre05s[i])
			{
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "'ScVre05' value is identified as" << "'"
					<< legacyScVre05 << "' which is not equal to measured 'ScVre05' for this base sourcerock. Hence, 'ScVre05' value is resetting to measured value of " << measuredScVre05s[i];
				return measuredScVre05s[i];
			}

		}
	}
	
	return legacyScVre05;
}
	
double Prograde::SourceRockConverter::upgradeEa(const std::string & legacySourceRockType, const std::string & baseSRName, double legacyEa, double legacyScVre05)
{
	std::vector<double>measuredEa{ 214,211,210,211,212,208,206 };
	std::vector<double>measuredEas{ 214,211,210,210,210,208,209 };

	if (legacyScVre05 > 0.0)
	{
		for (size_t i = 0; i < bpaBaseSourceRockWithSulphur.size(); ++i)
		{

			if (baseSRName.compare(bpaBaseSourceRockWithSulphur[i]) == 0 && legacyEa != measuredEas[i])
			{
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "'PreAsphaltStartAct' value is identified as" << "'"
					<< legacyEa << "' which is not equal to measured 'PreAsphaltStartAct' for this base sourcerock. Hence, 'PreAsphaltStartAct' value is resetting to measured value of " << measuredEas[i];
				return measuredEas[i];
			}

		}
	}
	else
	{
		for (size_t i = 0; i < bpaBaseSourceRockWithoutSulphur.size(); ++i)
		{

			if (baseSRName.compare(bpaBaseSourceRockWithoutSulphur[i]) == 0 && legacyEa != measuredEa[i])
			{
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "'PreAsphaltStartAct' value is identified as" << "'"
					<< legacyEa << "' which is not equal to measured 'PreAsphaltStartAct' for this base sourcerock. Hence, 'PreAsphaltStartAct' value is resetting to measured value of " << measuredEa[i];
				return measuredEa[i];
			}

		}
	}
	return legacyEa;
}

void Prograde::SourceRockConverter::upgradeSourceRockName(const std::string & legacySourceRockType, const std::string & legacyBaseSourceRockType, double legacyScVRe05, std::string & bpa2SourceRockType, std::string & bpa2BaseSourceRockType)
{
	std::vector<std::string>bpa2BaseSourceRockNameWithoutSulphur{
		"Type_I_CenoMesozoic_Lacustrine_Algal_Shale_kin" ,
		"Type_I_II_Mesozoic_Marine_Shale_lit" ,
		"Type_II_Mesozoic_Marine_Shale_kin" ,
		"Type_II_Mesozoic_Marine_Marl_kin" ,
		"Type_II_Paleozoic_Marine_Shale_kin" ,
		"Type_III_II_Mesozoic_Terrestrial_Humic_Coal_lit" ,
		"Type_III_Paleozoic_Terrestrial_Vitrinitic_Coals_kin" };
	std::vector<std::string>bpa2SourceRockNameWithoutSulphur{
		"Type I CenoMesozoic Lacustrine Algal Shale (Kinetics)" ,
		"Type I/II Mesozoic Marine Shale (Literature)" ,
		"Type II Mesozoic Marine Shale (Kinetics)" ,
		"Type II Mesozoic Marine Marl (Kinetics)" ,
		"Type II Paleozoic Marine Shale (Kinetics)" ,
		"Type III/II Mesozoic Terrestrial Humic Coal (Literature)" ,
		"Type III Paleozoic Terrestrial Vitrinitic Coals (Kinetics)" };
	std::vector<std::string>bpa2BaseSourceRockNameWithSulphur{
		"Type_I_CenoMesozoic_Lacustrine_Algal_Shale_kin_s" ,
		"Type_I_II_Mesozoic_Marine_Shale_lit_s" ,
		"Type_II_Mesozoic_Marine_Shale_kin_s" ,
		"Type_II_Mesozoic_Marine_Marl_kin_s" ,
		"Type_II_Paleozoic_Marine_Shale_kin_s" ,
		"Type_II_III_Mesozoic_Terrestrial_Humic_Coal_lit_s" ,
		"Type_I_II_Cenozoic_Marine_Marl_lit_s" };
	std::vector<std::string>bpa2SourceRockNameWithSulphur{
		"Type I - Sulfur - CenoMesozoic Lacustrine Algal Shale (Kinetics)" ,
		"Type I/II - Sulfur -Mesozoic Marine Shale (Literature)" ,
		"Type II - Sulfur - Mesozoic Marine Shale (Kinetics)" ,
		"Type II - Sulfur - Mesozoic Marine Marl (Kinetics)" ,
		"Type II - Sulfur - Paleozoic Marine Shale (Kinetics)" ,
		"Type II/III - Sulfur - Mesozoic Terrestrial Humic Coal (Literature)" ,
		"Type I/II - Sulfur -Cenozoic Marine Marl (literature)" };

	if (legacyScVRe05 > 0.0)
	{
		for (size_t i = 0; i < bpaBaseSourceRockWithSulphur.size(); ++i)
		{
			if (legacyBaseSourceRockType.compare(bpaBaseSourceRockWithSulphur[i]) == 0)
			{
				bpa2BaseSourceRockType = bpa2BaseSourceRockNameWithSulphur[i];
				bpa2SourceRockType = bpa2SourceRockNameWithSulphur[i];
				break;
			}

		}

	}
	else
	{
		for (size_t i = 0; i < bpaBaseSourceRockWithoutSulphur.size(); ++i)
		{
			if (legacyBaseSourceRockType.compare(bpaBaseSourceRockWithoutSulphur[i]) == 0)
			{
				bpa2BaseSourceRockType = bpa2BaseSourceRockNameWithoutSulphur[i];
				bpa2SourceRockType = bpa2SourceRockNameWithoutSulphur[i];
				break;
			}

		}
	}
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Source rock type is prograded to '" << bpa2SourceRockType << "'";
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Base source rock type is prograded to '" << bpa2BaseSourceRockType << "'";
}
	
void Prograde::SourceRockConverter::upgradeDiffusionEnergy(const std::string & bpaBaseSourceRockType, double ScVre05, double & asphalteneDE, double & resinDE, double & C15AroDE, double & C15SatDE)
{
	std::vector<double> bpaBaseSourceRockAsphalteneDE{ 90,90,88,88,87,90,90 };
	std::vector<double> bpaBaseSourceRockAsphalteneDEs{ 90,88,88,87,87,87,88 };
	std::vector<double> bpaBaseSourceRockResinDE{ 83,87,85,85,80,87,87 };
	std::vector<double> bpaBaseSourceRockResinDEs{ 83,85,85,83,82,84,85 };
	std::vector<double> bpaBaseSourceRockC15AroDE{ 81,80,80,74,77,81,84 };
	std::vector<double> bpaBaseSourceRockC15AroDEs{ 81,77,77,75,78,79,77 };
	std::vector<double> bpaBaseSourceRockC15SatDE{ 72,74,75,69,71,76,81 };
	std::vector<double> bpaBaseSourceRockC15SatDEs{ 72,72,72,71,72,74,72 };
	if (ScVre05 > 0.0)
	{
		for (size_t i = 0; i < bpaBaseSourceRockAsphalteneDEs.size(); ++i)
		{
			if (bpaBaseSourceRockType.compare(bpaBaseSourceRockWithSulphur[i]) == 0)
			{
				if (asphalteneDE != bpaBaseSourceRockAsphalteneDEs[i])
				{
					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Asphaltene Diffusion energy field is prograded from " << asphalteneDE << " to "  << bpaBaseSourceRockAsphalteneDEs[i];
					asphalteneDE = bpaBaseSourceRockAsphalteneDEs[i];
				}
				if (resinDE != bpaBaseSourceRockResinDEs[i])
				{
					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Resin Diffusion energy field is prograded from " << resinDE << " to "  << bpaBaseSourceRockResinDEs[i];
					resinDE = bpaBaseSourceRockResinDEs[i];
				}
				if (C15AroDE != bpaBaseSourceRockC15AroDEs[i])
				{
					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "C15Aro Diffusion energy field is prograded from " << C15AroDE << " to " << bpaBaseSourceRockC15AroDEs[i];
					C15AroDE = bpaBaseSourceRockC15AroDEs[i];
				}
				if (C15SatDE != bpaBaseSourceRockC15SatDEs[i])
				{
					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "C15Sat Diffusion energy field is prograded from " << C15SatDE << " to " << bpaBaseSourceRockC15SatDEs[i];
					C15SatDE = bpaBaseSourceRockC15SatDEs[i];
				}

			}
		}
	}
	else
	{
		for (size_t i = 0; i < bpaBaseSourceRockAsphalteneDE.size(); ++i)
		{
			if (bpaBaseSourceRockType.compare(bpaBaseSourceRockWithoutSulphur[i]) == 0)
			{
				if (asphalteneDE != bpaBaseSourceRockAsphalteneDE[i])
				{
					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Asphaltene Diffusion energy field is prograded from " << asphalteneDE << " to " << bpaBaseSourceRockAsphalteneDE[i];
					asphalteneDE = bpaBaseSourceRockAsphalteneDE[i];
				}
				if (resinDE != bpaBaseSourceRockResinDE[i])
				{
					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Resin Diffusion energy field is prograded from " << resinDE << " to " << bpaBaseSourceRockResinDE[i];
					resinDE = bpaBaseSourceRockResinDE[i];
				}
				if (C15AroDE != bpaBaseSourceRockC15AroDE[i])
				{
					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "C15Aro Diffusion energy field is prograded from " << C15AroDE << " to " << bpaBaseSourceRockC15AroDE[i];
					C15AroDE = bpaBaseSourceRockC15AroDE[i];
				}
				if (C15SatDE != bpaBaseSourceRockC15SatDE[i])
				{
					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "C15Sat Diffusion energy field is prograded from " << C15SatDE << " to " << bpaBaseSourceRockC15SatDE[i];
					C15SatDE = bpaBaseSourceRockC15SatDE[i];
				}

			}
		}
	}
}

void Prograde::SourceRockConverter::upgradeVESlimit(const std::string & legacyVESlimitIndicator, double & VESlimit)
{
	if (legacyVESlimitIndicator.compare("No") == 0)
	{
		if (VESlimit == -9999)
		{			
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "BPA Cauldron VES limit indicator identified as '" << legacyVESlimitIndicator << "' and VES Limit value is '" << VESlimit << "'";
			VESlimit = 50;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "BPA2-Basin VES limit indicator remain as '" << legacyVESlimitIndicator << "' and VES Limit is set as '" << VESlimit << "'";
		}		
	}
	else if (VESlimit < 0.0)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "BPA Cauldron VES limit indicator identified as '" << legacyVESlimitIndicator << "' and VES Limit value is '" << VESlimit << "'";
		VESlimit = 0.0;
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "BPA2-Basin VES limit indicator remain as '" << legacyVESlimitIndicator << "' and VES Limit is set as '" << VESlimit << "'";
	}
	else if (VESlimit > 70.0)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "BPA Cauldron VES limit indicator identified as '" << legacyVESlimitIndicator << "' and VES Limit value is '" << VESlimit << "'";
		VESlimit = 70.0;
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "BPA2-Basin VES limit indicator remain as '" << legacyVESlimitIndicator << "' and VES Limit is set as '" << VESlimit << "'";
	} // For all other cases VESlimit will remain same
}

void Prograde::SourceRockConverter::upgradeVREthreshold(const std::string & legacyVREoptimization, double & VREthreshold)
{
	if (legacyVREoptimization.compare("No") == 0)
	{
		if (VREthreshold == -9999)
		{
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "BPA Cauldron VRE optimization identified as '" << legacyVREoptimization << "' and VRE threshold value is '" << VREthreshold << "'";
			VREthreshold = 0.5;
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "BPA2-Basin VRE optimization remain as '" << legacyVREoptimization << "' and VRE threshold is set as '" << VREthreshold << "'";
		}
	}	
}