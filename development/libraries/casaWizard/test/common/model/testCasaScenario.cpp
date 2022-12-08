#include "model/casaScenario.h"
#include "model/scenarioWriter.h"
#include "model/scenarioReader.h"
#include "model/well.h"
#include "stubCasaScenario.h"

#include <gtest/gtest.h>

TEST( CasaScenarioTest, testAddOneWell )
{
  casaWizard::StubCasaScenario scenario{};

  const int wellIndexExpected{0};

  const QString wellName{"test"};
  const double x{0.0};
  const double y{0.0};

  casaWizard::CalibrationTargetManager& ctManager = scenario.calibrationTargetManager();

  const int wellIndexActualReturn = ctManager.addWell(wellName, x, y);
  EXPECT_EQ(wellIndexActualReturn, wellIndexExpected)
      << "Returned well index does not match expected value";

  const casaWizard::Well& well = ctManager.well(0);
  const int wellIndexActualData = well.id();
  EXPECT_EQ(wellIndexActualData, wellIndexExpected)
      << "Well index in data does not match expected value";
}

void defineScenario(casaWizard::StubCasaScenario& scenario)
{
  scenario.setWorkingDirectory("workdir");
  scenario.setExpertUser(true);
  scenario.setNumberCPUs(10);
  scenario.setRunLocation("TestRun");
  scenario.setApplicationName("fastcauldron");
  scenario.setClusterName("ClusterName");
}

TEST( CasaScenarioTest, testSetScenarioProperties)
{
  // Given
  casaWizard::StubCasaScenario scenario{};

  // When
  defineScenario(scenario);

  // Then
  EXPECT_EQ(scenario.workingDirectory().toStdString(), "workdir");
  EXPECT_EQ(scenario.project3dPath().toStdString(), "");
  EXPECT_EQ(scenario.expertUser(), true);
  EXPECT_EQ(scenario.numberCPUs(), 10);
  EXPECT_EQ(scenario.applicationName().toStdString(), "fastcauldron");
  EXPECT_EQ(scenario.clusterName().toStdString(), "ClusterName");
  EXPECT_EQ(scenario.runLocation().toStdString(), "TestRun");
}

TEST( CasaScenarioTest, testClearScenario )
{
  // Given
  casaWizard::StubCasaScenario scenario{};
  defineScenario(scenario);

  // When
  scenario.clear();

  // Then
  EXPECT_EQ(scenario.project3dPath().toStdString(), "");
  EXPECT_EQ(scenario.workingDirectory().toStdString(), "");
  EXPECT_EQ(scenario.expertUser(), false);
  EXPECT_EQ(scenario.numberCPUs(), 1);
  EXPECT_EQ(scenario.applicationName().toStdString(), "fastcauldron \"-itcoupled\"");
  EXPECT_EQ(scenario.clusterName().toStdString(), "LOCAL");
  EXPECT_EQ(scenario.runLocation().toStdString(), "CaseSet");
}

TEST( CasaScenarioTest, testApplyObjectiveFunctionOnCalibrationTargets )
{
  // Given
  casaWizard::StubCasaScenario scenario{};
  defineScenario(scenario);

  casaWizard::CalibrationTargetManager& ctManagerWriter = scenario.calibrationTargetManager();
  ctManagerWriter.addToMapping("Temperature", "Temperature");
  ctManagerWriter.addToMapping("Vre", "VRe");
  ctManagerWriter.addWell("Test1", 10, 10);
  ctManagerWriter.addWell("Test2", 3, 4);
  ctManagerWriter.addCalibrationTarget("Target1", "Temperature", 1, 2, 3);
  ctManagerWriter.addCalibrationTarget("Target2", "Vre", 1, 6, 7);

  casaWizard::ObjectiveFunctionManager& ofManagerWriter = scenario.objectiveFunctionManager();
  ofManagerWriter.setVariables({"Temperature","Vre"}, ctManagerWriter.userNameToCauldronNameMapping());
  ofManagerWriter.setValue(0, 0, 1);
  ofManagerWriter.setValue(0, 1, 2);
  ofManagerWriter.setValue(0, 2, 0.75);
  ofManagerWriter.setValue(1, 0, 4);
  ofManagerWriter.setValue(1, 1, 6);
  ofManagerWriter.setValue(1, 2, 0.25);

  // When
  scenario.applyObjectiveFunctionOnCalibrationTargets();

  // Then
  const QVector<const casaWizard::CalibrationTarget*> targets = ctManagerWriter.calibrationTargets();

  EXPECT_DOUBLE_EQ(targets[0]->standardDeviation(), 1 + 2 * 3);
  EXPECT_DOUBLE_EQ(targets[1]->standardDeviation(), 4 + 6 * 7);

  EXPECT_DOUBLE_EQ(targets[0]->uaWeight(), 0.75);
  EXPECT_DOUBLE_EQ(targets[1]->uaWeight(), 0.25);
}

