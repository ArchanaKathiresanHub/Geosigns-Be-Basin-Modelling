#include "model/calibrationTargetManager.h"

#include "model/calibrationTarget.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"
#include "model/well.h"

#include "expectFileEq.h"

#include <QSet>

#include <gtest/gtest.h>

TEST(CalibrationTargetManagerTest, testWriteToFile)
{
  casaWizard::CalibrationTargetManager manager;

  // Note: calibration targets of each well should be added right after the well
  // is added. Here, there are two wells, each well having one calibration target.
  manager.addToMapping("Temperature", "Temperature");
  manager.addToMapping("VReUserName", "VRe");

  const int expectedIDWell0 = 0;
  const int expectedIDWell1 = 1;
  const int actualIDWell0 = manager.addWell("testWell0", 0.0, 1.0);
  manager.addCalibrationTarget("Target1", "Temperature", actualIDWell0, 2.0, 3.0);
  const int actualIDWell1 = manager.addWell("testWell1", 4.0, 5.0);
  manager.addCalibrationTarget("Target2", "VReUserName", actualIDWell1, 6.0, 7.0);
  EXPECT_EQ(expectedIDWell0, actualIDWell0);
  EXPECT_EQ(expectedIDWell1, actualIDWell1);

  manager.setWellIsActive(false, 0);
  manager.setWellIsExcluded(false, 0);
  manager.setWellActiveProperty("Temperature", false, 0);
  manager.setWellActiveProperty("VRe", true, 0);

  manager.setWellIsActive(true, 1);
  manager.setWellIsExcluded(true, 1);

  casaWizard::ScenarioWriter writer{"calibrationTargetManagerActual.dat"};
  manager.writeToFile(writer);
  writer.close();

  expectFileEq("calibrationTargetManagerVersion1.dat", "calibrationTargetManagerActual.dat");
}

void testReading(const QString& testFile, const int version)
{
  casaWizard::CalibrationTargetManager manager;
  casaWizard::ScenarioReader reader{testFile};
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

  //Test the other method of obtaining well names
  const QStringList wellNames = manager.getWellNames();
  EXPECT_EQ(wellNames[0], actualNameWell0);
  EXPECT_EQ(wellNames[1], actualNameWell1);

  const bool expectedIsActiveWell0 = false;
  const bool expectedIsActiveWell1 = true;
  const bool actualIsActiveWell0 = well0.isActive();
  const bool actualIsActiveWell1 = well1.isActive();
  EXPECT_EQ(expectedIsActiveWell0, actualIsActiveWell0);
  EXPECT_EQ(expectedIsActiveWell1, actualIsActiveWell1);

  const bool expectedIsExcludedWell0 = false;
  const bool expectedIsExcludedWell1 = true;
  const bool actualIsExcludedWell0 = well0.isExcluded();
  const bool actualIsExcludedWell1 = well1.isExcluded();
  EXPECT_EQ(expectedIsExcludedWell0, actualIsExcludedWell0);
  EXPECT_EQ(expectedIsExcludedWell1, actualIsExcludedWell1);

  const bool expectedIsInvalidWell0 = false;
  const bool actualIsIsInvalidWell0 = well0.isInvalid();
  EXPECT_EQ(expectedIsInvalidWell0, actualIsIsInvalidWell0);

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
  EXPECT_EQ(targets[0]->propertyUserName().toStdString(), "Temperature");
  EXPECT_DOUBLE_EQ(targets[0]->z(), 2.0);
  EXPECT_DOUBLE_EQ(targets[0]->value(), 3.0);
  EXPECT_DOUBLE_EQ(targets[0]->standardDeviation(), 0);
  EXPECT_EQ(targets[1]->name().toStdString(), "Target2");
  EXPECT_EQ(targets[1]->propertyUserName().toStdString(), "VReUserName");
  EXPECT_DOUBLE_EQ(targets[1]->z(), 6.0);
  EXPECT_DOUBLE_EQ(targets[1]->value(), 7.0);
  EXPECT_DOUBLE_EQ(targets[1]->standardDeviation(), 0);
}

