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

#include "BasicCrustThinningModelConverter.h"


//cmbAPI
#include "BottomBoundaryManager.h"


using namespace mbapi;

TEST(BasicCrustThinningModelConverter, upgrade)
{
   Prograde::BasicCrustThinningModelConverter modelConverter;

   EXPECT_EQ(BottomBoundaryManager::AdvancedCrustThinning, modelConverter.upgradeBotBoundModel(BottomBoundaryManager::BasicCrustThinning));
   EXPECT_EQ(BottomBoundaryManager::BaseSedimentHeatFlow, modelConverter.upgradeBotBoundModel(BottomBoundaryManager::BaseSedimentHeatFlow));
   EXPECT_EQ(BottomBoundaryManager::AdvancedCrustThinning, modelConverter.upgradeBotBoundModel(BottomBoundaryManager::AdvancedCrustThinning));

   EXPECT_EQ(BottomBoundaryManager::StandardCondModel, modelConverter.upgradeCrustPropModel(BottomBoundaryManager::LegacyCrust));
   EXPECT_EQ(BottomBoundaryManager::StandardCondModel, modelConverter.upgradeCrustPropModel(BottomBoundaryManager::LowCondModel));
   EXPECT_EQ(BottomBoundaryManager::StandardCondModel, modelConverter.upgradeCrustPropModel(BottomBoundaryManager::StandardCondModel));

   EXPECT_EQ(BottomBoundaryManager::HighCondMnModel, modelConverter.upgradeMantlePropModel(BottomBoundaryManager::LegacyMantle));
   EXPECT_EQ(BottomBoundaryManager::HighCondMnModel, modelConverter.upgradeMantlePropModel(BottomBoundaryManager::LowCondMnModel));
   EXPECT_EQ(BottomBoundaryManager::HighCondMnModel, modelConverter.upgradeMantlePropModel(BottomBoundaryManager::StandardCondMnModel));
   EXPECT_EQ(BottomBoundaryManager::HighCondMnModel, modelConverter.upgradeMantlePropModel(BottomBoundaryManager::HighCondMnModel));

   EXPECT_EQ("ContCrustalThicknessIoTbl", modelConverter.upgradeGridMapTable("CrustIoTbl"));
   EXPECT_EQ("StratIoTbl", modelConverter.upgradeGridMapTable("StratIoTbl"));

}