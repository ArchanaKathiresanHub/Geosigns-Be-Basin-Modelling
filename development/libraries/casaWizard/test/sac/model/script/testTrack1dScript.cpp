#include "model/script/track1dScript.h"
#include <QString>

#include <gtest/gtest.h>

TEST(Track1dScriptTest, testGenerateCommands)
{
  // Given
  casaWizard::sac::Track1DScript script("baseDirectory", "Project");

  // When
  script.addWell(0.0, 1.1, "Density", "CaseSet");
  script.addWell(2.2, 3.3, "SonicSlowness", "RunLocation");

  script.generateCommands();
  QStringList expectedCommands;
  expectedCommands.push_back("track1d -coordinates 0.000000,1.100000 -properties Density -age 0 -project Project.project3d -save wellTrajectory-Project-Density.csv -lean");
  expectedCommands.push_back("track1d -coordinates 0.000000,1.100000 -properties Density -age 0 -project bestMatchedCase.project3d -save wellTrajectory-bestMatchedCase-Density.csv -lean");
  expectedCommands.push_back("track1d -coordinates 2.200000,3.300000 -properties SonicSlowness -age 0 -project Project.project3d -save wellTrajectory-Project-SonicSlowness.csv -lean");
  expectedCommands.push_back("track1d -coordinates 2.200000,3.300000 -properties SonicSlowness -age 0 -project bestMatchedCase.project3d -save wellTrajectory-bestMatchedCase-SonicSlowness.csv -lean");

  // Then
  int counter = 0;
  for (const casaWizard::RunCommand& command : script.commands())
  {
    EXPECT_EQ(command.relativeDirectory, counter < 2 ? "CaseSet" : "RunLocation");
    EXPECT_EQ(command.command, expectedCommands[counter]) << "Wrong at command" << counter;
    counter++;
  }
}

