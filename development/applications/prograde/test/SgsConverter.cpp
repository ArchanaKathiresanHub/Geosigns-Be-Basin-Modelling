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


#include "../src/SgsConverter.h"
#include "../src/SgsManager.h"

// Adsorption TOC Dependent is on in BPA, set it to off in BPA2
TEST(SgsConverter, AdsorptionTOCDependent)
{
   Prograde::SgsConverter sgsConverter;  
   int legacyAdsorptionTOCDependent = 0;
   int bpa2AdsorptionTOCDependent;
   bpa2AdsorptionTOCDependent = sgsConverter.upgradeAdsorptionTOCDependent(legacyAdsorptionTOCDependent);
   EXPECT_EQ(bpa2AdsorptionTOCDependent, 0);
   legacyAdsorptionTOCDependent = 1;
   bpa2AdsorptionTOCDependent = sgsConverter.upgradeAdsorptionTOCDependent(legacyAdsorptionTOCDependent);
   EXPECT_EQ(bpa2AdsorptionTOCDependent, 0);   
}

// If Adsorption TOC Dependent is on in BPA, set adsorption capacity function name to Low Langmuir Isotherm in BPA2
TEST(SgsConverter, AdsorptionCapacityFunctionName)
{
	Prograde::SgsConverter sgsConverter;
	std::string legacyAdsorptionCapacityFunctionName="TOC";
	std::string bpa2AdsorptionCapacityFunctionName;
	int legacyAdsorptionTOCDependent = 1;
	bpa2AdsorptionCapacityFunctionName = sgsConverter.upgradeAdsorptionCapacityFunctionName(legacyAdsorptionTOCDependent, legacyAdsorptionCapacityFunctionName);
	EXPECT_EQ(bpa2AdsorptionCapacityFunctionName, "Low Langmuir Isotherm");
    legacyAdsorptionTOCDependent = 0;
	legacyAdsorptionCapacityFunctionName = "Langmuir Isotherm";
	bpa2AdsorptionCapacityFunctionName = sgsConverter.upgradeAdsorptionCapacityFunctionName(legacyAdsorptionTOCDependent, legacyAdsorptionCapacityFunctionName);
	EXPECT_EQ(bpa2AdsorptionCapacityFunctionName, "Default Langmuir Isotherm");
	legacyAdsorptionTOCDependent = 0;
	legacyAdsorptionCapacityFunctionName = "user choice";
	bpa2AdsorptionCapacityFunctionName = sgsConverter.upgradeAdsorptionCapacityFunctionName(legacyAdsorptionTOCDependent, legacyAdsorptionCapacityFunctionName);
	EXPECT_EQ(bpa2AdsorptionCapacityFunctionName, "user choice");
	
}

// If user uses low, medium or high IWS data, no changes required else, user defined IWS data will be converted to low IWS data 
TEST(SgsConverter, IrreducibleWaterSaturationCoefficients)
{
	Prograde::SgsConverter sgsConverter;
	double legacyCoefficientA, legacyCoefficientB;
	double bpa2CoefficientA, bpa2CoefficientB;
	legacyCoefficientA = -0.13;
	legacyCoefficientB = 0.7;
	sgsConverter.upgradeIrreducibleWaterSaturationCoefficients(legacyCoefficientA, legacyCoefficientB, bpa2CoefficientA, bpa2CoefficientB);
	EXPECT_EQ(bpa2CoefficientA, legacyCoefficientA);
	EXPECT_EQ(bpa2CoefficientB, legacyCoefficientB);
	legacyCoefficientA = -0.14;
	legacyCoefficientB = 0.8;
	sgsConverter.upgradeIrreducibleWaterSaturationCoefficients(legacyCoefficientA, legacyCoefficientB, bpa2CoefficientA, bpa2CoefficientB);
	EXPECT_EQ(bpa2CoefficientA, legacyCoefficientA);
	EXPECT_EQ(bpa2CoefficientB, legacyCoefficientB);
	legacyCoefficientA = -0.15;
	legacyCoefficientB = 0.9;
	sgsConverter.upgradeIrreducibleWaterSaturationCoefficients(legacyCoefficientA, legacyCoefficientB, bpa2CoefficientA, bpa2CoefficientB);
	EXPECT_EQ(bpa2CoefficientA, legacyCoefficientA);
	EXPECT_EQ(bpa2CoefficientB, legacyCoefficientB);
	legacyCoefficientA = -0.139;
	legacyCoefficientB = 0.9;
	sgsConverter.upgradeIrreducibleWaterSaturationCoefficients(legacyCoefficientA, legacyCoefficientB, bpa2CoefficientA, bpa2CoefficientB);
	EXPECT_EQ(bpa2CoefficientA, -0.13);
	EXPECT_EQ(bpa2CoefficientB, 0.7);
}
