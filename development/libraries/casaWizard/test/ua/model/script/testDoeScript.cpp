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
  const std::string actualFile{"uaScript.casa"};

  casaWizard::ua::DoEScript script(scenario);
  script.writeScript();

  expectFileEq(expectedFile, actualFile);
}
