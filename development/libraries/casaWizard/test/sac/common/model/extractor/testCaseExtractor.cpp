#include "model/extractor/caseExtractor.h"

#include "model/calibrationTargetManager.h"

#include "stubSACScenario.h"
#include "stubCaseExtractor.h"

#include <gtest/gtest.h>
#include <memory>
using namespace casaWizard::sac;
using namespace casaWizard;

TEST(CaseExtractorTest, testInitialization)
{
  // Given
  std::unique_ptr<StubSacLithologyScenario> scenario(new StubSacLithologyScenario);
  scenario->setWorkingDirectory("workingDirectory");
  scenario->setRunLocation("CaseSet");

  // When
  StubCaseExtractor extractor(*scenario);

  // Then
  EXPECT_EQ(extractor.iterationPath().toStdString(), "workingDirectory/calibration_step1/CaseSet/Iteration_1");
}

TEST(CaseExtractorTest, testInitializationWithInvalidWorkingDirectory)
{
  // Given
  std::unique_ptr<StubSacLithologyScenario> scenario(new StubSacLithologyScenario);
  scenario->setWorkingDirectory("doesNotExist");
  scenario->setRunLocation("CaseSet");

  // When
  StubCaseExtractor extractor(*scenario);

  // Then
  EXPECT_EQ(extractor.iterationPath(), QString(""));
}

TEST(CaseExtractorTest, testInitializationWithInvalidRunLocation)
{
  // Given
  std::unique_ptr<StubSacLithologyScenario> scenario(new StubSacLithologyScenario);
  scenario->setWorkingDirectory("workingDirectory");
  scenario->setRunLocation("doesNotExist");

  // When
  StubCaseExtractor extractor(*scenario);

  // Then
  EXPECT_EQ(extractor.iterationPath(), QString(""));
}

TEST(CaseExtractorTest, testInitializationWithoutIterationFolders)
{
  // Given
  std::unique_ptr<StubSacLithologyScenario> scenario(new StubSacLithologyScenario);
  scenario->setWorkingDirectory("workingDirectoryNoIterations");
  scenario->setRunLocation("CaseSet");

  // When
  StubCaseExtractor extractor(*scenario);

  // Then
  EXPECT_EQ(extractor.iterationPath(), QString(""));
}


TEST(CaseExtractorTest, testExtract)
{
  // Given
  std::unique_ptr<StubSacLithologyScenario> scenario(new StubSacLithologyScenario);
  scenario->setWorkingDirectory("workingDirectory");
  scenario->setRunLocation("CaseSet");
  CalibrationTargetManager& ctManager = scenario->calibrationTargetManager();
  // Wells will be sorted in ascending x order
  ctManager.addWell("Well 4", 4.0, 2.0);
  ctManager.addWell("Well 3 No Directory", 3.0, 10.2);
  ctManager.addWell("Well 2", 2.0, 16.2);
  ctManager.addWell("Well 1", 1.0, 1.0);

  StubCaseExtractor extractor(*scenario);

  // When
  extractor.extract();

  // Then
  EXPECT_EQ(extractor.updatedCases().size(), 3);

  // Since Wells were sorted in ascending order, these case folders are in descending order
  EXPECT_EQ(extractor.updatedCases().at(0), "Case_4");
  EXPECT_EQ(extractor.updatedCases().at(2), "Case_2");
  EXPECT_EQ(extractor.updatedCases().at(3), "Case_1");
}

TEST(CaseExtractorTest, testExtractInvalidWorkingDirectory)
{
  // Given
  std::unique_ptr<StubSacLithologyScenario> scenario(new StubSacLithologyScenario);
  scenario->setWorkingDirectory("doesNotExist");
  scenario->setRunLocation("CaseSet");
  CalibrationTargetManager& ctManager = scenario->calibrationTargetManager();
  ctManager.addWell("Well 1", 1.0, 1.0);
  StubCaseExtractor extractor(*scenario);

  // When
  extractor.extract();

  // Then
  EXPECT_EQ(extractor.updatedCases().size(), 0);
}

TEST(CaseExtractorTest, testGetScenario)
{
  // Given
  std::unique_ptr<StubSacLithologyScenario> scenario(new StubSacLithologyScenario);
  scenario->setWorkingDirectory("workingDirectory");
  scenario->setRunLocation("CaseSet");

  // When
  StubCaseExtractor extractor(*scenario);

  // Then
  EXPECT_EQ(scenario->workingDirectory(), extractor.scenario().workingDirectory());
}
