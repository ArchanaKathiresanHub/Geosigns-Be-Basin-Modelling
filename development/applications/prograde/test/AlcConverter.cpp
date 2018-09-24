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

   EXPECT_EQ(BottomBoundaryManager::StandardCondModel, modelConverter.upgradeAlcCrustPropModel(BottomBoundaryManager::LegacyCrust));
   EXPECT_EQ(BottomBoundaryManager::StandardCondModel, modelConverter.upgradeAlcCrustPropModel(BottomBoundaryManager::LowCondModel));
   EXPECT_EQ(BottomBoundaryManager::StandardCondModel, modelConverter.upgradeAlcCrustPropModel(BottomBoundaryManager::StandardCondModel));

   EXPECT_EQ(BottomBoundaryManager::HighCondMnModel, modelConverter.upgradeAlcMantlePropModel(BottomBoundaryManager::LegacyMantle));
   EXPECT_EQ(BottomBoundaryManager::HighCondMnModel, modelConverter.upgradeAlcMantlePropModel(BottomBoundaryManager::LowCondMnModel));
   EXPECT_EQ(BottomBoundaryManager::HighCondMnModel, modelConverter.upgradeAlcMantlePropModel(BottomBoundaryManager::StandardCondMnModel));
   EXPECT_EQ(BottomBoundaryManager::HighCondMnModel, modelConverter.upgradeAlcMantlePropModel(BottomBoundaryManager::HighCondMnModel));
}