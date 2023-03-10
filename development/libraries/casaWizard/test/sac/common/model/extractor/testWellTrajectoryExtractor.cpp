#include "model/extractor/wellTrajectoryExtractor.h"

#include "model/calibrationTargetManager.h"

#include "stubSACScenario.h"
#include "stubCaseExtractor.h"

#include <gtest/gtest.h>
#include <memory>

using namespace casaWizard::sac::lithology;
using namespace casaWizard::sac;
using namespace casaWizard;

TEST(WellTrajectoryExtractorTest, testInitialization)
{
  // Given
  std::unique_ptr<StubSacLithologyScenario> scenario(new StubSacLithologyScenario);
  scenario->setWorkingDirectory("workingDirectory");
  scenario->setRunLocation("CaseSet");

  // When
  WellTrajectoryExtractor extractor(*scenario, "bestMatchedCase");

  // Then
  EXPECT_EQ(extractor.iterationPath(), QString("workingDirectory/calibration_step1/CaseSet/Iteration_1"));
}

TEST(WellTrajectoryExtractorTest, testExtract)
{
  // Given
  std::unique_ptr<StubSacLithologyScenario> scenario(new StubSacLithologyScenario);
  scenario->setWorkingDirectory("workingDirectory");
  scenario->setRunLocation("CaseSet");
  CalibrationTargetManager& ctManager = scenario->calibrationTargetManager();
  ctManager.addWell("Well 1", 1.0, 1.0);
  ctManager.addWell("Well 2", 2.0, 16.2);
  casaWizard::sac::WellTrajectoryManager& manager = scenario->wellTrajectoryManager();
  manager.addWellTrajectory(0, "Density");
  manager.addWellTrajectory(1, "SonicSlowness");

  WellTrajectoryExtractor extractor(*scenario, "bestMatchedCase");

  // When
  extractor.extract();
  extractor.script().generateCommands();

  // Then
  EXPECT_EQ(extractor.script().commands().size(), 2); // size is number of trajectories
}

TEST(OptimizedLithofractionExtractorTest, testGetDataCreator)
{
  // Given
  std::unique_ptr<StubSacLithologyScenario> scenario(new StubSacLithologyScenario);
  scenario->setWorkingDirectory("workingDirectory");
  scenario->setRunLocation("CaseSet");
  WellTrajectoryExtractor extractor(*scenario, "bestMatchedCase");

  // Then
  EXPECT_NO_THROW(extractor.dataCreator());
}