TEST(CalibrationTargetManagerTest, testReadVersion0FromFile)
{
  const QString testFile = "calibrationTargetManagerVersion0.dat";
  testReading(testFile, 0);
}

TEST(CalibrationTargetManagerTest, testReadVersion1FromFile)
{
  const QString testFile = "calibrationTargetManagerVersion1.dat";
  testReading(testFile, 1);
}

TEST(CalibrationTargetManagerTest, testActiveCalibrationTargetsGetter)
{
  casaWizard::CalibrationTargetManager manager;

  // Note: calibration targets of each well should be added right after the well
  // is added. Here, there are two wells, each well having one calibration target.
  const int actualIDWell0 = manager.addWell("testWell0", 0.0, 1.0);
  manager.addCalibrationTarget("Target1", "Temp", actualIDWell0, 2.0, 3.0);
  const int actualIDWell1 = manager.addWell("testWell1", 4.0, 5.0);
  manager.addCalibrationTarget("Target2", "Vre", actualIDWell1, 6.0, 7.0);

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
  manager.addCalibrationTarget("A", "TWTT", actualIDWell0, 0.0, valueTWTExcpected);

  QStringList propertiesExpected{"Temperature", "TWTT"};
  QStringList propertiesActual;

  QVector<QVector<const casaWizard::CalibrationTarget*>> targetsInWell = manager.extractWellTargets(propertiesActual, 0);

  for (int i = 1; i < propertiesExpected.size(); ++i )
  {
    EXPECT_EQ(propertiesActual[i].toStdString(), propertiesExpected[i].toStdString());
  }

  const double valueT1Actual = targetsInWell[0][0]->value();
  const double valueT2Actual = targetsInWell[0][1]->value();
  const double valueTWTActual = targetsInWell[1][0]->value();

  EXPECT_EQ(valueT1Actual, valueT1Expected);
  EXPECT_EQ(valueT2Actual, valueT2Expected);
  EXPECT_EQ(valueTWTActual, valueTWTExcpected);
}

TEST(CalibrationTargetManagerTest, testActiveWells)
{
  casaWizard::CalibrationTargetManager manager;
  manager.addWell("testWell0", 0.0, 1.0);
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

  manager.addCalibrationTarget("A", "Temperature", wellID0, 0.0, 0.1);
  manager.addCalibrationTarget("A", "Temperature", wellID0, 1.0, 0.1);
  manager.addCalibrationTarget("A", "DT", wellID0, 2.0, 0.1);
  manager.addCalibrationTarget("A", "TWTT", wellID0, 3.0, 0.1);
  manager.addCalibrationTarget("A", "MaxVes", wellID1, 4.0, 0.1);
  manager.addCalibrationTarget("A", "Temperature", wellID1, 5.0, 0.1);

  const QStringList propertiesActual = manager.activePropertyUserNames();
  const QStringList propertiesExpected = {"Temperature", "DT", "TWTT", "MaxVes"};
  ASSERT_EQ(4, propertiesActual.size());
  for (int i=0; i<4; i++)
  {
    EXPECT_EQ(propertiesExpected[i], propertiesActual[i]);
  }

  manager.setWellIsActive(false, 1);
  const QStringList activePropertiesActual = manager.activePropertyUserNames();
  const QStringList activePropertiesExpected = {"Temperature", "DT", "TWTT"};
  ASSERT_EQ(3, activePropertiesActual.size());
  for (int i=0; i<3; i++)
  {
    EXPECT_EQ(activePropertiesExpected[i], activePropertiesActual[i]);
  }
}

TEST(CalibrationTargetManagerTest, testCopyMappingFrom)
{
  casaWizard::CalibrationTargetManager manager;
  manager.addToMapping("key1", "value1");
  manager.addToMapping("key2", "value2");
  manager.addToMapping("key3", "value3");

  casaWizard::CalibrationTargetManager newManager;
  newManager.addToMapping("key1", "someOtherValue");
  newManager.addToMapping("key4", "value4");
  EXPECT_EQ(newManager.getCauldronPropertyName("key1"), "someOtherValue");

  newManager.copyMappingFrom(manager);

  EXPECT_EQ(newManager.getCauldronPropertyName("key1"), "value1");
  EXPECT_EQ(newManager.getCauldronPropertyName("key2"), "value2");
  EXPECT_EQ(newManager.getCauldronPropertyName("key3"), "value3");
  EXPECT_EQ(newManager.getCauldronPropertyName("key4"), "value4");
}

