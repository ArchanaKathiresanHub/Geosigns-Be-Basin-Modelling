#include "model/extractor/OptimizedTCHPExtractor.h"

#include "model/calibrationTargetManager.h"

#include "stubSACScenario.h"
#include "stubCaseExtractor.h"

#include <gtest/gtest.h>
#include <memory>

using namespace casaWizard::sac::thermal;
using namespace casaWizard::sac;
using namespace casaWizard;

TEST(OptimizedTCHPExtractorTest, testInitialization)
{
  // Given
  std::unique_ptr<StubThermalScenario> scenario(new StubThermalScenario);
  scenario->setWorkingDirectory("workingDirectory");
  scenario->setRunLocation("CaseSet");

  // When
  OptimizedTCHPExtractor extractor(*scenario);

  // Then
  EXPECT_EQ(extractor.iterationPath(), QString("workingDirectory/calibration_step1/CaseSet/Iteration_1"));
}

TEST(OptimizedTCHPExtractorTest, testExtract)
{
  // Given
  std::unique_ptr<StubThermalScenario> scenario(new StubThermalScenario);
  scenario->setWorkingDirectory("workingDirectory");
  scenario->setRunLocation("CaseSet");
  CalibrationTargetManager& ctManager = scenario->calibrationTargetManager();
  ctManager.addWell("Well 1", 1.0, 1.0);
  ctManager.addWell("Well 2", 2.0, 16.2);
  OptimizedTCHPExtractor extractor(*scenario);

  // When
  extractor.extract();
  extractor.script().generateCommands();

  // Then
  EXPECT_EQ(extractor.script().commands().size(), 3); // size is number of wells + 1 due to the "which casa" command
}

TEST(OptimizedTCHPExtractorTest, testGetDataCreator)
{
  // Given
  std::unique_ptr<StubThermalScenario> scenario(new StubThermalScenario);
  scenario->setWorkingDirectory("workingDirectory");
  scenario->setRunLocation("CaseSet");
  OptimizedTCHPExtractor extractor(*scenario);

  // Then
  EXPECT_NO_THROW(extractor.dataCreator());
}

