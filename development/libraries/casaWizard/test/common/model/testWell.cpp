//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <gtest/gtest.h>

#include "model/well.h"

using namespace casaWizard;

TEST(WellTest, testRemovingDataBelowDepth)
{
  // Given
  Well well(0, "Well1", 0.0, 0.0);
  well.addCalibrationTarget("ValidTarget", "DT", 500, 300);
  well.addCalibrationTarget("invalidTarget1", "DT", 8000, 300);
  well.addCalibrationTarget("invalidTarget2", "DT", 4000, 300);
  well.addCalibrationTarget("invalidTarget3", "DT", 6000, 300);
  EXPECT_EQ(well.calibrationTargets().size(), 4);

  // When
  well.removeDataBelowDepth(3500);

  // Then
  EXPECT_EQ(well.calibrationTargets().size(), 1);
  EXPECT_EQ(well.calibrationTargets()[0]->name(), "ValidTarget");
}

TEST(WellTest, testRemovingDataAboveDepth)
{
  // Given
  Well well(0, "Well1", 0.0, 0.0);
  well.addCalibrationTarget("ValidTarget", "DT", 500, 300);
  well.addCalibrationTarget("invalidTarget1", "DT", 10, 300);
  well.addCalibrationTarget("invalidTarget2", "DT", -30, 300);
  well.addCalibrationTarget("invalidTarget3", "DT", -100, 300);
  EXPECT_EQ(well.calibrationTargets().size(), 4);

  // When
  well.removeDataAboveDepth(15);

  // Then
  EXPECT_EQ(well.calibrationTargets().size(), 1);
  EXPECT_EQ(well.calibrationTargets()[0]->name(), "ValidTarget");
}

TEST(WellTest, testRemovingCalibrationTargetsWithPropertyUserName)
{
  // Given
  Well well(0, "Well1", 0.0, 0.0);
  well.addCalibrationTarget("invalidTarget1", "DT", 600, 300);
  well.addCalibrationTarget("invalidTarget2", "DT", 700, 300);
  well.addCalibrationTarget("invalidTarget3", "DT", 500, 300);
  well.addCalibrationTarget("ValidTarget", "TWTT", 800, 300);
  EXPECT_EQ(well.calibrationTargets().size(), 4);

  // When
  well.removeCalibrationTargetsWithPropertyUserName("DT");

  // Then
  EXPECT_EQ(well.calibrationTargets().size(), 1);
  EXPECT_EQ(well.calibrationTargets()[0]->name(), "ValidTarget");
}

TEST(WellTest, testCalibrationTargetsWithPropertyUserName)
{
  // Given
  Well well(0, "Well1", 0.0, 0.0);
  well.addCalibrationTarget("target1", "DT", 600, 300);
  well.addCalibrationTarget("target2", "DT", 700, 300);
  well.addCalibrationTarget("target3", "DT", 500, 300);
  well.addCalibrationTarget("otherTarget", "TWTT", 800, 300);
  EXPECT_EQ(well.calibrationTargets().size(), 4);

  // When
  const QVector<const CalibrationTarget*> targets = well.calibrationTargetsWithPropertyUserName("DT");

  // Then
  EXPECT_EQ(targets.size(), 3);
}


TEST(WellTest, testGetActivePropertyTargets)
{
  // Given
  Well well(0, "Well1", 0.0, 0.0);
  well.addCalibrationTarget("target1", "Temperature", 600, 300);
  well.addCalibrationTarget("target2", "Temperature", 700, 300);
  well.addCalibrationTarget("target3", "Temperature", 500, 300);
  well.addCalibrationTarget("otherTarget", "VRe", 800, 300);
  well.addCalibrationTarget("otherTarget", "VRe", 900, 300);

  EXPECT_EQ(well.calibrationTargets().size(), 5);

  well.setPropertyActive("VRe", false);

  // Then
  EXPECT_EQ(well.activePropertyTargets().size(), 3);
}

