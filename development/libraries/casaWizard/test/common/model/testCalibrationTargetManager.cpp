#include "model/calibrationTargetManager.h"

#include "model/calibrationTarget.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"
#include "model/well.h"

#include "expectFileEq.h"

#include <gtest/gtest.h>

TEST(CalibrationTargetManagerTest, testWriteToFile)
{
  casaWizard::CalibrationTargetManager manager;

  // Note: calibration targets of each well should be added right after the well
  // is added. Here, there are two wells, each well having one calibration target.
  const int expectedIDWell0 = 0;
  const int expectedIDWell1 = 1;
  const int actualIDWell0 = manager.addWell("testWell0", 0.0, 1.0);
  manager.addCalibrationTarget("Target1", "Temperature", actualIDWell0, 2.0, 3.0);
  const int actualIDWell1 = manager.addWell("testWell1", 4.0, 5.0);
  manager.addCalibrationTarget("Target2", "VRe", actualIDWell1, 6.0, 7.0);
  EXPECT_EQ(expectedIDWell0, actualIDWell0);
  EXPECT_EQ(expectedIDWell1, actualIDWell1);

  manager.setWellIsActive(true, 0);
  manager.setWellIsActive(false, 1);

  casaWizard::ScenarioWriter writer{"calibrationTargetManagerActual.dat"};
  manager.writeToFile(writer);
  writer.close();

  expectFileEq("calibrationTargetManager.dat", "calibrationTargetManagerActual.dat");
}

TEST(CalibrationTargetManagerTest, testReadFromFile)
{
  casaWizard::CalibrationTargetManager manager;
  casaWizard::ScenarioReader reader{"calibrationTargetManager.dat"};
  manager.readFromFile(reader);

  const int expectedWellsSize = 2;
  const int actualWellsSize = manager.wells().size();
  ASSERT_EQ(expectedWellsSize, actualWellsSize);

  const casaWizard::Well& well0 = manager.well(0);
  const casaWizard::Well& well1 = manager.well(1);

  const int expectedIDWell0 = 0;
  const int expectedIDWell1 = 1;
  const int actualIDWell0 = well0.id();
  const int actualIDWell1 = well1.id();
  EXPECT_EQ(expectedIDWell0, actualIDWell0);
  EXPECT_EQ(expectedIDWell1, actualIDWell1);

  const QString expectedNameWell0 = "testWell0";
  const QString expectedNameWell1 = "testWell1";
  const QString actualNameWell0 = well0.name();
  const QString actualNameWell1 = well1.name();
  EXPECT_EQ(expectedNameWell0, actualNameWell0);
  EXPECT_EQ(expectedNameWell1, actualNameWell1);

  const bool expectedIsActiveWell0 = true;
  const bool expectedIsActiveWell1 = false;
  const bool actualIsActiveWell0 = well0.isActive();
  const bool actualIsActiveWell1 = well1.isActive();
  EXPECT_EQ(expectedIsActiveWell0, actualIsActiveWell0);
  EXPECT_EQ(expectedIsActiveWell1, actualIsActiveWell1);

  const double expectedCoordinateXWell0 = 0.0;
  const double expectedCoordinateYWell0 = 1.0;
  const double expectedCoordinateXWell1 = 4.0;
  const double expectedCoordinateYWell1 = 5.0;
  const double actualCoordinateXWell0 = well0.x();
  const double actualCoordinateYWell0 = well0.y();
  const double actualCoordinateXWell1 = well1.x();
  const double actualCoordinateYWell1 = well1.y();
  EXPECT_DOUBLE_EQ(expectedCoordinateXWell0, actualCoordinateXWell0);
  EXPECT_DOUBLE_EQ(expectedCoordinateYWell0, actualCoordinateYWell0);
  EXPECT_DOUBLE_EQ(expectedCoordinateXWell1, actualCoordinateXWell1);
  EXPECT_DOUBLE_EQ(expectedCoordinateYWell1, actualCoordinateYWell1);

  const int expectedAmountOfActiveWells = 1;
  const int actualAmountOfActiveWells = manager.amountOfActiveCalibrationTargets();
  EXPECT_EQ(expectedAmountOfActiveWells, actualAmountOfActiveWells);

  const QVector<const casaWizard::CalibrationTarget*> targets = manager.calibrationTargets();

  ASSERT_EQ(targets.size(), 2);

  EXPECT_EQ(targets[0]->name().toStdString(), "Target1");
  EXPECT_EQ(targets[0]->property().toStdString(), "Temperature");
  EXPECT_DOUBLE_EQ(targets[0]->z(), 2.0);
  EXPECT_DOUBLE_EQ(targets[0]->value(), 3.0);
  EXPECT_DOUBLE_EQ(targets[0]->standardDeviation(), 1);

  EXPECT_EQ(targets[1]->name().toStdString(), "Target2");
  EXPECT_EQ(targets[1]->property().toStdString(), "VRe");
  EXPECT_DOUBLE_EQ(targets[1]->z(), 6.0);
  EXPECT_DOUBLE_EQ(targets[1]->value(), 7.0);
  EXPECT_DOUBLE_EQ(targets[1]->standardDeviation(), 1);
}

TEST(CalibrationTargetManagerTest, testActiveCalibrationTargetsGetter)
{
  casaWizard::CalibrationTargetManager manager;

  // Note: calibration targets of each well should be added right after the well
  // is added. Here, there are two wells, each well having one calibration target.
  const int actualIDWell0 = manager.addWell("testWell0", 0.0, 1.0);
  manager.addCalibrationTarget("Target1", "Temperature", actualIDWell0, 2.0, 3.0);
  const int actualIDWell1 = manager.addWell("testWell1", 4.0, 5.0);
  manager.addCalibrationTarget("Target2", "VRe", actualIDWell1, 6.0, 7.0);

  manager.setWellIsActive(false, 0);
  manager.setWellIsActive(true, 1);

  QVector<const casaWizard::CalibrationTarget*> targets = manager.activeCalibrationTargets();
  ASSERT_EQ(targets.size(), 1);
  EXPECT_EQ(targets[0]->name().toStdString(), "Target2");
}

