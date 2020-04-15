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

   EXPECT_EQ(BottomBoundaryManager::CrustPropertyModel::StandardCondModel, modelConverter.upgradeAlcCrustPropModel(BottomBoundaryManager::CrustPropertyModel::LegacyCrust));
   EXPECT_EQ(BottomBoundaryManager::CrustPropertyModel::StandardCondModel, modelConverter.upgradeAlcCrustPropModel(BottomBoundaryManager::CrustPropertyModel::LowCondModel));
   EXPECT_EQ(BottomBoundaryManager::CrustPropertyModel::StandardCondModel, modelConverter.upgradeAlcCrustPropModel(BottomBoundaryManager::CrustPropertyModel::StandardCondModel));

   EXPECT_EQ(BottomBoundaryManager::MantlePropertyModel::HighCondMnModel, modelConverter.upgradeAlcMantlePropModel(BottomBoundaryManager::MantlePropertyModel::LegacyMantle));
   EXPECT_EQ(BottomBoundaryManager::MantlePropertyModel::HighCondMnModel, modelConverter.upgradeAlcMantlePropModel(BottomBoundaryManager::MantlePropertyModel::LowCondMnModel));
   EXPECT_EQ(BottomBoundaryManager::MantlePropertyModel::HighCondMnModel, modelConverter.upgradeAlcMantlePropModel(BottomBoundaryManager::MantlePropertyModel::StandardCondMnModel));
   EXPECT_EQ(BottomBoundaryManager::MantlePropertyModel::HighCondMnModel, modelConverter.upgradeAlcMantlePropModel(BottomBoundaryManager::MantlePropertyModel::HighCondMnModel));
}