TEST(CalibrationTargetManagerTest, testAppendWellsFrom)
{
  casaWizard::CalibrationTargetManager manager1;
  manager1.addWell("Well1", 100, 200);
  manager1.addWell("Well2", 200, 100);
  manager1.addToMapping("key1", "value1");
  manager1.addToMapping("key2", "Unknown");

  casaWizard::CalibrationTargetManager manager2;
  manager2.addWell("Well1-manager2", 300, 200);
  manager2.addWell("Well2-manager2", 400, 100);
  manager1.addToMapping("key2", "alreadyKnown");

  manager2.appendFrom(manager1);

  EXPECT_EQ(manager2.wells().size(), 4);
  EXPECT_EQ(manager2.getCauldronPropertyName("key1"), "value1");
  EXPECT_EQ(manager2.getCauldronPropertyName("key2"), "alreadyKnown"); // should not be overwritten with the "Unknown"

  int counter = 0;
  for (const casaWizard::Well* well : manager2.wells())
  {
    EXPECT_EQ(well->id(), counter);
    counter++;
  }
}

TEST(CalibrationTargetManagerTest, testRenamePropertyUserName)
{
  casaWizard::CalibrationTargetManager manager;
  manager.addWell("Well1", 100, 200);
  manager.addCalibrationTarget("Calibrationtarget1", "oldPropertyUserName", 0, 1.03, 4.0);
  manager.addCalibrationTarget("Calibrationtarget2", "oldPropertyUserName", 0, 1.03, 4.0);
  manager.addCalibrationTarget("Calibrationtarget3", "oldPropertyUserName", 0, 1.03, 4.0);
  manager.addCalibrationTarget("Calibrationtarget4", "oldPropertyUserName", 0, 1.03, 4.0);

  manager.addWell("Well2", 200, 100);
  manager.addCalibrationTarget("Calibrationtarget1", "oldPropertyUserName", 1, 1.03, 4.0);
  manager.addCalibrationTarget("Calibrationtarget2", "oldPropertyUserName", 1, 1.03, 4.0);
  manager.addCalibrationTarget("Calibrationtarget3", "oldPropertyUserName", 1, 1.03, 4.0);
  manager.addCalibrationTarget("Calibrationtarget4", "oldPropertyUserName", 1, 1.03, 4.0);

  manager.addToMapping("oldPropertyUserName", "someCauldronName");

  for (const auto& calibrationTarget : manager.calibrationTargets())
  {
    EXPECT_EQ(calibrationTarget->propertyUserName().toStdString(), "oldPropertyUserName");
  }

  manager.renameUserPropertyNameInWells("oldPropertyUserName", "newPropertyUserName");

  for (const auto& calibrationTarget : manager.calibrationTargets())
  {
    EXPECT_EQ(calibrationTarget->propertyUserName().toStdString(), "newPropertyUserName");
  }
}

