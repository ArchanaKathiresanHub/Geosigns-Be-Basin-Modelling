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

TEST( CasaScenarioTest, testWriteToFile )
{
  // Definition of the scenario to write
  casaWizard::StubCasaScenario writeScenario{};
  QString workingDirectory{"workdir"};
  const QString filename{"scenario.dat"};

  casaWizard::CalibrationTargetManager& ctManagerWriter = writeScenario.calibrationTargetManager();

  writeScenario.setWorkingDirectory(workingDirectory);
  ctManagerWriter.addWell("Test1", 10, 10);
  ctManagerWriter.addWell("Test2", 3, 4);

  casaWizard::CalibrationTargetManager& ctManagerWrite = writeScenario.calibrationTargetManager();
  ctManagerWrite.addCalibrationTarget("Target1", "Temperature", 1, 2, 3);
  ctManagerWrite.addCalibrationTarget("Target2", "VRe", 1, 6, 7);

  ctManagerWrite.setObjectiveFunctionVariables({"Temperature","VRe"});
  ctManagerWrite.setObjectiveFunction(0, 0, 1);
  ctManagerWrite.setObjectiveFunction(0, 1, 2);
  ctManagerWrite.setObjectiveFunction(0, 2, 3);
  ctManagerWrite.setObjectiveFunction(1, 0, 4);

  // Writing the scenario
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

  const QVector<casaWizard::Well>& wells1 = ctManagerWriter.wells();
  const QVector<casaWizard::Well>& wells2 = ctManagerReader.wells();

  ASSERT_EQ(wells1.size(), wells2.size());
  for (int i=0; i<wells1.size(); ++i)
  {
    EXPECT_EQ(wells1[i].id(),              wells2[i].id());
    EXPECT_EQ(wells1[i].name(),            wells2[i].name());
    EXPECT_DOUBLE_EQ(wells1[i].x(),        wells2[i].x());
    EXPECT_DOUBLE_EQ(wells1[i].y(),        wells2[i].y());
    EXPECT_DOUBLE_EQ(wells1[i].isActive(), wells2[i].isActive());
  }

  casaWizard::CalibrationTargetManager& ctManagerRead = readScenario.calibrationTargetManager();
  const QVector<const casaWizard::CalibrationTarget*> targets = ctManagerRead.calibrationTargets();

  ASSERT_EQ(targets.size(), 2);

  EXPECT_EQ(targets[0]->name().toStdString(), "Target1");
  EXPECT_EQ(targets[0]->property().toStdString(), "Temperature");
  EXPECT_DOUBLE_EQ(targets[0]->z(), 2);
  EXPECT_DOUBLE_EQ(targets[0]->value(), 3);

  EXPECT_EQ(targets[1]->name().toStdString(), "Target2");
  EXPECT_EQ(targets[1]->property().toStdString(), "VRe");
  EXPECT_DOUBLE_EQ(targets[1]->z(), 6);
  EXPECT_DOUBLE_EQ(targets[1]->value(), 7);

  const casaWizard::ObjectiveFunctionManager& objectiveFunctionRead = ctManagerRead.objectiveFunctionManager();
  QStringList variables{objectiveFunctionRead.variables()};
  ASSERT_EQ(variables.size(), 2);
  EXPECT_EQ(variables[0].toStdString(), "Temperature");
  EXPECT_EQ(variables[1].toStdString(), "VRe");

  EXPECT_DOUBLE_EQ(objectiveFunctionRead.absoluteError(0), 1);
  EXPECT_DOUBLE_EQ(objectiveFunctionRead.relativeError(0), 2);
  EXPECT_DOUBLE_EQ(objectiveFunctionRead.weight(0), 3);
  EXPECT_DOUBLE_EQ(objectiveFunctionRead.absoluteError(1), 4);
}
