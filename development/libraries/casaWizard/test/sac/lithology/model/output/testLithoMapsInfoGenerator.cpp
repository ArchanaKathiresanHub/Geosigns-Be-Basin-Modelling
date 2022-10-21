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
#include "model/sacLithologyScenario.h"
#include "model/MapsManagerLithology.h"

#include <gtest/gtest.h>

using namespace casaWizard::sac;

TEST( LithoMapsInfoGenerator, testInfoGenerator )
{
   // Given
   std::remove("infoLithoMapsActual.txt");
   casaWizard::ProjectReader* lithoMapsProjectReader = new casaWizard::CMBProjectReader();
   lithoMapsProjectReader->load("LithoMapsProject.project3d");
   SacLithologyScenario scenario(lithoMapsProjectReader);
   scenario.calibrationTargetManager().addToMapping("TWTT", "TwoWayTime");
   scenario.calibrationTargetManager().addToMapping("DT", "SonicSlowness");
   scenario.calibrationTargetManager().addToMapping("GR", "GammaRay");
   scenario.calibrationTargetManager().addToMapping("Density", "BulkDensity");
   scenario.calibrationTargetManager().addToMapping("Velocity", "Velocity");

  casaWizard::sac::MapsManagerLithology& mapsManager = scenario.mapsManager();
  mapsManager.setPIDW(3);
  mapsManager.setSmoothingOption(1);
  mapsManager.setRadiusSmoothing(5000);
  mapsManager.setSmartGridding(false);

   // Set Wells
   scenario.calibrationTargetManager().addWell("10_AML2_AV", 184550.00, 608300.00);
   scenario.calibrationTargetManager().addWell("11_AMN1_AV", 192000.00, 615000.00);
   scenario.calibrationTargetManager().addWell("12_AMN2_AV", 191500.00, 615750.00);
   scenario.calibrationTargetManager().setWellIsActive(false, 0);
   scenario.calibrationTargetManager().setWellIsExcluded(true, 1);

   // Set Objective Function
   scenario.calibrationTargetManager().addCalibrationTarget("Name1", "TWTT", 0, 25.0, 20.0);
   scenario.calibrationTargetManager().addCalibrationTarget("Name1", "TWTT", 1, 30.0, 20.0);
   scenario.calibrationTargetManager().addCalibrationTarget("Name2", "DT", 0, 25.0, 20.0);
   scenario.calibrationTargetManager().addCalibrationTarget("Name2", "DT", 1, 30.0, 20.0);

   // These targets should not show up in the info.txt since they are disabled by default, or manually
   // disabled
   scenario.calibrationTargetManager().addCalibrationTarget("Name2", "GR", 0, 25.0, 20.0);
   scenario.calibrationTargetManager().addCalibrationTarget("Name2", "GR", 1, 30.0, 20.0);
   scenario.calibrationTargetManager().addCalibrationTarget("Name2", "Velocity", 0, 25.0, 20.0);
   scenario.calibrationTargetManager().addCalibrationTarget("Name2", "Velocity", 1, 30.0, 20.0);
   scenario.calibrationTargetManager().addCalibrationTarget("Name2", "Density", 0, 25.0, 20.0);
   scenario.calibrationTargetManager().addCalibrationTarget("Name2", "Density", 1, 30.0, 20.0);
   scenario.updateObjectiveFunctionFromTargets();
   scenario.objectiveFunctionManager().setEnabledState(false, scenario.objectiveFunctionManager().indexOfUserName("Density"));

   // When
   {
      std::unique_ptr<casaWizard::ProjectReader> lithoMapsProjectReader = std::unique_ptr<casaWizard::ProjectReader>(new casaWizard::CMBProjectReader());
      lithoMapsProjectReader->load("LithoMapsProject.project3d");
      LithoMapsInfoGenerator generator(scenario, std::move(lithoMapsProjectReader));
      generator.setFileName("infoLithoMapsActual.txt");
      generator.generateInfoTextFile();
   }

   // Then
   expectFileEq("infoLithoMapsExpected.txt", "infoLithoMapsActual.txt");
}