TEST(CalibrationTargetManagerTest, testRemoveDataOutofModelDepths)
{
  std::vector<double> basementDepthsActiveAtWellLocations = {6000, 5000};
  std::vector<double> mudlineDepthsAtActiveWellLocations = {0, 150};

  casaWizard::CalibrationTargetManager manager;
  manager.addWell("Well1", 100, 200);
  manager.addCalibrationTarget("Calibrationtarget1", "TWTT", 0, -10, 4.0); // Invalid
  manager.addCalibrationTarget("Calibrationtarget2", "TWTT", 0, 100, 4.0);
  manager.addCalibrationTarget("Calibrationtarget3", "TWTT", 0, 5500, 4.0);
  manager.addCalibrationTarget("Calibrationtarget4", "TWTT", 0, 8000, 4.0); // Invalid

  manager.addWell("Well2", 200, 100);
  manager.addCalibrationTarget("Calibrationtarget1", "TWTT", 1, -10, 4.0);
  manager.addCalibrationTarget("Calibrationtarget2", "TWTT", 1, 100, 4.0);
  manager.addCalibrationTarget("Calibrationtarget3", "TWTT", 1, 5500, 4.0);
  manager.addCalibrationTarget("Calibrationtarget4", "TWTT", 1, 8000, 4.0);
  manager.setWellIsActive(false, 1);

  manager.addWell("Well3", 300, 100);
  manager.addCalibrationTarget("Calibrationtarget1", "TWTT", 2, -10, 4.0); // Invalid
  manager.addCalibrationTarget("Calibrationtarget2", "TWTT", 2, 100, 4.0); // Invalid
  manager.addCalibrationTarget("Calibrationtarget3", "TWTT", 2, 5500, 4.0); // Invalid
  manager.addCalibrationTarget("Calibrationtarget4", "TWTT", 2, 8000, 4.0); // Invalid

  manager.removeDataOutsideModelDepths(basementDepthsActiveAtWellLocations, mudlineDepthsAtActiveWellLocations);
  EXPECT_EQ(manager.wells()[0]->calibrationTargets().size(), 2);
  EXPECT_EQ(manager.wells()[1]->calibrationTargets().size(), 4); // Well was inactive, so nothing happened
  EXPECT_EQ(manager.wells()[2]->calibrationTargets().size(), 0);
}

TEST(CalibrationTargetManagerTest, testRemoveWellsOutsideBasinModel)
{
  std::vector<double> basementDepthsActiveAtWellLocations = {6000, 5000};
  std::vector<double> mudlineDepthsAtActiveWellLocations = {0, 150};

  casaWizard::CalibrationTargetManager manager;
  manager.addWell("Well1", 179250, 603750);

  manager.addWell("Well2", 179000, 603500); // Invalid

  manager.addWell("Well3", 179000, 603500); // Invalid, but inactive
  manager.setWellIsActive(false, 2);

  manager.removeWellsOutsideBasinOutline("./TestWellValidator.project3d", "MAP-27105");

  EXPECT_EQ(manager.wells().size(), 2);
  EXPECT_EQ(manager.well(0).name().toStdString(), "Well1");
  EXPECT_EQ(manager.well(1).name().toStdString(), "Well3");
}

TEST(CalibrationTargetManagerTest, testConvertDTtoTWT)
{
  casaWizard::CalibrationTargetManager manager;
  manager.addWell("10_AML2_AV", 184550, 608300);
  manager.addCalibrationTarget("Test", "SonicSlowness", 0, 100, 100);
  manager.addCalibrationTarget("Test", "SonicSlowness", 0, 200, 200);
  manager.addWell("11_AMN1_AV", 192000, 615000);
  manager.addCalibrationTarget("Test", "SonicSlowness", 1, 100, 100);
  manager.addCalibrationTarget("Test", "SonicSlowness", 1, 200, 200);
  manager.setWellIsActive(false, 1);
  manager.addToMapping("SonicSlowness", "SonicSlowness");

  manager.convertDTtoTWT("./original1d/CaseSet/Iteration_1/", "Project.project3d");

  EXPECT_EQ(manager.well(0).calibrationTargets().size(), 4);
  EXPECT_EQ(manager.well(1).calibrationTargets().size(), 2);
}

TEST(CalibrationTargetManagerTest, testConvertDTfromVPtoTWT)
{
  casaWizard::CalibrationTargetManager manager;
  manager.addWell("10_AML2_AV", 184550, 608300);
  manager.addCalibrationTarget("Test", "SonicSlowness", 0, 100, 100);
  manager.addCalibrationTarget("Test", "SonicSlowness", 0, 200, 200);
  manager.addWell("11_AMN1_AV", 192000, 615000);
  manager.addCalibrationTarget("Test", "SonicSlowness", 1, 100, 100);
  manager.addCalibrationTarget("Test", "SonicSlowness", 1, 200, 200);
  manager.setWellIsActive(false, 1);
  manager.addToMapping("DT_FROM_VP", "SonicSlowness");

  manager.convertDTtoTWT("./original1d/CaseSet/Iteration_1/", "Project.project3d");

  EXPECT_EQ(manager.well(0).calibrationTargets().size(), 4);
  EXPECT_EQ(manager.well(1).calibrationTargets().size(), 2);
}

