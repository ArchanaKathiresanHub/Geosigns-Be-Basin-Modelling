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

   EXPECT_EQ(BottomBoundaryManager::BottomBoundaryModel::AdvancedCrustThinning, modelConverter.upgradeBotBoundModel(
       BottomBoundaryManager::BottomBoundaryModel::BasicCrustThinning));
   EXPECT_EQ(BottomBoundaryManager::BottomBoundaryModel::BaseSedimentHeatFlow, modelConverter.upgradeBotBoundModel(
       BottomBoundaryManager::BottomBoundaryModel::BaseSedimentHeatFlow));
   EXPECT_EQ(BottomBoundaryManager::BottomBoundaryModel::AdvancedCrustThinning, modelConverter.upgradeBotBoundModel(
       BottomBoundaryManager::BottomBoundaryModel::AdvancedCrustThinning));

   EXPECT_EQ(BottomBoundaryManager::CrustPropertyModel::StandardCondModel, modelConverter.upgradeCrustPropModel(
       BottomBoundaryManager::CrustPropertyModel::LegacyCrust));
   EXPECT_EQ(BottomBoundaryManager::CrustPropertyModel::StandardCondModel, modelConverter.upgradeCrustPropModel(
       BottomBoundaryManager::CrustPropertyModel::LowCondModel));
   EXPECT_EQ(BottomBoundaryManager::CrustPropertyModel::StandardCondModel, modelConverter.upgradeCrustPropModel(
       BottomBoundaryManager::CrustPropertyModel::StandardCondModel));

   EXPECT_EQ(BottomBoundaryManager::MantlePropertyModel::HighCondMnModel, modelConverter.upgradeMantlePropModel(
       BottomBoundaryManager::MantlePropertyModel::LegacyMantle));
   EXPECT_EQ(BottomBoundaryManager::MantlePropertyModel::HighCondMnModel, modelConverter.upgradeMantlePropModel(
       BottomBoundaryManager::MantlePropertyModel::LowCondMnModel));
   EXPECT_EQ(BottomBoundaryManager::MantlePropertyModel::HighCondMnModel, modelConverter.upgradeMantlePropModel(
       BottomBoundaryManager::MantlePropertyModel::StandardCondMnModel));
   EXPECT_EQ(BottomBoundaryManager::MantlePropertyModel::HighCondMnModel, modelConverter.upgradeMantlePropModel(
       BottomBoundaryManager::MantlePropertyModel::HighCondMnModel));

   EXPECT_EQ("ContCrustalThicknessIoTbl", modelConverter.upgradeGridMapTable("CrustIoTbl"));
   EXPECT_EQ("StratIoTbl", modelConverter.upgradeGridMapTable("StratIoTbl"));

}