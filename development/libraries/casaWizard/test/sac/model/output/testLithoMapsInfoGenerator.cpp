//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "expectFileEq.h"
#include "model/input/cmbProjectReader.h"
#include "model/output/LithoMapsInfoGenerator.h"
#include "model/sacScenario.h"

#include <gtest/gtest.h>

using namespace casaWizard::sac;

TEST( LithoMapsInfoGenerator, testInfoGenerator )
{
  // Given
  std::remove("infoLithoMapsActual.txt");
  casaWizard::ProjectReader* lithoMapsProjectReader = new casaWizard::CMBProjectReader();
  lithoMapsProjectReader->load("LithoMapsProject.project3d");
  SACScenario scenario(lithoMapsProjectReader);

  scenario.setPIDW(3);
  scenario.setSmoothingOption(1);
  scenario.setRadiusSmoothing(5000);

  // Set Wells
  scenario.calibrationTargetManager().addWell("10_AML2_AV", 184550.00, 608300.00);
  scenario.calibrationTargetManager().addWell("11_AMN1_AV", 192000.00, 615000.00);
  scenario.calibrationTargetManager().addWell("12_AMN2_AV", 191500.00, 615750.00);
  scenario.calibrationTargetManager().setWellIsActive(false, 0);
  scenario.calibrationTargetManager().setWellIsExcluded(true, 1);

  // Set Objective Function
  scenario.calibrationTargetManager().addCalibrationTarget("Name1", "SonicSlowness", 0, 25.0, 20.0);
  scenario.calibrationTargetManager().addCalibrationTarget("Name1", "SonicSlowness", 1, 30.0, 20.0);
  scenario.calibrationTargetManager().addCalibrationTarget("Name2", "TwoWayTime", 0, 25.0, 20.0);
  scenario.calibrationTargetManager().addCalibrationTarget("Name2", "TwoWayTime", 1, 30.0, 20.0);
  scenario.calibrationTargetManager().setObjectiveFunction(0,0, 50);
  scenario.calibrationTargetManager().setObjectiveFunction(0,1, 0);
  scenario.calibrationTargetManager().setObjectiveFunction(1,0, 1);
  scenario.calibrationTargetManager().setObjectiveFunction(1,1, 0.05);
  scenario.calibrationTargetManager().updateObjectiveFunctionFromTargets();

  // When
  LithoMapsInfoGenerator generator(scenario, *lithoMapsProjectReader);
  generator.setFileName("infoLithoMapsActual.txt");
  generator.generateInfoTextFile();

  // Then
  expectFileEq("infoLithoMapsExpected.txt", "infoLithoMapsActual.txt");
}
