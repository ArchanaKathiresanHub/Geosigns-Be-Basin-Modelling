//
// Copyright (C) 2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell .
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

//
#include <gtest/gtest.h>


#include "../src/SourceRockConverter.h"
#include "../src/SourceRockManager.h"

// Adsorption TOC Dependent is on in BPA, set it to off in BPA2
Prograde::SourceRockConverter sourceRockConverter;
std::vector<std::string>bpaBaseSourceRockNameWithoutSulphur{
	"Type_I_CenoMesozoic_Lacustrine_kin" ,
	"Type_I_II_Mesozoic_MarineShale_lit" ,
	"Type_II_Mesozoic_MarineShale_kin" ,
	"Type_II_Mesozoic_Marl_kin" ,
	"Type_II_Paleozoic_MarineShale_kin" ,
	"Type_III_II_Mesozoic_HumicCoal_lit" ,
	"Type_III_MesoPaleozoic_VitriniticCoal_kin" };
std::vector<std::string>bpaBaseSourceRockNameWithSulphur{
	"Type_I_CenoMesozoic_Lacustrine_kin" ,
	"Type_I_II_Mesozoic_MarineShale_lit" ,
	"Type_II_Mesozoic_MarineShale_kin" ,
	"Type_II_Mesozoic_Marl_kin" ,
	"Type_II_Paleozoic_MarineShale_kin" ,
	"Type_III_II_Mesozoic_HumicCoal_lit" ,
	"Type_I_II_Cenozoic_Marl_kin" };
TEST(SourceRockConverter, HcVre05)
{	
	std::vector<double>measuredHcVre05{ 1.45, 1.35, 1.25, 1.25, 1.2, 1.03, 0.801 };
	std::vector<double>measuredHI{ 747, 656, 471, 471, 361, 210, 94 };
	std::vector<double>measuredHcVre05s{ 1.56, 1.35, 1.24, 1.24, 1.13, 1.03, 1.35 };
	std::vector<double>measuredHIs{ 823, 656, 444, 444, 281, 210, 656 };
	double minHI = 9999.0; 
	double maxHI = -9999.0;
	double hcVre05;
	for (size_t i = 0; i < bpaBaseSourceRockNameWithoutSulphur.size(); ++i)
	{		
		hcVre05 = sourceRockConverter.upgradeHcVre05("Type_II_Mesozoic_Marl_kin", bpaBaseSourceRockNameWithoutSulphur[i], 1.1, 0.0, minHI, maxHI);
		EXPECT_EQ(hcVre05, measuredHcVre05[i]);		
	}
	for (size_t i = 0; i < bpaBaseSourceRockNameWithSulphur.size(); ++i)
	{
		hcVre05 = sourceRockConverter.upgradeHcVre05("Type_II_Mesozoic_Marl_kin", bpaBaseSourceRockNameWithSulphur[i], 1.1, 0.2, minHI, maxHI);
		EXPECT_EQ(hcVre05, measuredHcVre05s[i]);		
	}
	EXPECT_EQ(minHI, 94);
	EXPECT_EQ(maxHI, 823);
}

TEST(SourceRockConverter, ScVre05)
{
	std::vector<double>measuredScVre05{ 0.02, 0.02, 0.035, 0.05, 0.045, 0.02, 0.05 };
	for (size_t i = 0; i < bpaBaseSourceRockNameWithSulphur.size(); ++i)
	{
		double scVre05 = sourceRockConverter.upgradeScVre05("Type_II_Mesozoic_Marl_kin", bpaBaseSourceRockNameWithSulphur[i], 1.1);
		EXPECT_EQ(scVre05, measuredScVre05[i]);
	}
}

TEST(SourceRockConverter, PreAsphaltStartAct)
{
	std::vector<double>measuredEa{ 214,211,210,211,212,208,206 };
	std::vector<double>measuredEas{ 214,211,210,210,210,208,209 };
	for (size_t i = 0; i < bpaBaseSourceRockNameWithSulphur.size(); ++i)
	{
		double ea = sourceRockConverter.upgradeEa( "Type_II_Mesozoic_Marl_kin", bpaBaseSourceRockNameWithSulphur[i], 1.1, 1.2);
		EXPECT_EQ(ea, measuredEas[i]);
	}
	for (size_t i = 0; i < bpaBaseSourceRockNameWithoutSulphur.size(); ++i)
	{
		double ea = sourceRockConverter.upgradeEa( "Type_II_Mesozoic_Marl_kin", bpaBaseSourceRockNameWithoutSulphur[i], 1.1, 0.0);
		EXPECT_EQ(ea, measuredEa[i]);
	}
}

