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

  CalibrationTargetManager& ctManager = scenario.calibrationTargetManager();
  ctManager.addWell("Well 1", 1.0, 1.0);
  ctManager.addWell("Well 2", 2.0, 16.2);
  ctManager.addWell("Well 3", 1.0, 6.2);
  ctManager.addWell("Well 4", 1.0, 6.2);

  ctManager.setWellIsExcluded(true, 0);
  ctManager.setWellIsExcluded(true, 2);
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
  scenario.setSmoothingOption(1);
  scenario.setPIDW(3);
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
  scenario.setSmoothingOption(2);
  scenario.setRadiusSmoothing(3500);
  scenario.setInterpolationMethod(1);

  const std::string expectedFile{"generate3DScenarioScript_NN_MovingAverageExpected.casa"};
  const std::string actualFile{"generate3DScenarioScript.casa"};

  casaWizard::sac::Generate3DScenarioScript script(scenario);
  script.writeScript();

  expectFileEq(expectedFile, actualFile);
}

