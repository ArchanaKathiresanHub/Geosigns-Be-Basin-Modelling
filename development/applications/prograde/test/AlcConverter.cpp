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
	
}