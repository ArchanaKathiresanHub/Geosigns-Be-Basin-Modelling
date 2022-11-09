#include "model/script/depthConversionScript.h"

#include "model/LithologyScenario.h"
#include "stubProjectReader.h"
#include "expectFileEq.h"

#include <QString>

#include <gtest/gtest.h>

TEST(DepthConversionScriptTest, testGenerateCommandsLocal)
{
  // Given
  casaWizard::sac::LithologyScenario scenario{new casaWizard::StubProjectReader()};
  scenario.setWorkingDirectory(".");
  scenario.setClusterName("LOCAL");
  scenario.setProject3dFileNameAndLoadFile("./Project.project3d");
  scenario.setT2zNumberCPUs(10);
  scenario.setApplicationName("fastcauldron \"-temperature\"");
  std::unique_ptr<casaWizard::RunScript> script(new casaWizard::sac::DepthConversionScript(scenario, scenario.workingDirectory() + "/T2Z_step2", workloadmanagers::WorkLoadManagerType::AUTO));

  script->generateCommands();
  EXPECT_EQ(script->commands().size(), 1); // When running locally now, still a script is created
  EXPECT_EQ(script->commands()[0].command, "sh"); // It's running sh
  EXPECT_EQ(script->commands()[0].commandArguments.size(), 2);
  EXPECT_EQ(script->commands()[0].commandArguments[0], "-c");
  EXPECT_EQ(script->commands()[0].commandArguments[1], "./T2Z_step2/runt2z.sh"); // The generated script
}

TEST(DepthConversionScriptTest, testGenerateCommandsClusterSLURM)
{
  // Given
  casaWizard::sac::LithologyScenario scenario{new casaWizard::StubProjectReader()};
  scenario.setWorkingDirectory(".");
  scenario.setClusterName("Cluster");
  scenario.setProject3dFileNameAndLoadFile("./Project.project3d");
  scenario.setApplicationName("fastcauldron \"-temperature\"");
  std::unique_ptr<casaWizard::RunScript> script(new casaWizard::sac::DepthConversionScript(scenario, scenario.workingDirectory() + "/T2Z_step2", workloadmanagers::WorkLoadManagerType::SLURM));

  script->generateCommands();

  EXPECT_EQ(script->commands().size(), 1);
  EXPECT_EQ(script->commands()[0].command, "sh"); // It's running sh
  EXPECT_EQ(script->commands()[0].commandArguments.size(), 2);
  EXPECT_EQ(script->commands()[0].commandArguments[0], "-c");
  EXPECT_EQ(script->commands()[0].commandArguments[1], "./T2Z_step2/runt2z.sh"); // The generated script
#ifndef _WIN32
  system("sed -i '9d' ./T2Z_step2/runt2z.sh");
  expectFileEq("./T2Z_step2/runt2z.sh", "./T2Z_step2/runt2zExpectedSLURM.sh");
#endif
}

TEST(DepthConversionScriptTest, testGenerateCommandsClusterLSF)
{
  // Given
  casaWizard::sac::LithologyScenario scenario{new casaWizard::StubProjectReader()};
  scenario.setWorkingDirectory(".");
  scenario.setClusterName("Cluster");
  scenario.setProject3dFileNameAndLoadFile("./Project.project3d");
  scenario.setApplicationName("fastcauldron \"-temperature\"");
  std::unique_ptr<casaWizard::RunScript> script(new casaWizard::sac::DepthConversionScript(scenario, scenario.workingDirectory() + "/T2Z_step2", workloadmanagers::WorkLoadManagerType::IBM_LSF));

  script->generateCommands();

  EXPECT_EQ(script->commands().size(), 1);
  EXPECT_EQ(script->commands()[0].command, "sh"); // It's running sh
  EXPECT_EQ(script->commands()[0].commandArguments.size(), 2);
  EXPECT_EQ(script->commands()[0].commandArguments[0], "-c");
  EXPECT_EQ(script->commands()[0].commandArguments[1], "./T2Z_step2/runt2z.sh"); // The generated script
#ifndef _WIN32
  system("sed -i '9d' ./T2Z_step2/runt2z.sh");
  expectFileEq("./T2Z_step2/runt2z.sh", "./T2Z_step2/runt2zExpectedLSF.sh");
#endif
}

TEST(DepthConversionScriptTest, testGenerateCommandsInvalidScriptLocation)
{
  // Given
  casaWizard::sac::LithologyScenario scenario{new casaWizard::StubProjectReader()};
  scenario.setWorkingDirectory("doesNotExist");
  scenario.setClusterName("Cluster");
  std::unique_ptr<casaWizard::RunScript> script(new casaWizard::sac::DepthConversionScript(scenario, scenario.workingDirectory() + "/T2Z_step2/", workloadmanagers::WorkLoadManagerType::SLURM));

  // Then
  EXPECT_FALSE(script->generateCommands());
}

