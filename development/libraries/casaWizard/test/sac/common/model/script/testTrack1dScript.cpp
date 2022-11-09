#include "model/script/track1dScript.h"
#include <QString>

#include <gtest/gtest.h>

TEST(Track1dScriptTest, testGenerateCommands)
{
   // Given
   casaWizard::sac::Track1DScript script("baseDirectory", "Project");

   // When
   script.addWell(0.0, 1.1, "Density", "Density", "CaseSet");
   script.addWell(2.2, 3.3, "SonicSlowness", "SonicSlowness", "RunLocation");

   script.generateCommands();
   QStringList expectedCommands = {"track1d", "track1d"};
   QVector<QStringList> expectedCommandArguments;
   expectedCommandArguments.push_back({"-coordinates", "0.000000,1.100000", "-properties", "Density", "-age", "0", "-project", "Project.project3d", "-save", "wellTrajectory-Project-Density.csv", "-lean"});
   expectedCommandArguments.push_back({"-coordinates", "2.200000,3.300000", "-properties", "SonicSlowness", "-age", "0", "-project", "Project.project3d", "-save", "wellTrajectory-Project-SonicSlowness.csv", "-lean"});

   // Then
   EXPECT_EQ(script.commands()[0].relativeDirectory, "CaseSet");
   EXPECT_EQ(script.commands()[1].relativeDirectory, "RunLocation");


   for (int iCommand = 0; iCommand < 2; iCommand++)
   {
      EXPECT_EQ(script.commands()[iCommand].command, expectedCommands[iCommand]) << "Wrong at command 0";
      EXPECT_EQ(script.commands()[iCommand].commandArguments.size(), expectedCommandArguments[iCommand].size()) << "Wrong at command 0";
      for (int i = 0; i < expectedCommandArguments[iCommand].size(); i++)
      {
         EXPECT_EQ(expectedCommandArguments[iCommand][i], script.commands()[iCommand].commandArguments[i]);
      }
   }
}


