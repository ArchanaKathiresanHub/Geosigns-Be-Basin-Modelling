#include "model/script/depthConversionScript.h"

#include "model/sacScenario.h"
#include "stubProjectReader.h"
#include "expectFileEq.h"

#include <QString>

#include <gtest/gtest.h>

TEST(DepthConversionScriptTest, testGenerateCommandsLocal)
{
  // Given
  casaWizard::sac::SACScenario scenario{new casaWizard::StubProjectReader()};
  scenario.setWorkingDirectory(".");
  scenario.setClusterName("LOCAL");
  scenario.setProject3dFilePath("./Project.project3d");
  scenario.setT2zNumberCPUs(10);
  scenario.setApplicationName("fastcauldron \"-temperature\"");
  std::unique_ptr<casaWizard::RunScript> script(new casaWizard::sac::DepthConversionScript(scenario, scenario.workingDirectory() + "/T2Z_step2", workloadmanagers::WorkLoadManagerType::AUTO));

  script->generateCommands();
  std::string expectedCommand = "mpirun_wrap.sh -n 10 fastdepthconversion -project Project.project3d -temperature -onlyat 0 -referenceSurface 0 -endSurface 10 -noofpp -preserveErosion -noCalculatedTWToutput";

  EXPECT_EQ(script->commands()[0].command.toStdString(), expectedCommand);

  scenario.setApplicationName("fastcauldron \"-itcoupled\"");
  script->generateCommands();

  expectedCommand = "mpirun_wrap.sh -n 10 fastdepthconversion -project Project.project3d -itcoupled -onlyat 0 -referenceSurface 0 -endSurface 10 -noofpp -preserveErosion -noCalculatedTWToutput";
  EXPECT_EQ(script->commands()[1].command.toStdString(), expectedCommand);

}

TEST(DepthConversionScriptTest, testGenerateCommandsClusterSLURM)
{
  // Given
  casaWizard::sac::SACScenario scenario{new casaWizard::StubProjectReader()};
  scenario.setWorkingDirectory(".");
  scenario.setClusterName("Cluster");
  scenario.setProject3dFilePath("./Project.project3d");
  scenario.setApplicationName("fastcauldron \"-temperature\"");
  std::unique_ptr<casaWizard::RunScript> script(new casaWizard::sac::DepthConversionScript(scenario, scenario.workingDirectory() + "/T2Z_step2", workloadmanagers::WorkLoadManagerType::SLURM));

  script->generateCommands();

  QString expectedCommand = scenario.workingDirectory() + "/T2Z_step2/runt2z.sh";
  EXPECT_EQ(script->commands()[0].command, expectedCommand);
#ifndef _WIN32
  system("sed -i '9d' ./T2Z_step2/runt2z.sh");
  expectFileEq("./T2Z_step2/runt2z.sh", "./T2Z_step2/runt2zExpectedSLURM.sh");
#endif
}

TEST(DepthConversionScriptTest, testGenerateCommandsClusterLSF)
{
  // Given
  casaWizard::sac::SACScenario scenario{new casaWizard::StubProjectReader()};
  scenario.setWorkingDirectory(".");
  scenario.setClusterName("Cluster");
  scenario.setProject3dFilePath("./Project.project3d");
  scenario.setApplicationName("fastcauldron \"-temperature\"");
  std::unique_ptr<casaWizard::RunScript> script(new casaWizard::sac::DepthConversionScript(scenario, scenario.workingDirectory() + "/T2Z_step2", workloadmanagers::WorkLoadManagerType::IBM_LSF));

  script->generateCommands();

  QString expectedCommand = scenario.workingDirectory() + "/T2Z_step2/runt2z.sh";
  EXPECT_EQ(script->commands()[0].command, expectedCommand);
#ifndef _WIN32
  system("sed -i '9d' ./T2Z_step2/runt2z.sh");
  expectFileEq("./T2Z_step2/runt2z.sh", "./T2Z_step2/runt2zExpectedLSF.sh");
#endif
}

TEST(DepthConversionScriptTest, testGenerateCommandsInvalidScriptLocation)
{
  // Given
  casaWizard::sac::SACScenario scenario{new casaWizard::StubProjectReader()};
  scenario.setWorkingDirectory("doesNotExist");
  scenario.setClusterName("Cluster");
  std::unique_ptr<casaWizard::RunScript> script(new casaWizard::sac::DepthConversionScript(scenario, scenario.workingDirectory() + "/T2Z_step2/", workloadmanagers::WorkLoadManagerType::SLURM));

  // Then
  EXPECT_FALSE(script->generateCommands());
}

