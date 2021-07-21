#include "model/input/calibrationTargetCreator.h"

#include "model/casaScenario.h"
#include "model/calibrationTarget.h"
#include "model/well.h"

#include "stubCasaScenario.h"

#include <gtest/gtest.h>

TEST( CalibrationTargetCreatorTest, testCreateFromExcel )
{
  const QString& excelFilename{"testWellData.xlsx"};
  casaWizard::StubCasaScenario scenario{};
  casaWizard::CalibrationTargetManager& manager{scenario.calibrationTargetManager()};

  casaWizard::calibrationTargetCreator::createFromExcel(scenario, excelFilename);

  const QVector<const casaWizard::Well*> wellsActual = manager.wells();
  const QVector<const casaWizard::CalibrationTarget*> targetsActual = manager.calibrationTargets();

  QVector<casaWizard::Well> wellsExpected{};
  wellsExpected.push_back(casaWizard::Well{0, "NW01S", 1234567.9876, 987654.12345, true});
  wellsExpected.push_back(casaWizard::Well{1, "well02", 123.566, 987.123, true});

  ASSERT_EQ(wellsActual.size(), wellsExpected.size());
  const int nWells = wellsActual.size();
  for (int iWell = 0; iWell<nWells; ++iWell)
  {
    EXPECT_EQ       (wellsActual[iWell]->id(),                 wellsExpected[iWell].id())                 << " Mismatch at index " << iWell;
    EXPECT_EQ       (wellsActual[iWell]->name().toStdString(), wellsExpected[iWell].name().toStdString()) << " Mismatch at index " << iWell;
    EXPECT_DOUBLE_EQ(wellsActual[iWell]->x(),                  wellsExpected[iWell].x())                  << " Mismatch at index " << iWell;
    EXPECT_DOUBLE_EQ(wellsActual[iWell]->y(),                  wellsExpected[iWell].y())                  << " Mismatch at index " << iWell;
    EXPECT_EQ       (wellsActual[iWell]->isActive(),           wellsExpected[iWell].isActive())           << " Mismatch at index " << iWell;
  }

  QVector<casaWizard::CalibrationTarget> targetsExpected{};
  targetsExpected.append(casaWizard::CalibrationTarget("TwoWayTime(1234568.0,987654.1,100.0)",  "Two way time", 100, 1000.3234, 0, 1.0/3.0));
  targetsExpected.append(casaWizard::CalibrationTarget("TwoWayTime(1234568.0,987654.1,200.0)",  "Two way time", 200, 5000.652, 0, 1.0/3.0));
  targetsExpected.append(casaWizard::CalibrationTarget("BulkDensity(1234568.0,987654.1,123.0)", "Density", 123, 2200, 0, 1.0/3.0));
  targetsExpected.append(casaWizard::CalibrationTarget("BulkDensity(1234568.0,987654.1,456.0)", "Density", 456, 2553.1278, 0, 1.0/3.0));

  targetsExpected.append(casaWizard::CalibrationTarget("Temperature(123.6,987.1,50.6)",   "Temperature", 50.6, 0, 0, 1.0/3.0));
  targetsExpected.append(casaWizard::CalibrationTarget("Temperature(123.6,987.1,223.4)",  "Temperature", 223.4, 49, 0, 1.0/3.0));
  targetsExpected.append(casaWizard::CalibrationTarget("Temperature(123.6,987.1,700.1)",  "Temperature", 700.1, 61.8, 0, 1.0/3.0));
  targetsExpected.append(casaWizard::CalibrationTarget("Temperature(123.6,987.1,1100.2)", "Temperature", 1100.2, 75.2, 0, 1.0/3.0));

  ASSERT_EQ(targetsExpected.size(), targetsActual.size());
  const int nTargets = targetsActual.size();
  for (int i  = 0; i<nTargets; ++i)
  {
    EXPECT_EQ(targetsExpected[i].name().toStdString(),     targetsActual[i]->name().toStdString())     << " Mismatch at index " << i;
    EXPECT_EQ(targetsExpected[i].propertyUserName().toStdString(), targetsActual[i]->propertyUserName().toStdString()) << " Mismatch at index " << i;

    EXPECT_DOUBLE_EQ(targetsExpected[i].z(),                 targetsActual[i]->z()) << " Mismatch at index " << i;
    EXPECT_DOUBLE_EQ(targetsExpected[i].value(),             targetsActual[i]->value()) << " Mismatch at index " << i;
    EXPECT_DOUBLE_EQ(targetsExpected[i].uaWeight(),          targetsActual[i]->uaWeight()) << " Mismatch at index " << i;
  }
}

TEST( CalibrationTargetCreator, testCreateFromExcelWithDeletes )
{
  const QString& excelFilename{"testWellDataWithDeletes.xlsx"};
  casaWizard::StubCasaScenario scenario{};
  casaWizard::CalibrationTargetManager& manager{scenario.calibrationTargetManager()};

  casaWizard::calibrationTargetCreator::createFromExcel(scenario, excelFilename);

  const QVector<const casaWizard::CalibrationTarget*> targetsActual = manager.calibrationTargets();
  const int nActualTargets{targetsActual.size()};
  const int nExpectedTargets{15};

  EXPECT_EQ(nActualTargets, nExpectedTargets);
}
