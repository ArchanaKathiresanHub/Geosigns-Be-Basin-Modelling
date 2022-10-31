//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

//note: Very simmilar to testInfoLithoMapGenerator

#include "expectFileEq.h"
#include "stubProjectReader.h"
#include "model/output/ThermalInfoGenerator.h"
#include "model/ThermalScenario.h"
#include "model/ThermalMapManager.h"

#include <gtest/gtest.h>

using namespace casaWizard::sac::thermal;

TEST( TCHPMapsInfoGenerator, testInfoGenerator )
{
   // Given
   std::remove("infoTCHPMapActual.txt");
   casaWizard::ProjectReader* TCHPMapsProjectReader = new casaWizard::StubProjectReader();
   ThermalScenario scenario(TCHPMapsProjectReader);
   scenario.TCHPmanager().setMinTCHP(0.2);
   scenario.TCHPmanager().setMaxTCHP(4.8);

   scenario.calibrationTargetManager().addToMapping("TWTT", "TwoWayTime");
   scenario.calibrationTargetManager().addToMapping("DT", "SonicSlowness");
   scenario.calibrationTargetManager().addToMapping("GR", "GammaRay");
   scenario.calibrationTargetManager().addToMapping("Density", "BulkDensity");
   scenario.calibrationTargetManager().addToMapping("Velocity", "Velocity");

   casaWizard::sac::thermal::ThermalMapManager& mapsManager = scenario.mapsManager();
   mapsManager.setPIDW(3);
   mapsManager.setSmoothingOption(1);
   mapsManager.setRadiusSmoothing(5000);

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
      std::unique_ptr<casaWizard::ProjectReader> TCHPMapsProjectReader = std::unique_ptr<casaWizard::ProjectReader>(new casaWizard::StubProjectReader());
      ThermalInfoGenerator generator(scenario, std::move(TCHPMapsProjectReader));
      generator.setFileName("infoTCHPMapActual.txt");
      generator.generateInfoTextFile();
   }

   // Then
   expectFileEq("infoTCHPMapExpected.txt", "infoTCHPMapActual.txt");
}
