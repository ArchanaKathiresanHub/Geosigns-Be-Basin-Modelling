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
  expectedCommands.push_back("track1d -coordinates 2.200000,3.300000 -properties SonicSlowness -age 0 -project Project.project3d -save wellTrajectory-Project-SonicSlowness.csv -lean");

  // Then
  EXPECT_EQ(script.commands()[0].relativeDirectory, "CaseSet");
  EXPECT_EQ(script.commands()[0].command, expectedCommands[0]) << "Wrong at command 0";
  EXPECT_EQ(script.commands()[1].relativeDirectory, "RunLocation");
  EXPECT_EQ(script.commands()[1].command, expectedCommands[1]) << "Wrong at command 1";
}


