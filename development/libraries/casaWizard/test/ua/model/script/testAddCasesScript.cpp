#include "model/script/addCasesScript.h"

#include "expectFileEq.h"
#include "setupTestCasaScript.h"

#include <QString>

#include <gtest/gtest.h>

class AddCasesScriptTest : public SetupTestCasaScript
{
};

TEST_F(AddCasesScriptTest, testWriteScript)
{
  casaWizard::ua::ManualDesignPointManager& manager = scenario.manualDesignPointManager();
  manager.completeAll();
  manager.addDesignPoint({1.4});


  const std::string expectedFile{"AddCasesScriptExpected.casa"};
  const std::string actualFile{"addCasesScript.casa"};

  casaWizard::ua::AddCasesScript script(scenario);

  EXPECT_TRUE(script.writeScript());

  expectFileEq(expectedFile, actualFile);
}
