//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/script/collectObservablesScript.h"

#include "expectFileEq.h"
#include "setupTestCasaScript.h"

#include <QString>

#include <gtest/gtest.h>

namespace
{
class CollectObservablesScriptTest : public SetupTestCasaScript
{};
}

TEST_F(CollectObservablesScriptTest, testWriteScript)
{
   scenario.updateIterationDir();

   const std::string expectedFile{"collectObservablesScriptExpected.casa"};
   const std::string actualFile{"collectObservablesScript.casa"};

   casaWizard::ua::CollectObservablesScript script(scenario);
   EXPECT_TRUE(script.writeScript());
   expectFileEq(expectedFile, actualFile);
}
