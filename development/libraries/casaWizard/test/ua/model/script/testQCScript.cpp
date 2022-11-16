#include "model/script/qcScript.h"

#include "expectFileEq.h"
#include "setupTestCasaScript.h"

#include <QString>

#include <gtest/gtest.h>

namespace
{
class QCScriptTest : public SetupTestCasaScript
{
public:
   QCScriptTest()
   {
      scenario.setStateFileNameQC("stateFileTestCasaScriptQC.txt");
   }
};
}

TEST_F(QCScriptTest, testWriteScript)
{
   const std::string expectedFile{"QCScriptExpected.casa"};
   const std::string actualFile{"qcScript.casa"};

   //Disable BaseCase:
   scenario.setIsDoeOptionSelected(0,false);

   //Disable spaceFilling:
   scenario.setIsDoeOptionSelected(6, false);

   scenario.updateIterationDir();

   casaWizard::ua::QCScript script(scenario);
   EXPECT_TRUE(script.writeScript());

   expectFileEq(expectedFile, actualFile);
}