TEST(CalibrationTargetManagerTest, testExtractWells)
{
  casaWizard::CalibrationTargetManager manager;
  const int actualIDWell0 = manager.addWell("testWell0", 0.0, 1.0);

  const double valueT1Expected{1};
  const double valueT2Expected{2};
  const double valueTWTExcpected{3};

  manager.addCalibrationTarget("A","Temperature", actualIDWell0, 0.0, valueT1Expected);
  manager.addCalibrationTarget("A","Temperature", actualIDWell0, 0.0, valueT2Expected);
  manager.addCalibrationTarget("A","TwoWayTime",  actualIDWell0, 0.0, valueTWTExcpected);

  QStringList propertiesExpected{"Temperature", "TwoWayTime"};
  QStringList propertiesActual;

  QVector<QVector<casaWizard::CalibrationTarget>> targetsInWell = manager.extractWellTargets(propertiesActual, 0);

  for (int i = 1; i < propertiesExpected.size(); ++i )
  {
    EXPECT_EQ(propertiesActual[i].toStdString(), propertiesExpected[i].toStdString());
  }

  const double valueT1Actual = targetsInWell[0][0].value();
  const double valueT2Actual = targetsInWell[0][1].value();
  const double valueTWTActual = targetsInWell[1][0].value();

  EXPECT_EQ(valueT1Actual, valueT1Expected);
  EXPECT_EQ(valueT2Actual, valueT2Expected);
  EXPECT_EQ(valueTWTActual, valueTWTExcpected);
}

TEST(CalibrationTargetManagerTest, testExtractMultipleWells)
{
  casaWizard::CalibrationTargetManager manager;
  const int actualIDWell0 = manager.addWell("testWell0", 0.0, 1.0);
  const int actualIDWell1 = manager.addWell("testWell1", 0.0, 1.0);
  const int actualIDWell2 = manager.addWell("testWell2", 0.0, 1.0);

  const double valueT1Expected{1};
  const double valueT2Expected{2};
  const double valueTWTExcpected{3};

  manager.addCalibrationTarget("A","Temperature", actualIDWell0, 0.0, valueT1Expected);
  manager.addCalibrationTarget("A","Temperature", actualIDWell1, 0.0, valueT2Expected);
  manager.addCalibrationTarget("A","TwoWayTime",  actualIDWell2, 0.0, valueTWTExcpected);

  QStringList propertiesExpected{"Temperature", "TwoWayTime"};
  QStringList propertiesActual;

  QVector<QVector<casaWizard::CalibrationTarget>> targetsInWell = manager.extractWellTargets(propertiesActual, {0,2});

  for (int i = 1; i < propertiesExpected.size(); ++i )
  {
    EXPECT_EQ(propertiesActual[i].toStdString(), propertiesExpected[i].toStdString());
  }

  const double valueT1Actual = targetsInWell[0][0].value();
  const double valueTWTActual = targetsInWell[1][0].value();

  EXPECT_EQ(valueT1Actual, valueT1Expected);
  EXPECT_EQ(valueTWTActual, valueTWTExcpected);
}

TEST(CalibrationTargetManagerTest, testActiveWells)
{
  casaWizard::CalibrationTargetManager manager;
  const int wellID0 = manager.addWell("testWell0", 0.0, 1.0);
  const int wellID1 = manager.addWell("testWell1", 2.0, 3.0);
  manager.setWellIsActive(false, 0);
  manager.setWellIsActive(true, 1);

  const QVector<const casaWizard::Well*> activeWells = manager.activeWells();

  ASSERT_EQ(1, activeWells.size());
  EXPECT_EQ(wellID1, activeWells[0]->id());
}

TEST(CalibrationTargetManagerTest, testActiveProperties)
{
  casaWizard::CalibrationTargetManager manager;
  const int wellID0 = manager.addWell("testWell0", 0.0, 1.0);
  const int wellID1 = manager.addWell("testWell1", 2.0, 3.0);

  manager.addCalibrationTarget("A", "Temperature",   wellID0, 0.0, 0.1);
  manager.addCalibrationTarget("A", "Temperature",   wellID0, 1.0, 0.1);
  manager.addCalibrationTarget("A", "SonicSlowness", wellID0, 2.0, 0.1);
  manager.addCalibrationTarget("A", "TwoWayTime",    wellID0, 3.0, 0.1);
  manager.addCalibrationTarget("A", "MaxVES",        wellID1, 4.0, 0.1);
  manager.addCalibrationTarget("A", "Temperature",   wellID1, 5.0, 0.1);

  const QStringList propertiesActual = manager.activeProperties();
  const QStringList propertiesExpected = {"Temperature", "SonicSlowness", "TwoWayTime", "MaxVES"};
  ASSERT_EQ(4, propertiesActual.size());
  for (int i=0; i<4; i++)
  {
    EXPECT_EQ(propertiesExpected[i], propertiesActual[i]);
  }

  manager.setWellIsActive(false, 1);
  const QStringList activePropertiesActual = manager.activeProperties();
  const QStringList activePropertiesExpected = {"Temperature", "SonicSlowness", "TwoWayTime"};
  ASSERT_EQ(3, activePropertiesActual.size());
  for (int i=0; i<3; i++)
  {
    EXPECT_EQ(activePropertiesExpected[i], activePropertiesActual[i]);
  }
}