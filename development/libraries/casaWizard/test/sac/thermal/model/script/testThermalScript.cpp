#include "model/script/ThermalScript.h"

#include "expectFileEq.h"
#include "stubProjectReader.h"

#include <QString>

#include <gtest/gtest.h>

TEST(SACScriptTest, testWriteThermalScript)
{
  casaWizard::sac::thermal::ThermalScenario scenario{new casaWizard::StubProjectReader()};
  casaWizard::sac::thermal::TCHPManager& tchpManager{scenario.TCHPmanager()};
  tchpManager.setMinTCHP(1);
  tchpManager.setMaxTCHP(4.8);

  casaWizard::sac::WellTrajectoryManager& wellTrajectoryManager{scenario.wellTrajectoryManager()};
  scenario.setWorkingDirectory(".");
  scenario.setProject3dFileNameAndLoadFile("projStub.project3d");

  casaWizard::CalibrationTargetManager& ctManager = scenario.calibrationTargetManager();

  const int wellIndex1 = ctManager.addWell("Well1", 0, 0);
  wellTrajectoryManager.addWellTrajectory(wellIndex1, "VRe");
  ctManager.addCalibrationTarget("TargetName", "VRe", wellIndex1, 200.0, 0.7);
  wellTrajectoryManager.addWellTrajectory(wellIndex1, "Temp");
  ctManager.addCalibrationTarget("TargetName", "Temp", wellIndex1, 300.0, 17.4);

  const int wellIndex2 = ctManager.addWell("Well2", 1, 2);
  wellTrajectoryManager.addWellTrajectory(wellIndex2, "VRe");
  ctManager.addCalibrationTarget("TargetName", "VRe", wellIndex2, 200.0, 0.5);

  // This well should not be written to the script, since the enabled state is set to false
  const int wellIndex3 = ctManager.addWell("Well3", 1, 2);
  wellTrajectoryManager.addWellTrajectory(wellIndex3, "DT");
  ctManager.addCalibrationTarget("TargetName", "DT", wellIndex3, 200.0, 0.5);
  scenario.updateObjectiveFunctionFromTargets();
  scenario.objectiveFunctionManager().setEnabledState(false, scenario.objectiveFunctionManager().indexOfUserName("DT"));
  wellTrajectoryManager.updateWellTrajectories(ctManager);

  scenario.calibrationTargetManager().addToMapping("VRe", "Vr");
  scenario.calibrationTargetManager().addToMapping("Temp", "Temperature");
  scenario.calibrationTargetManager().addToMapping("DT", "SonicSlowness");

  const std::string expectedFile{"ThermalScriptExpected.casa"};
  const std::string actualFile{"sacScript.casa"};

  casaWizard::sac::thermal::ThermalScript script(scenario, scenario.workingDirectory());
  script.writeScript();

  expectFileEq(expectedFile, actualFile);
}
