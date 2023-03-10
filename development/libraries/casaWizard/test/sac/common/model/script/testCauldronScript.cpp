//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/script/cauldronScript.h"

#include "model/LithologyScenario.h"
#include "stubProjectReader.h"
#include "expectFileEq.h"

#include <QString>
#include <QFile>

#include <gtest/gtest.h>

TEST(CauldronScriptTest, testInitialization)
{
  // Given
  casaWizard::sac::lithology::LithologyScenario scenario{new casaWizard::StubProjectReader()};
  scenario.setWorkingDirectory(".");
  scenario.setProject3dFileNameAndLoadFile("projStub.project3d");
  casaWizard::sac::CauldronScript script(scenario, "baseDirectory");

  // Then
  EXPECT_EQ(script.scenario().workingDirectory(), scenario.workingDirectory());
  EXPECT_EQ(script.scenario().workingDirectory(), ".");
  EXPECT_EQ(script.workingDirectory(), scenario.workingDirectory());
  EXPECT_EQ(script.scriptFilename(), "cauldronScript.casa");
}

TEST(CauldronScriptTest, testWriteScript)
{
  // Given
  casaWizard::sac::lithology::LithologyScenario scenario{new casaWizard::StubProjectReader()};
  scenario.setWorkingDirectory(".");
  scenario.setProject3dFileNameAndLoadFile("projStub.project3d");
  scenario.setNumberCPUs(10);
  casaWizard::sac::CauldronScript script(scenario, "");

  // When
  EXPECT_TRUE(script.writeScript());

  // Then
  expectFileEq("cauldronScript.casa", "cauldronScriptExpected.casa");
}
