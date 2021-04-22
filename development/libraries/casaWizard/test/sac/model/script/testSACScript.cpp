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
  wellTrajectoryManager.addWellTrajectory(wellIndex1, "TwoWayTime");
  wellTrajectoryManager.addWellTrajectory(wellIndex1, "Temperature");

  const int wellIndex2 = ctManager.addWell("Well2", 1, 2);
  wellTrajectoryManager.addWellTrajectory(wellIndex2, "TwoWayTime");

  const std::string expectedFile{"SACScriptExpected.casa"};
  const std::string actualFile{"sacScript.casa"};

  casaWizard::sac::SACScript script(scenario, scenario.workingDirectory());
  script.writeScript();

  expectFileEq(expectedFile, actualFile);
}
