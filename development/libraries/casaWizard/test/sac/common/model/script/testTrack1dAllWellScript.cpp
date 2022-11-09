#include "model/script/track1dAllWellScript.h"

#include <QString>
#include <QStringList>
#include <gtest/gtest.h>

TEST(Track1dAllWellScriptTest, testGenerateCommands)
{
  // Given
  QVector<double> xCoordinates = {0.0, 1.1, 2.2};
  QVector<double> yCoordinates = {0.0, 1.1, 2.2};
  QStringList properties = {"Depth", "SonicSlowness", "Density"};

  casaWizard::sac::Track1DAllWellScript script("baseDirectory", xCoordinates, yCoordinates, properties, "Project.project3d");

  // When
  script.generateCommands();

  QString actualCommand = script.commands()[1].command;
  QStringList actualCommandArguments = script.commands()[1].commandArguments;
  QStringList expectedCommandArguments = {"-coordinates", "0.000000,0.000000,1.100000,1.100000,2.200000,2.200000", "-properties", "Depth,SonicSlowness,Density", "-age", "0", "-project", "Project.project3d", "-save", "welldata.csv"};

  // Then
  EXPECT_EQ(script.commands().size(), 2);
  EXPECT_EQ(script.commands()[0].command, "which");
  EXPECT_EQ(script.commands()[0].commandArguments[0], "track1d");
  EXPECT_EQ(actualCommand, "track1d");
  EXPECT_EQ(actualCommandArguments.size(), expectedCommandArguments.size());
  for (int i = 0; i < actualCommandArguments.size(); i++ )
  {
     EXPECT_EQ(actualCommandArguments[i], expectedCommandArguments[i]);
  }
}

TEST(Track1dAllWellScriptTest, testGenerateCommandsNoProperties)
{
  // Given
  QVector<double> xCoordinates = {0.0, 1.1, 2.2};
  QVector<double> yCoordinates = {0.0, 1.1, 2.2};
  QStringList properties;

  casaWizard::sac::Track1DAllWellScript script("baseDirectory", xCoordinates, yCoordinates, properties, "Project.project3d");

  // Then
  EXPECT_FALSE(script.generateCommands());
}

TEST(Track1dAllWellScriptTest, testGenerateCommandsNoCoordinates)
{
  // Given
  QVector<double> xCoordinates;
  QVector<double> yCoordinates;
  QStringList properties = {"Depth", "SonicSlowness", "Density"};

  casaWizard::sac::Track1DAllWellScript script("baseDirectory", xCoordinates, yCoordinates, properties, "Project.project3d");

  // Then
  EXPECT_FALSE(script.generateCommands());
}

TEST(Track1dAllWellScriptTest, testGenerateCommandsDifferentXYSizes)
{
  // Given
  QVector<double> xCoordinates = {0.0, 1.1, 2.2};
  QVector<double> yCoordinates = {1.1, 2.2};
  QStringList properties = {"Depth", "SonicSlowness", "Density"};

  casaWizard::sac::Track1DAllWellScript script("baseDirectory", xCoordinates, yCoordinates, properties, "Project.project3d");

  // Then
  EXPECT_FALSE(script.generateCommands());
}