TEST(SourceRockConverter, upgradeSourceRockName)
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
		"Type I/II - Sulfur - Mesozoic Marine Shale (Literature)" ,
		"Type II - Sulfur - Mesozoic Marine Shale (Kinetics)" ,
		"Type II - Sulfur - Mesozoic Marine Marl (Kinetics)" ,
		"Type II - Sulfur - Paleozoic Marine Shale (Kinetics)" ,
		"Type II/III - Sulfur - Mesozoic Terrestrial Humic Coal (Literature)" ,
		"Type I/II - Sulfur - Cenozoic Marine Marl (Literature)" };
	bool litFlag = 0;
	for (size_t i = 0; i < bpaBaseSourceRockNameWithoutSulphur.size(); ++i)
	{
		std::string SourceRockType;
		std::string bpa2SourceRockType;
		std::string bpa2BaseSourceRockType;
		sourceRockConverter.upgradeSourceRockName(SourceRockType, bpaBaseSourceRockNameWithoutSulphur[i], 0.0, 
			bpa2SourceRockType, bpa2BaseSourceRockType, litFlag);
		EXPECT_EQ(bpa2SourceRockType, bpa2SourceRockNameWithoutSulphur[i]);
		EXPECT_EQ(bpa2BaseSourceRockType, bpa2BaseSourceRockNameWithoutSulphur[i]);						
	}
	for (size_t i = 0; i < bpaBaseSourceRockNameWithSulphur.size(); ++i)
	{
		std::string SourceRockType;
		std::string bpa2SourceRockType;
		std::string bpa2BaseSourceRockType;
		sourceRockConverter.upgradeSourceRockName(SourceRockType, bpaBaseSourceRockNameWithSulphur[i], 1.2, 
			bpa2SourceRockType, bpa2BaseSourceRockType, litFlag);
		EXPECT_EQ(bpa2SourceRockType, bpa2SourceRockNameWithSulphur[i]);
		EXPECT_EQ(bpa2BaseSourceRockType, bpa2BaseSourceRockNameWithSulphur[i]);
	}
}

TEST(SourceRockConverter, testLitSourceRockName)
{
	const std::string BSrWithoutSulfur = "Type_I_II_Mesozoic_MarineShale_lit";
	std::string SourceRockType;
	std::string bpa2SourceRockType;
	std::string bpa2BaseSourceRockType;
	bool litFlag = 0;
	sourceRockConverter.upgradeSourceRockName(SourceRockType, BSrWithoutSulfur, 0.0,
		bpa2SourceRockType, bpa2BaseSourceRockType, litFlag);
	
	EXPECT_EQ(litFlag, 1);
	litFlag = 0;
	
	std::string BSrWithSulfur = "Type_III_II_Mesozoic_HumicCoal_lit_s";
	sourceRockConverter.upgradeSourceRockName(SourceRockType, BSrWithSulfur, 1.2,
		bpa2SourceRockType, bpa2BaseSourceRockType, litFlag);
	EXPECT_EQ(litFlag, 1);
	litFlag = 0;

	BSrWithSulfur = "Type_III_II_Mesozoic_HumicCoal_lit";
	sourceRockConverter.upgradeSourceRockName(SourceRockType, BSrWithSulfur, 1.2,
		bpa2SourceRockType, bpa2BaseSourceRockType, litFlag);
	EXPECT_EQ(litFlag, 1);
}