TEST(CalibrationTargetManagerTest, testConvertDTtoTWTWithoutDTData)
{
  std::vector<double> basementDepthsActiveAtWellLocations = {6000, 5000};
  std::vector<double> mudlineDepthsAtActiveWellLocations = {0, 150};

  casaWizard::CalibrationTargetManager manager;
  manager.addWell("10_AML2_AV", 184550, 608300);
  manager.addCalibrationTarget("Test", "TwoWayTime", 0, 100, 100);
  manager.addToMapping("TwoWayTime", "TwoWayTime");

  manager.convertDTtoTWT("./original1d/CaseSet/Iteration_1/", "Project.project3d");

  EXPECT_EQ(manager.well(0).calibrationTargets().size(), 1);
}

TEST(CalibrationTargetManagerTest, testConvertDTtoTWT_MultipleTimes)
{
  casaWizard::CalibrationTargetManager manager;
  manager.addWell("10_AML2_AV", 184550, 608300);
  manager.addCalibrationTarget("Test", "SonicSlowness", 0, 100, 100);
  manager.addCalibrationTarget("Test", "SonicSlowness", 0, 200, 200);

  manager.addWell("11_AMN1_AV", 192000, 615000);
  manager.addCalibrationTarget("Test", "SonicSlowness", 1, 100, 100);
  manager.addCalibrationTarget("Test", "SonicSlowness", 1, 200, 200);

  manager.setWellIsActive(false, 1);
  manager.addToMapping("SonicSlowness", "SonicSlowness");

  manager.convertDTtoTWT("./original1d/CaseSet/Iteration_1/", "Project.project3d");
  manager.convertDTtoTWT("./original1d/CaseSet/Iteration_1/", "Project.project3d");
  manager.convertDTtoTWT("./original1d/CaseSet/Iteration_1/", "Project.project3d");

  EXPECT_EQ(manager.well(0).calibrationTargets().size(), 4);
  EXPECT_EQ(manager.well(1).calibrationTargets().size(), 2);
}

TEST(CalibrationTargetManagerTest, testConvertVPtoDT)
{
  casaWizard::CalibrationTargetManager manager;
  manager.addWell("10_AML2_AV", 184550, 608300);
  manager.addCalibrationTarget("Test", "Velocity", 0, 100, 100);
  manager.addWell("11_AMN1_AV", 192000, 615000);
  manager.addCalibrationTarget("Test", "Velocity", 1, 100, 100);
  manager.setWellIsActive(false, 1);
  manager.addToMapping("Velocity", "Velocity");

  manager.convertVPtoDT();

  EXPECT_EQ(manager.well(0).calibrationTargets().size(), 2);
  EXPECT_EQ(manager.well(0).metaData().toStdString(), "Created DT_FROM_VP targets converted from Velocity.");
  EXPECT_EQ(manager.well(1).calibrationTargets().size(), 1);
}

TEST(CalibrationTargetManagerTest, testConvertVPToDTWithoutVPData)
{
  casaWizard::CalibrationTargetManager manager;
  manager.addWell("10_AML2_AV", 184550, 608300);
  manager.addCalibrationTarget("Test", "TwoWayTime", 0, 100, 100);
  manager.addToMapping("TwoWayTime", "TwoWayTime");

  manager.convertVPtoDT();

  EXPECT_EQ(manager.well(0).calibrationTargets().size(), 1);
}

