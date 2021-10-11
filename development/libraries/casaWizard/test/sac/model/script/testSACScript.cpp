#include "model/script/sacScript.h"

#include "expectFileEq.h"
#include "model/sacScenario.h"
#include "stubProjectReader.h"

#include <QString>

#include <gtest/gtest.h>


TEST(SACScriptTest, testWriteScript)
{
  casaWizard::sac::SACScenario scenario{new casaWizard::StubProjectReader()};
  casaWizard::sac::LithofractionManager& lithofractionManager{scenario.lithofractionManager()};
  casaWizard::sac::WellTrajectoryManager& wellTrajectoryManager{scenario.wellTrajectoryManager()};
  scenario.setWorkingDirectory(".");

  lithofractionManager.addLithofraction("Layer 1", true, false);
  lithofractionManager.addLithofraction("Layer 2");
  lithofractionManager.addLithofraction("Layer 3");
  lithofractionManager.setLithofractionSecondComponent(0, 3);
  lithofractionManager.setLithofractionSecondComponent(1, 2);
  lithofractionManager.setLithoFractionDoFirstOptimization(2, false);

  casaWizard::CalibrationTargetManager& ctManager = scenario.calibrationTargetManager();

  const int wellIndex1 = ctManager.addWell("Well1", 0, 0);
  wellTrajectoryManager.addWellTrajectory(wellIndex1, "TWTT");
  ctManager.addCalibrationTarget("TargetName", "TWTT", wellIndex1, 200.0, 0.7);
  wellTrajectoryManager.addWellTrajectory(wellIndex1, "Temp");
  ctManager.addCalibrationTarget("TargetName", "Temp", wellIndex1, 300.0, 17.4);

  const int wellIndex2 = ctManager.addWell("Well2", 1, 2);
  wellTrajectoryManager.addWellTrajectory(wellIndex2, "TWTT");
  ctManager.addCalibrationTarget("TargetName", "TWTT", wellIndex2, 200.0, 0.5);

  // This well should not be written to the script, since the enabled state is set to false
  const int wellIndex3 = ctManager.addWell("Well2", 1, 2);
  wellTrajectoryManager.addWellTrajectory(wellIndex3, "DT");
  ctManager.addCalibrationTarget("TargetName", "DT", wellIndex3, 200.0, 0.5);
  scenario.updateObjectiveFunctionFromTargets();
  scenario.objectiveFunctionManager().setEnabledState(false, scenario.objectiveFunctionManager().indexOfUserName("DT"));
  wellTrajectoryManager.updateWellTrajectories(ctManager);

  scenario.calibrationTargetManager().addToMapping("TWTT", "TwoWayTime");
  scenario.calibrationTargetManager().addToMapping("Temp", "Temperature");
  scenario.calibrationTargetManager().addToMapping("DT", "SonicSlowness");

  const std::string expectedFile{"SACScriptExpected.casa"};
  const std::string actualFile{"sacScript.casa"};

  casaWizard::sac::SACScript script(scenario, scenario.workingDirectory());
  script.writeScript();

  expectFileEq(expectedFile, actualFile);
}
