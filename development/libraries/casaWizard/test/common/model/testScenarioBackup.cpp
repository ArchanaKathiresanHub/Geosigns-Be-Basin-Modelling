#include "model/scenarioBackup.h"

#include "model/casaScenario.h"
#include "stubProjectReader.h"

#include <QDir>
#include <gtest/gtest.h>

TEST(ScenarioBackupTest, testBackup)
{
  // Given
  casaWizard::CasaScenario scenario(new casaWizard::StubProjectReader);
  scenario.setWorkingDirectory(".");
  QDir dir( "./autosave" );
  dir.setFilter( QDir::AllEntries | QDir::NoDotAndDotDot );
  dir.removeRecursively(); // clean up previous test runs

  // When
  casaWizard::scenarioBackup::backup(scenario);

  // Then
  EXPECT_EQ(dir.entryList().size(), 1);
  EXPECT_TRUE(dir.entryList()[0].endsWith(".dat")); // only test if a .dat file is created, the writer is tested separately
}
