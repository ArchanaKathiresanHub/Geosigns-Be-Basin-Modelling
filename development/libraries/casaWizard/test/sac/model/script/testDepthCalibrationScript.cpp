#include "model/script/depthCalibrationScript.h"

#include "model/sacScenario.h"
#include "stubProjectReader.h"
#include "expectFileEq.h"

#include <QString>

#include <gtest/gtest.h>

TEST(DepthCalibrationScriptTest, testGenerateCommandsLocal)
{
  // Given
  casaWizard::sac::SACScenario scenario{new casaWizard::StubProjectReader()};
  scenario.setWorkingDirectory(".");
  scenario.setClusterName("LOCAL");
  scenario.setProject3dFilePath("./Project.project3d");
  std::unique_ptr<casaWizard::RunScript> script(new casaWizard::sac::DepthCalibrationScript(scenario, "baseDirectory"));

  script->generateCommands();

  QString expectedCommand = "fastdepthcalibration -project Project.project3d -temperature -onlyat 0 -referenceSurface 0 -endSurface 6";

  EXPECT_EQ(script->commands()[0].command, expectedCommand);
}

TEST(DepthCalibrationScriptTest, testGenerateCommandsCluster)
{
  // Given
  casaWizard::sac::SACScenario scenario{new casaWizard::StubProjectReader()};
  scenario.setWorkingDirectory(".");
  scenario.setClusterName("Cluster");
  scenario.setProject3dFilePath("./Project.project3d");
  std::unique_ptr<casaWizard::RunScript> script(new casaWizard::sac::DepthCalibrationScript(scenario, "baseDirectory"));

  script->generateCommands();

  QString expectedCommand = "bsub -K < " + scenario.workingDirectory() + "/T2Z_step2/runt2z.sh";
  EXPECT_EQ(script->commands()[0].command, expectedCommand);
  expectFileEq("./T2Z_step2/runt2z.sh", "./T2Z_step2/runt2zExpected.sh");
}

TEST(DepthCalibrationScriptTest, testGenerateCommandsInvalidScriptLocation)
{
  // Given
  casaWizard::sac::SACScenario scenario{new casaWizard::StubProjectReader()};
  scenario.setWorkingDirectory("doesNotExist");
  scenario.setClusterName("Cluster");
  std::unique_ptr<casaWizard::RunScript> script(new casaWizard::sac::DepthCalibrationScript(scenario, "baseDirectory"));

  // Then
  EXPECT_FALSE(script->generateCommands());
}