TEST(CalibrationTargetManagerTest, testSmoothenData)
{
  // Given
  casaWizard::CalibrationTargetManager manager;
  manager.addWell("well_1", 184550, 608300);
  manager.addCalibrationTarget("target_1", "TwoWayTime", 0, 100, 100);
  manager.addCalibrationTarget("target_2", "TwoWayTime", 0, 150, 200);
  manager.addCalibrationTarget("target_3", "TwoWayTime", 0, 300, 130);
  manager.addCalibrationTarget("target_4", "TwoWayTime", 0, 700, 400);

  // When
  double radius = 200.0;
  QString property = "TwoWayTime";
  manager.smoothenData({property}, radius);

  // Then
  const QVector<const casaWizard::CalibrationTarget*> targets = manager.well(0).calibrationTargets();
  ASSERT_EQ(targets.size(), 4);
  EXPECT_DOUBLE_EQ(targets[0]->value(), 145.78962600948938);
  EXPECT_DOUBLE_EQ(targets[1]->value(), 147.13859153244337);
  EXPECT_DOUBLE_EQ(targets[2]->value(), 158.51092271014977);
  EXPECT_DOUBLE_EQ(targets[3]->value(), 361.99914465090529);
  QString expectedMetaData = "Gaussian smoothing with radius " + QString::number(radius) + " applied to " + property + " targets";
  EXPECT_EQ(expectedMetaData, manager.well(0).metaData());
}


TEST(CalibrationTargetManagerTest, testScaleData)
{
  // Given
  casaWizard::CalibrationTargetManager manager;
  manager.addWell("well_1", 184550, 608300);
  manager.addCalibrationTarget("target_1", "TwoWayTime", 0, 100, 100);
  manager.addCalibrationTarget("target_2", "TwoWayTime", 0, 150, 200);
  manager.addCalibrationTarget("target_3", "TwoWayTime", 0, 300, 130);
  manager.addCalibrationTarget("target_4", "TwoWayTime", 0, 700, 400);

  // When
  const double scalingFactor = 0.1;
  const QString property = "TwoWayTime";
  manager.scaleData({property}, scalingFactor);

  // Then
  const QVector<const casaWizard::CalibrationTarget*> targets = manager.well(0).calibrationTargets();
  ASSERT_EQ(targets.size(), 4);
  EXPECT_DOUBLE_EQ(targets[0]->value(), 10.0);
  EXPECT_DOUBLE_EQ(targets[1]->value(), 20.0);
  EXPECT_DOUBLE_EQ(targets[2]->value(), 13.0);
  EXPECT_DOUBLE_EQ(targets[3]->value(), 40.0);
  const QString expectedMetaData = "Scaling with scaling factor " + QString::number(scalingFactor) + " applied to " + property + " targets";
  EXPECT_EQ(expectedMetaData, manager.well(0).metaData());
}


TEST(CalibrationTargetManagerTest, testSubsampleData)
{
  // Given
  casaWizard::CalibrationTargetManager manager;
  manager.addWell("well_1", 184550, 608300);
  manager.addCalibrationTarget("target_1", "TwoWayTime", 0, 100, 100);
  manager.addCalibrationTarget("target_2", "TwoWayTime", 0, 150, 200);
  manager.addCalibrationTarget("target_3", "TwoWayTime", 0, 300, 130);
  manager.addCalibrationTarget("target_4", "TwoWayTime", 0, 700, 400);
  manager.addCalibrationTarget("target_5", "TwoWayTime", 0, 1100, 100);
  manager.addCalibrationTarget("target_6", "TwoWayTime", 0, 1150, 200);
  manager.addCalibrationTarget("target_7", "TwoWayTime", 0, 300, 130);
  manager.addCalibrationTarget("target_8", "TwoWayTime", 0, 1700, 400);

  // When
  const double length = 200.0;
  const QString property = "TwoWayTime";
  manager.subsampleData({property}, length);

  // Then
  const QVector<const casaWizard::CalibrationTarget*> targets = manager.well(0).calibrationTargets();
  EXPECT_EQ(targets.size(), 4);

  QString expectedMetaData = "Subsampling with length " + QString::number(length) + " applied to " + property + " targets";
  EXPECT_EQ(expectedMetaData, manager.well(0).metaData());
}

