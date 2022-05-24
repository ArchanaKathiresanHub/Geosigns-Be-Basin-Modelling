#include "model/script/Generate3DScenarioScript.h"

#include "model/calibrationTargetManager.h"
#include "expectFileEq.h"
#include "model/sacScenario.h"
#include "stubProjectReader.h"

#include <QString>

#include <gtest/gtest.h>

using namespace casaWizard::sac;
using namespace casaWizard;

TEST(Generate3DScenarioScriptTest, testWriteScript)
{
  // Given
  casaWizard::sac::SACScenario scenario{new casaWizard::StubProjectReader()};
  scenario.setWorkingDirectory(".");
  scenario.setProject3dFileNameAndLoadFile("projStub.project3d");

  CalibrationTargetManager& ctManager = scenario.calibrationTargetManager();
  ctManager.addWell("Well 1", 1.0, 1.0); // sorted index = 0 and disabled
  ctManager.addWell("Well 2", 2.0, 16.2); // sorted index = 2 and disabled
  ctManager.addWell("Well 3", 1.0, 6.2); // sorted index = 1 and enabled
  ctManager.addWell("Well 4", 1.0, 6.2); // sorted index = NA, since this well is disabled.

  ctManager.setWellIsExcluded(true, 0);
  ctManager.setWellIsExcluded(true, 1);
  ctManager.setWellIsExcluded(true, 3);
  ctManager.setWellIsActive(false, 3);

  WellTrajectoryManager& manager = scenario.wellTrajectoryManager();
  manager.addWellTrajectory(0, "Density");
  manager.addWellTrajectory(1, "Density");
  manager.addWellTrajectory(2, "Density");
  manager.addWellTrajectory(3, "Density");

  casaWizard::sac::Generate3DScenarioScript script(scenario);

  // When
  script.writeScript();

  // Then
  const std::string expectedFile{"generate3DScenarioScriptExpected.casa"};
  const std::string actualFile{"generate3DScenarioScript.casa"};
  expectFileEq(expectedFile, actualFile);
}

TEST(Generate3DScenarioScriptTest, testWriteScriptIDWGaussian)
{
  casaWizard::sac::SACScenario scenario{new casaWizard::StubProjectReader()};
  scenario.setWorkingDirectory(".");
  scenario.setProject3dFileNameAndLoadFile("projStub.project3d");

  MapsManager& mapsManager = scenario.mapsManager();
  mapsManager.setSmoothingOption(1);
  mapsManager.setPIDW(3);
  mapsManager.setSmartGridding(false);

  const std::string expectedFile{"generate3DScenarioScript_IDW_GaussianExpected.casa"};
  const std::string actualFile{"generate3DScenarioScript.casa"};

  casaWizard::sac::Generate3DScenarioScript script(scenario);
  script.writeScript();

  expectFileEq(expectedFile, actualFile);
}

TEST(Generate3DScenarioScriptTest, testWriteScriptNNMovingAverage)
{
  casaWizard::sac::SACScenario scenario{new casaWizard::StubProjectReader()};
  scenario.setWorkingDirectory(".");
  scenario.setProject3dFileNameAndLoadFile("projStub.project3d");

  MapsManager& mapsManager = scenario.mapsManager();
  mapsManager.setSmoothingOption(2);
  mapsManager.setRadiusSmoothing(3500);
  mapsManager.setInterpolationMethod(1);

  const std::string expectedFile{"generate3DScenarioScript_NN_MovingAverageExpected.casa"};
  const std::string actualFile{"generate3DScenarioScript.casa"};

  casaWizard::sac::Generate3DScenarioScript script(scenario);
  script.writeScript();

  expectFileEq(expectedFile, actualFile);
}