TEST( CasaScenarioTest, testWriteToFile )
{
  // Definition of the scenario to write
  casaWizard::StubCasaScenario writeScenario{};
  defineScenario(writeScenario);

  casaWizard::CalibrationTargetManager& ctManagerWriter = writeScenario.calibrationTargetManager();
  ctManagerWriter.addToMapping("Temperature", "Temperature");
  ctManagerWriter.addToMapping("Vre", "VRe");
  ctManagerWriter.addWell("Test1", 10, 10);
  ctManagerWriter.addWell("Test2", 3, 4);
  ctManagerWriter.addCalibrationTarget("Target1", "Temperature", 1, 2, 3);
  ctManagerWriter.addCalibrationTarget("Target2", "Vre", 1, 6, 7);

  casaWizard::ObjectiveFunctionManager& ofManagerWriter = writeScenario.objectiveFunctionManager();
  ofManagerWriter.setVariables({"Temperature","Vre"}, ctManagerWriter.userNameToCauldronNameMapping());
  ofManagerWriter.setValue(0, 0, 1);
  ofManagerWriter.setValue(0, 1, 2);
  ofManagerWriter.setValue(0, 2, 3);
  ofManagerWriter.setValue(1, 0, 4);
  writeScenario.applyObjectiveFunctionOnCalibrationTargets();

  // Writing the scenario
  const QString filename{"scenario.dat"};
  casaWizard::ScenarioWriter writer{filename};
  writeScenario.writeToFile(writer);
  writer.close();

  // Reading the scenario
  casaWizard::StubCasaScenario readScenario{};
  casaWizard::ScenarioReader reader{filename};
  readScenario.readFromFile(reader);
  casaWizard::CalibrationTargetManager& ctManagerReader = readScenario.calibrationTargetManager();

  // Checking the read of the scenario
  EXPECT_EQ(writeScenario.workingDirectory().toStdString(), readScenario.workingDirectory().toStdString());
  EXPECT_EQ(writeScenario.expertUser(), readScenario.expertUser());
  EXPECT_EQ(writeScenario.numberCPUs(), readScenario.numberCPUs());
  EXPECT_EQ(writeScenario.applicationName().toStdString(), readScenario.applicationName().toStdString());
  EXPECT_EQ(writeScenario.clusterName().toStdString(), readScenario.clusterName().toStdString());
  EXPECT_EQ(writeScenario.runLocation().toStdString(), readScenario.runLocation().toStdString());
  EXPECT_EQ(ctManagerWriter.getCauldronPropertyName("Temperature"),
            ctManagerReader.getCauldronPropertyName("Temperature"));
  EXPECT_EQ(ctManagerWriter.getCauldronPropertyName("Vre"),
            ctManagerReader.getCauldronPropertyName("Vre"));
  EXPECT_EQ(ctManagerWriter.getCauldronPropertyName("NoProperty"),
            ctManagerReader.getCauldronPropertyName("NoProperty"));

  const QVector<const casaWizard::Well*>& wells1 = ctManagerWriter.wells();
  const QVector<const casaWizard::Well*>& wells2 = ctManagerReader.wells();

  ASSERT_EQ(wells1.size(), wells2.size());
  for (int i=0; i<wells1.size(); ++i)
  {
    EXPECT_EQ(wells1[i]->id(),              wells2[i]->id());
    EXPECT_EQ(wells1[i]->name(),            wells2[i]->name());
    EXPECT_DOUBLE_EQ(wells1[i]->x(),        wells2[i]->x());
    EXPECT_DOUBLE_EQ(wells1[i]->y(),        wells2[i]->y());
    EXPECT_DOUBLE_EQ(wells1[i]->isActive(), wells2[i]->isActive());
  }

  casaWizard::CalibrationTargetManager& ctManagerRead = readScenario.calibrationTargetManager();
  const QVector<const casaWizard::CalibrationTarget*> targets = ctManagerRead.calibrationTargets();

  ASSERT_EQ(targets.size(), 2);

  EXPECT_EQ(targets[0]->name().toStdString(), "Target1");
  EXPECT_EQ(targets[0]->propertyUserName().toStdString(), "Temperature");
  EXPECT_DOUBLE_EQ(targets[0]->z(), 2);
  EXPECT_DOUBLE_EQ(targets[0]->value(), 3);

  EXPECT_EQ(targets[1]->name().toStdString(), "Target2");
  EXPECT_EQ(targets[1]->propertyUserName().toStdString(), "Vre");
  EXPECT_DOUBLE_EQ(targets[1]->z(), 6);
  EXPECT_DOUBLE_EQ(targets[1]->value(), 7);

  const casaWizard::ObjectiveFunctionManager& objectiveFunctionRead = readScenario.objectiveFunctionManager();
  QStringList variables{objectiveFunctionRead.variablesCauldronNames()};
  ASSERT_EQ(variables.size(), 2);
  EXPECT_EQ(variables[0].toStdString(), "Temperature");
  EXPECT_EQ(variables[1].toStdString(), "VRe");


  EXPECT_DOUBLE_EQ(objectiveFunctionRead.absoluteError(0), 1);
  EXPECT_DOUBLE_EQ(objectiveFunctionRead.relativeError(0), 2);
  EXPECT_DOUBLE_EQ(objectiveFunctionRead.weight(0), 3);
  EXPECT_DOUBLE_EQ(objectiveFunctionRead.absoluteError(1), 4);
}

