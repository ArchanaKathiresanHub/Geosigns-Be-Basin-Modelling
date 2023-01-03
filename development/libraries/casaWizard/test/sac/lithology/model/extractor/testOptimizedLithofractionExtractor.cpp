#include "model/extractor/optimizedLithofractionExtractor.h"

#include "model/calibrationTargetManager.h"

#include "stubSACScenario.h"
#include "stubCaseExtractor.h"

#include <gtest/gtest.h>
#include <memory>

using namespace casaWizard::sac::lithology;
using namespace casaWizard::sac;
using namespace casaWizard;

TEST(OptimizedLithofractionExtractorTest, testInitialization)
{
  // Given
  std::unique_ptr<StubSacLithologyScenario> scenario(new StubSacLithologyScenario);
  scenario->setWorkingDirectory("workingDirectory");
  scenario->setRunLocation("CaseSet");

  // When
  OptimizedLithofractionExtractor extractor(*scenario);

  // Then
  EXPECT_EQ(extractor.iterationPath(), QString("workingDirectory/calibration_step1/CaseSet/Iteration_1"));
}

TEST(OptimizedLithofractionExtractorTest, testExtract)
{
  // Given
  std::unique_ptr<StubSacLithologyScenario> scenario(new StubSacLithologyScenario);
  scenario->setWorkingDirectory("workingDirectory");
  scenario->setRunLocation("CaseSet");
  CalibrationTargetManager& ctManager = scenario->calibrationTargetManager();
  ctManager.addWell("Well 1", 1.0, 1.0);
  ctManager.addWell("Well 2", 2.0, 16.2);
  OptimizedLithofractionExtractor extractor(*scenario);

  // When
  extractor.extract();
  extractor.script().generateCommands();

  // Then
  EXPECT_EQ(extractor.script().commands().size(), 3); // size is number of wells + 1 due to the "which casa" command
}

TEST(OptimizedLithofractionExtractorTest, testGetDataCreator)
{
  // Given
  std::unique_ptr<StubSacLithologyScenario> scenario(new StubSacLithologyScenario);
  scenario->setWorkingDirectory("workingDirectory");
  scenario->setRunLocation("CaseSet");
  OptimizedLithofractionExtractor extractor(*scenario);

  // Then
  EXPECT_NO_THROW(extractor.dataCreator());
}

