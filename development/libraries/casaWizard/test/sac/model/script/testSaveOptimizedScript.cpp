#include "model/script/saveOptimizedScript.h"

#include "expectFileEq.h"
#include "model/sacScenario.h"
#include "stubProjectReader.h"

#include <QString>

#include <gtest/gtest.h>


TEST(SaveOptimzedScriptTest, testWriteScript)
{
  casaWizard::sac::SACScenario scenario{std::unique_ptr<casaWizard::ProjectReader>(new casaWizard::StubProjectReader())};
  scenario.setWorkingDirectory(".");

  const std::string expectedFile{"saveOptimizedScriptExpected.casa"};
  const std::string actualFile{"saveOptimizedScript.casa"};

  casaWizard::sac::SaveOptimizedScript script(scenario);
  script.writeScript();

  expectFileEq(expectedFile, actualFile);
}