TEST(CasaScenarioTest, testGetDefaultFileDialogFolderNonExistingPaths)
{
   casaWizard::StubCasaScenario scenario{};

   scenario.m_amsterdamDirPath = "./NonExistantPath";
   scenario.m_houstonDirPath = "./NonExistantPath";

   EXPECT_EQ(scenario.defaultDirectoryLocation(), "./");
}

TEST(CasaScenarioTest, testGetDefaultFileDialogFolderAMSPathExists)
{
   casaWizard::StubCasaScenario scenario{};

   scenario.m_amsterdamDirPath = "./AMS";
   scenario.m_houstonDirPath = "./NonExistantPath";

   EXPECT_EQ(scenario.defaultDirectoryLocation().toStdString(), "./AMS/pt.sgs/data.nobackup/bpa2tools");
}

TEST(CasaScenarioTest, testGetDefaultFileDialogFolderHOUPathExists)
{
   casaWizard::StubCasaScenario scenario{};

   scenario.m_amsterdamDirPath = "./NonExistantPath";
   scenario.m_houstonDirPath = "./HOU";

   EXPECT_EQ(scenario.defaultDirectoryLocation().toStdString(), "./HOU/pt.sgs/data.nobackup/bpa2tools");
}

TEST(CasaScenarioTest, testGetDefaultFileDialogFolderHOUPathExistsAndWorkingDirectoryIsSet)
{
   casaWizard::StubCasaScenario scenario{};
   scenario.setWorkingDirectory("./BaseFolder/WorkingDir");

   scenario.m_amsterdamDirPath = "./NonExistantPath";
   scenario.m_houstonDirPath = "./HOU";

   EXPECT_EQ(scenario.defaultDirectoryLocation().toStdString(), "./BaseFolder/WorkingDir");
}

TEST(CasaScenarioTest, testGetDefaultFileDialogFolderWorkingDirectoryIsSetWithOneFolderHigher)
{
   casaWizard::StubCasaScenario scenario{};
   scenario.setWorkingDirectory("./BaseFolder/WorkingDir");

   scenario.m_amsterdamDirPath = "./NonExistantPath";
   scenario.m_houstonDirPath = "./NonExistantPath";

   const bool oneFolderHigher = true;
   EXPECT_EQ(scenario.defaultDirectoryLocation(oneFolderHigher).toStdString(), "./BaseFolder");
}



