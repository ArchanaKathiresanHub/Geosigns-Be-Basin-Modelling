#include "model/script/runOptimalCaseScript.h"

#include "expectFileEq.h"
#include "setupTestCasaScript.h"

#include <QString>

#include <gtest/gtest.h>

class RunOptimalCaseScriptTest : public SetupTestCasaScript
{
public:
  RunOptimalCaseScriptTest()
  {
    scenario.setStateFileNameMCMC("stateFileTestCasaScriptMCMC.txt");
    stateFile_ = scenario.stateFileNameMCMC().toStdString();
  }
};

TEST_F(RunOptimalCaseScriptTest, testWriteScript)
{
  const std::string expectedFile{"RunOptimalCaseScriptExpected.casa"};
  const std::string actualFile{"runOptimalCaseScript.casa"};

  casaWizard::ua::RunOptimalCaseScript script(scenario);
  script.writeScript();

  expectFileEq(expectedFile, actualFile);
}
