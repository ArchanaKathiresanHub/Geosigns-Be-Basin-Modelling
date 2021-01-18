#include "model/script/Generate3DScenarioScript.h"

#include "expectFileEq.h"
#include "model/sacScenario.h"
#include "stubProjectReader.h"

#include <QString>

#include <gtest/gtest.h>


TEST(SaveOptimzedScriptTest, testWriteScript)
{
  casaWizard::sac::SACScenario scenario{new casaWizard::StubProjectReader()};
  scenario.setWorkingDirectory(".");

  const std::string expectedFile{"generate3DScenarioScriptExpected.casa"};
  const std::string actualFile{"generate3DScenarioScript.casa"};

  casaWizard::sac::Generate3DScenarioScript script(scenario);
  script.writeScript();

  expectFileEq(expectedFile, actualFile);
}
