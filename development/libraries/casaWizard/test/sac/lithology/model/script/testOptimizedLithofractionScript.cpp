#include "model/script/optimizedLithofractionScript.h"

#include "model/LithologyScenario.h"
#include "stubProjectReader.h"

#include <QString>

#include <gtest/gtest.h>

TEST(OptimizedLithofractionScriptTest, testGenerateCommands)
{
  // Given
  casaWizard::sac::lithology::LithologyScenario scenario{new casaWizard::StubProjectReader()};
  scenario.setWorkingDirectory(".");
  casaWizard::sac::lithology::OptimizedLithofractionScript script("baseDirectory");

  // When
  script.addCase("Folder1");
  script.addCase("Folder2");
  script.generateCommands();

  // Then
  EXPECT_EQ(script.commands()[1].relativeDirectory, "Folder1"); // starts at 1, since the first command is "which casa"
  EXPECT_EQ(script.commands()[2].relativeDirectory, "Folder2");
  EXPECT_EQ(script.commands()[1].command, "casa.exe obtainLithofractions.casa");
  EXPECT_EQ(script.commands()[2].command, "casa.exe obtainLithofractions.casa");
}
