#include "model/script/OptimizedTCHPScript.h"

#include "model/ThermalScenario.h"
#include "stubProjectReader.h"

#include <QString>

#include <gtest/gtest.h>

TEST(OptimizedTCHPScriptTest, testGenerateCommands)
{
  // Given
  casaWizard::sac::thermal::ThermalScenario scenario{new casaWizard::StubProjectReader()};
  scenario.setWorkingDirectory(".");
  casaWizard::sac::thermal::OptimizedTCHPScript script("baseDirectory");

  // When
  script.addCase("Folder1");
  script.addCase("Folder2");
  script.generateCommands();

  // Then
  EXPECT_EQ(script.commands()[1].relativeDirectory, "Folder1"); // starts at 1, since the first command is "which casa"
  EXPECT_EQ(script.commands()[2].relativeDirectory, "Folder2");
  EXPECT_EQ(script.commands()[1].command, "casa.exe");
  EXPECT_EQ(script.commands()[2].command, "casa.exe");


  EXPECT_EQ(script.commands()[1].commandArguments.size(), 1);
  EXPECT_EQ(script.commands()[1].commandArguments[0], "obtainTCHP.casa");
  EXPECT_EQ(script.commands()[2].commandArguments.size(), 1);
  EXPECT_EQ(script.commands()[2].commandArguments[0], "obtainTCHP.casa");
}
