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
  const std::string actualFile{"mcmcScript.casa"};

  scenario.updateIterationDir();

  //Disable BaseCase:
  scenario.setIsDoeOptionSelected(0,false);

  //Disable spaceFilling:
  scenario.setIsDoeOptionSelected(6, false);

  casaWizard::ua::McmcScript script(scenario);

  script.writeScript();

  expectFileEq(expectedFile, actualFile);
}
