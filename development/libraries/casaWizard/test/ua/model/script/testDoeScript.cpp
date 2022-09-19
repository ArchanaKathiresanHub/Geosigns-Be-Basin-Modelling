#include "model/script/doeScript.h"

#include "expectFileEq.h"
#include "setupTestCasaScript.h"

#include <QString>

#include <gtest/gtest.h>

class DoeScriptTest : public SetupTestCasaScript
{
};

TEST_F(DoeScriptTest, testWriteScript)
{
   const std::string expectedFile{"DoEScriptExpected.casa"};
   const std::string actualFile{"doeScript.casa"};

   scenario.updateIterationDir();

   // user defined doe
   scenario.setIsDoeOptionSelected(6, true);

   scenario.manualDesignPointManager().addInfluentialParameter(1);
   scenario.manualDesignPointManager().addDesignPoint({1.2});

   casaWizard::ua::DoEScript script(scenario);

   EXPECT_TRUE(script.writeScript());

   expectFileEq(expectedFile, actualFile,{11}); //Exclude line 11 from comparison, which contains a time stamp.
}
