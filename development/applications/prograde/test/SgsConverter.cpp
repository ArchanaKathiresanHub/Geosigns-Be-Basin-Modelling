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


// If OTGC is off, set WhichAdsorptionSimulator to C1AdsorptionSimulator
TEST(SgsConverter, WhichAdsorptionSimulator)
{
	Prograde::SgsConverter sgsConverter;	
	std::string bpa2WhichAdsorptionSimulator;
	int legacyComputeOTGC = 0;
	bpa2WhichAdsorptionSimulator = sgsConverter.upgradeWhichAdsorptionSimulator(legacyComputeOTGC);
	EXPECT_EQ(bpa2WhichAdsorptionSimulator, "C1AdsorptionSimulator");
        legacyComputeOTGC = 1;
	bpa2WhichAdsorptionSimulator = sgsConverter.upgradeWhichAdsorptionSimulator(legacyComputeOTGC);
	EXPECT_EQ(bpa2WhichAdsorptionSimulator, "OTGCC1AdsorptionSimulator"); 
}