TEST(SourceRockConverter, upgradeDiffusionEnergy)
{
	std::vector<double> asphalteneDE{90, 90, 88, 88, 87, 90, 90};
	std::vector<double> asphalteneDEs{90, 88, 88, 87, 87, 87, 88};	

	std::vector<double> resinDE{83, 87, 85, 85, 80, 87, 87};
	std::vector<double> resinDEs{83, 85, 85, 83, 82, 84, 85};

	std::vector<double> C15AroDE{81, 80, 80, 74, 77, 81, 84};
	std::vector<double> C15AroDEs{81, 77, 77, 75, 78, 79, 77};

	std::vector<double> C15SatDE{72, 74, 75, 69, 71, 76, 81};
	std::vector<double> C15SatDEs{72, 72, 72, 71, 72, 74, 72};	
	
	for (size_t i = 0; i < bpaBaseSourceRockNameWithoutSulphur.size(); ++i)
	{
		double bpaAsphalteneDE = 10;
		double bpaResinDE = 10;
		double bpaC15AroDE = 10;
		double bpaC15SatDE = 10;
		//std::cerr << "[          ] random seed = " << bpaBaseSourceRockNameWithSulphur[i] << std::endl;
		sourceRockConverter.upgradeDiffusionEnergy(bpaBaseSourceRockNameWithoutSulphur[i], 0.0, bpaAsphalteneDE, bpaResinDE, bpaC15AroDE, bpaC15SatDE);
		EXPECT_EQ(bpaAsphalteneDE, asphalteneDE[i]);
		EXPECT_EQ(bpaResinDE, resinDE[i]);
		EXPECT_EQ(bpaC15AroDE, C15AroDE[i]);
		EXPECT_EQ(bpaC15SatDE, C15SatDE[i]);

		bpaAsphalteneDE = asphalteneDE[i];
		bpaResinDE = resinDE[i];
		bpaC15AroDE = C15AroDE[i];
		bpaC15SatDE = C15SatDE[i];
		sourceRockConverter.upgradeDiffusionEnergy(bpaBaseSourceRockNameWithoutSulphur[i], 0.0, bpaAsphalteneDE, bpaResinDE, bpaC15AroDE, bpaC15SatDE);
		EXPECT_EQ(bpaAsphalteneDE, asphalteneDE[i]);
		EXPECT_EQ(bpaResinDE, resinDE[i]);
		EXPECT_EQ(bpaC15AroDE, C15AroDE[i]);
		EXPECT_EQ(bpaC15SatDE, C15SatDE[i]);
	}
	for (size_t i = 0; i < bpaBaseSourceRockNameWithSulphur.size(); ++i)
	{
		double bpaAsphalteneDEs = 10;
		double bpaResinDEs = 10;
		double bpaC15AroDEs = 10;
		double bpaC15SatDEs = 10;
		//std::cerr << "[          ] random seed = " << bpaBaseSourceRockNameWithSulphur[i] << std::endl;
		sourceRockConverter.upgradeDiffusionEnergy(bpaBaseSourceRockNameWithSulphur[i], 1.2, bpaAsphalteneDEs, bpaResinDEs, bpaC15AroDEs, bpaC15SatDEs);
		EXPECT_EQ(bpaAsphalteneDEs, asphalteneDEs[i]);
		EXPECT_EQ(bpaResinDEs, resinDEs[i]);
		EXPECT_EQ(bpaC15AroDEs, C15AroDEs[i]);
		EXPECT_EQ(bpaC15SatDEs, C15SatDEs[i]);

		bpaAsphalteneDEs = asphalteneDEs[i];
		bpaResinDEs = resinDEs[i];
		bpaC15AroDEs = C15AroDEs[i];
		bpaC15SatDEs = C15SatDEs[i];
		sourceRockConverter.upgradeDiffusionEnergy(bpaBaseSourceRockNameWithSulphur[i], 1.2, bpaAsphalteneDEs, bpaResinDEs, bpaC15AroDEs, bpaC15SatDEs);
		EXPECT_EQ(bpaAsphalteneDEs, asphalteneDEs[i]);
		EXPECT_EQ(bpaResinDEs, resinDEs[i]);
		EXPECT_EQ(bpaC15AroDEs, C15AroDEs[i]);
		EXPECT_EQ(bpaC15SatDEs, C15SatDEs[i]);
	}
}


TEST(SourceRockConverter, upgradeVREthreshold)
{
	std::string legacyVREoptimization = "No";
	double VREthreshold = -9999;
	sourceRockConverter.upgradeVREthreshold(legacyVREoptimization, VREthreshold);
	EXPECT_EQ(legacyVREoptimization, "No");
	EXPECT_EQ(VREthreshold, 0.5);

	legacyVREoptimization = "No";
	VREthreshold = 0.5;
	sourceRockConverter.upgradeVREthreshold(legacyVREoptimization, VREthreshold);
	EXPECT_EQ(legacyVREoptimization, "No");
	EXPECT_EQ(VREthreshold, 0.5);

	legacyVREoptimization = "Yes";
	VREthreshold = 0.6;
	sourceRockConverter.upgradeVREthreshold(legacyVREoptimization, VREthreshold);
	EXPECT_EQ(legacyVREoptimization, "Yes");
	EXPECT_EQ(VREthreshold, 0.6);

	legacyVREoptimization = "Yes";
	VREthreshold = 0.4;
	sourceRockConverter.upgradeVREthreshold(legacyVREoptimization, VREthreshold);
	EXPECT_EQ(legacyVREoptimization, "Yes");
	EXPECT_EQ(VREthreshold, 0.4);

	legacyVREoptimization = "Yes";
	VREthreshold = 0.5;
	sourceRockConverter.upgradeVREthreshold(legacyVREoptimization, VREthreshold);
	EXPECT_EQ(legacyVREoptimization, "Yes");
	EXPECT_EQ(VREthreshold, 0.5);
}


