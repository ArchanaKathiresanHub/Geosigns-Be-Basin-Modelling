#include "model/script/mcmcScript.h"

#include "expectFileEq.h"
#include "setupTestCasaScript.h"

#include <QString>

#include <gtest/gtest.h>

class McmcScriptTest : public SetupTestCasaScript
{
public:
  McmcScriptTest()
  {
    scenario.setStateFileNameQC("stateFileTestCasaScriptQC.txt");
    scenario.setWorkingDirectory(".");
  }
};

TEST_F(McmcScriptTest, testWriteScript)
{
  const std::string expectedFile{"McmcScriptExpected.casa"};
  const std::string actualFile{"uaScript.casa"};

  casaWizard::ua::McmcScript script(scenario);
  script.writeScript();

  expectFileEq(expectedFile, actualFile);
}
