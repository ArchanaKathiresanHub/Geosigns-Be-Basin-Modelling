#include "model/script/qcScript.h"

#include "expectFileEq.h"
#include "setupTestCasaScript.h"

#include <QString>

#include <gtest/gtest.h>

class QCScriptTest : public SetupTestCasaScript
{
public:
  QCScriptTest()
  {
    scenario.setStateFileNameQC("stateFileTestCasaScriptQC.txt");
  }
};

TEST_F(QCScriptTest, testWriteScript)
{
  const std::string expectedFile{"QCScriptExpected.casa"};
  const std::string actualFile{"uaScript.casa"};

  casaWizard::ua::QCScript script(scenario);
  script.writeScript();

  expectFileEq(expectedFile, actualFile);
}
