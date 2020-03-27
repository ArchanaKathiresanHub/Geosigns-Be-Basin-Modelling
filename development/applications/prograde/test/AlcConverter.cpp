//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <gtest/gtest.h>

#include "AlcModelConverter.h"


//cmbAPI
#include "BottomBoundaryManager.h"


using namespace mbapi;

TEST(AlcModelConverter, upgrade)
{
	Prograde::AlcModelConverter modelConverter;
	std::string originalALC = "Advanced Lithosphere Calculator";
	std::string text = "Anything_xyz";

	EXPECT_EQ(BottomBoundaryManager::StandardCondModel, modelConverter.upgradeAlcCrustPropModel(BottomBoundaryManager::LegacyCrust));
	EXPECT_EQ(BottomBoundaryManager::StandardCondModel, modelConverter.upgradeAlcCrustPropModel(BottomBoundaryManager::LowCondModel));
	EXPECT_EQ(BottomBoundaryManager::StandardCondModel, modelConverter.upgradeAlcCrustPropModel(BottomBoundaryManager::StandardCondModel));

	EXPECT_EQ(BottomBoundaryManager::HighCondMnModel, modelConverter.upgradeAlcMantlePropModel(BottomBoundaryManager::LegacyMantle));
	EXPECT_EQ(BottomBoundaryManager::HighCondMnModel, modelConverter.upgradeAlcMantlePropModel(BottomBoundaryManager::LowCondMnModel));
	EXPECT_EQ(BottomBoundaryManager::HighCondMnModel, modelConverter.upgradeAlcMantlePropModel(BottomBoundaryManager::StandardCondMnModel));
	EXPECT_EQ(BottomBoundaryManager::HighCondMnModel, modelConverter.upgradeAlcMantlePropModel(BottomBoundaryManager::HighCondMnModel));

	EXPECT_EQ("Improved Lithosphere Calculator Linear Element Mode", modelConverter.updateBottomBoundaryModel(originalALC));
	EXPECT_EQ(text, modelConverter.updateBottomBoundaryModel(text));

	EXPECT_EQ(0, modelConverter.updateTopCrustHeatProd(-50.05));
	EXPECT_EQ(1000, modelConverter.updateTopCrustHeatProd(15200.78));
	EXPECT_EQ(598.89, modelConverter.updateTopCrustHeatProd(598.89));

	EXPECT_EQ(0, modelConverter.updateInitialLithosphericMantleThickness(-89.5));
	EXPECT_EQ(6300000, modelConverter.updateInitialLithosphericMantleThickness(800000000));
	EXPECT_EQ(38850, modelConverter.updateInitialLithosphericMantleThickness(38850));
	
}