TEST(SourceRockConverter, upgradeVESlimit)
{
	std::string legacyVESlimitIndicator = "No";
	double VESlimit = 50;
	sourceRockConverter.upgradeVESlimit(legacyVESlimitIndicator, VESlimit);
	EXPECT_EQ(legacyVESlimitIndicator, "No");
	EXPECT_EQ(VESlimit, 50);

	legacyVESlimitIndicator = "No";
	VESlimit = 10;
	sourceRockConverter.upgradeVESlimit(legacyVESlimitIndicator, VESlimit);
	EXPECT_EQ(legacyVESlimitIndicator, "No");
	EXPECT_EQ(VESlimit, 10);

	legacyVESlimitIndicator = "No";
	VESlimit = -9999;
	sourceRockConverter.upgradeVESlimit(legacyVESlimitIndicator, VESlimit);
	EXPECT_EQ(legacyVESlimitIndicator, "No");
	EXPECT_EQ(VESlimit, 50);

	legacyVESlimitIndicator = "Yes";
	VESlimit = -10;
	sourceRockConverter.upgradeVESlimit(legacyVESlimitIndicator, VESlimit);
	EXPECT_EQ(legacyVESlimitIndicator, "Yes");
	EXPECT_EQ(VESlimit, 0.0);

	legacyVESlimitIndicator = "Yes";
	VESlimit = 100;
	sourceRockConverter.upgradeVESlimit(legacyVESlimitIndicator, VESlimit);
	EXPECT_EQ(legacyVESlimitIndicator, "Yes");
	EXPECT_EQ(VESlimit, 70.0);

	legacyVESlimitIndicator = "Yes";
	VESlimit = 60;
	sourceRockConverter.upgradeVESlimit(legacyVESlimitIndicator, VESlimit);
	EXPECT_EQ(legacyVESlimitIndicator, "Yes");
	EXPECT_EQ(VESlimit, 60.0);
}

TEST(SourceRockConverter, upgradeHCEalimit)
{
	std::string bpaBaseSR = "Type_I_CenoMesozoic_Lacustrine_kin";
	double HcVal = 1.0;
	double EaVal = 200;
	double ScVre = 0;
	sourceRockConverter.limitHcEa(bpaBaseSR, HcVal, EaVal, ScVre);
	EXPECT_EQ(HcVal, 1.24);
	EXPECT_EQ(EaVal, 212);

	bpaBaseSR = "Type_I_CenoMesozoic_Lacustrine_kin";
	HcVal = 1.6;
	EaVal = 250;
	ScVre = 0;
	sourceRockConverter.limitHcEa(bpaBaseSR, HcVal, EaVal, ScVre);
	EXPECT_EQ(HcVal, 1.479);
	EXPECT_EQ(EaVal, 215);

	bpaBaseSR = "Type_I_CenoMesozoic_Lacustrine_kin";
	HcVal = 1.48;
	EaVal = 215.1;
	ScVre = 0;
	sourceRockConverter.limitHcEa(bpaBaseSR, HcVal, EaVal, ScVre);
	EXPECT_EQ(HcVal, 1.479);
	EXPECT_EQ(EaVal, 215);

	bpaBaseSR = "Type_I_CenoMesozoic_Lacustrine_kin";
	HcVal = 1.28;
	EaVal = 215.1;
	ScVre = 0.1;
	sourceRockConverter.limitHcEa(bpaBaseSR, HcVal, EaVal, ScVre);
	EXPECT_EQ(HcVal, 1.3);
	EXPECT_EQ(EaVal, 214);

	bpaBaseSR = "Type_II_Paleozoic_MarineShale_kin";
	HcVal = 0.2;
	EaVal = 300;
	ScVre = 0;
	sourceRockConverter.limitHcEa(bpaBaseSR, HcVal, EaVal, ScVre);
	EXPECT_EQ(HcVal, 0.9);
	EXPECT_EQ(EaVal, 212);

	bpaBaseSR = "Type_I_II_Mesozoic_MarineShale_lit";
	HcVal = 1.51;
	EaVal = 199.9;
	ScVre = 0.6;
	sourceRockConverter.limitHcEa(bpaBaseSR, HcVal, EaVal, ScVre);
	EXPECT_EQ(HcVal, 1.499);
	EXPECT_EQ(EaVal, 209);

	bpaBaseSR = "Type_II_Mesozoic_Marl_kin";
	HcVal = 1.41;
	EaVal = 213;
	ScVre = 0;
	sourceRockConverter.limitHcEa(bpaBaseSR, HcVal, EaVal, ScVre);
	EXPECT_EQ(HcVal, 1.4);
	EXPECT_EQ(EaVal, 212);

	bpaBaseSR = "Type_II_Mesozoic_Marl_kin";
	HcVal = 1.42;
	EaVal = 212;
	ScVre = 0.1;
	sourceRockConverter.limitHcEa(bpaBaseSR, HcVal, EaVal, ScVre);
	EXPECT_EQ(HcVal, 1.41);
	EXPECT_EQ(EaVal, 211);

}
