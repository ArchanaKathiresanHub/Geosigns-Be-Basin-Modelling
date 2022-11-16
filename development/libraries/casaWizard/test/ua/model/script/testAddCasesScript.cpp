#include "model/script/addCasesScript.h"

#include "expectFileEq.h"
#include "setupTestCasaScript.h"

#include <QString>

#include <gtest/gtest.h>

namespace
{
class AddCasesScriptTest : public SetupTestCasaScript
{};
}

TEST_F(AddCasesScriptTest, testWriteScript)
{
   // user defined doe
   scenario.setIsDoeOptionSelected(6, true);

   casaWizard::ua::ManualDesignPointManager& manager = scenario.manualDesignPointManager();
   manager.addInfluentialParameter(1);
   manager.addDesignPoint({1.4});

   scenario.updateIterationDir();

   const std::string expectedFile{"AddCasesScriptExpected.casa"};
   const std::string actualFile{"addCasesScript.casa"};

   casaWizard::ua::AddCasesScript script(scenario);
   EXPECT_TRUE(script.writeScript());

   const int lineWithTimeStamp = 8; //skip line with time stamp in equality check
   expectFileEq(expectedFile, actualFile,{lineWithTimeStamp});
}
