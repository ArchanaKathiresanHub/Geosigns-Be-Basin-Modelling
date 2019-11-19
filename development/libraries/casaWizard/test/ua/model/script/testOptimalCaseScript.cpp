#include "model/script/optimalCaseScript.h"

#include "expectFileEq.h"
#include "setupTestCasaScript.h"

#include <QString>

#include <gtest/gtest.h>

class OptimalCaseScriptTest : public SetupTestCasaScript
{
public:
  OptimalCaseScriptTest()
  {
    scenario.setStateFileNameMCMC("stateFileTestCasaScriptMCMC.txt");
    stateFile_ = scenario.stateFileNameMCMC().toStdString();
  }
};

TEST_F(OptimalCaseScriptTest, testWriteScript)
{
  const std::string expectedFile{"OptimalCaseScriptExpected.casa"};
  const std::string actualFile{"optimalCaseScript.casa"};

  casaWizard::ua::OptimalCaseScript script(scenario);

  EXPECT_TRUE(script.writeScript());

  expectFileEq(expectedFile, actualFile);
}