TEST(CalibrationTargetManagerTest, testApplyCutOff)
{
  // Given
  casaWizard::CalibrationTargetManager manager;
  manager.addWell("well_1", 184550, 608300);
  manager.addCalibrationTarget("target_1", "TwoWayTime", 0, 100, 100);
  manager.addCalibrationTarget("target_2", "TwoWayTime", 0, 150, 200);
  manager.addCalibrationTarget("target_3", "TwoWayTime", 0, 300, 130);
  manager.addCalibrationTarget("target_4", "TwoWayTime", 0, 700, 400);
  manager.addCalibrationTarget("target_5", "TwoWayTime", 0, 1100, 100);
  manager.addCalibrationTarget("target_6", "TwoWayTime", 0, 1150, 200);
  manager.addCalibrationTarget("target_7", "TwoWayTime", 0, 300, 130);
  manager.addCalibrationTarget("target_8", "TwoWayTime", 0, 1700, 400);

  QMap<QString,QPair<double,double>> propertiesWithCutOffRanges;
  const QString property = "TwoWayTime";
  propertiesWithCutOffRanges[property] = {110,300};

  // When
  manager.applyCutOff(propertiesWithCutOffRanges);

  // Then
  const QVector<const casaWizard::CalibrationTarget*> targets = manager.well(0).calibrationTargets();
  EXPECT_EQ(targets.size(), 4);
  const QString expectedMetaData = "Cut off " + QString::number(4) + " targets of property " + property + " outside the range [" +
        QString::number(propertiesWithCutOffRanges[property].first) + ", " +
        QString::number(propertiesWithCutOffRanges[property].second) + "]";

  EXPECT_EQ(expectedMetaData, manager.well(0).metaData());

  // When applying again, metadata should not be changed
  manager.applyCutOff(propertiesWithCutOffRanges);
  EXPECT_EQ(expectedMetaData, manager.well(0).metaData());
}

TEST(CalibrationTargetManagerTest, testRenamePropertiesAfterImport)
{
  casaWizard::CalibrationTargetManager manager;
  manager.addToMapping("TWT", "TwoWayTime");
  manager.addToMapping("SAC-TWT-from-DT", "TWT_FROM_DT");
  manager.addWell("well_1", 184550, 608300);
  manager.addCalibrationTarget("target_1", "TWT", 0, 100, 100);
  manager.addCalibrationTarget("target_2", "SAC-TWT-from-DT", 0, 100, 100);

  manager.renamePropertiesAfterImport();

  EXPECT_EQ(manager.getCauldronPropertyName("TWT_FROM_DT"), "TwoWayTime");
  EXPECT_EQ(manager.getCauldronPropertyName("TwoWayTime"), "TwoWayTime");

  EXPECT_EQ(manager.calibrationTargets()[0]->propertyUserName(), "TwoWayTime");
  EXPECT_EQ(manager.calibrationTargets()[1]->propertyUserName(), "TWT_FROM_DT");
}

TEST(CalibrationTargetManagerTest, test)
{
   casaWizard::CalibrationTargetManager manager;
   manager.addWell("Well_1", 0, 0);
   manager.addCalibrationTarget("test", "TwoWayTime", 0, 100, 200);
   manager.addCalibrationTarget("test", "VRe", 0, 100, 200);
   manager.addCalibrationTarget("test", "Temperature", 0, 100, 200);
   manager.addWell("Well_2", 0, 0);
   manager.addCalibrationTarget("test", "TwoWayTime", 1, 100, 200);
   manager.addWell("Well_3", 0, 0);
   manager.addCalibrationTarget("test", "TwoWayTime", 2, 100, 200);
   manager.addCalibrationTarget("test", "VRe", 2, 100, 200);

   QMap<QString, QSet<int>> propertyNamesPerWell = manager.getPropertyNamesPerWellForTargetTable();
   EXPECT_EQ(propertyNamesPerWell.size(), 3);
   EXPECT_EQ(propertyNamesPerWell["TwoWayTime"].size(), 3);
   EXPECT_EQ(propertyNamesPerWell["VRe"].size(), 2);
   EXPECT_EQ(propertyNamesPerWell["Temperature"].size(), 1);

   manager.setShowPropertiesInTable(false);
   propertyNamesPerWell = manager.getPropertyNamesPerWellForTargetTable();
   EXPECT_EQ(propertyNamesPerWell.size(), 0);
}
