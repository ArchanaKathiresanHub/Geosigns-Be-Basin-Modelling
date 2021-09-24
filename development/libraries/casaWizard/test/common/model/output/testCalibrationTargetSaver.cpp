//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/output/calibrationTargetSaver.h"
#include "model/input/calibrationTargetCreator.h"

#include "stubCasaScenario.h"

#include <QString>

#include <gtest/gtest.h>

using namespace casaWizard;

TEST(CalibrationTargetSaverTest, testSave)
{
  const QString& excelFilename{"testWellData.xlsx"};
  casaWizard::StubCasaScenario scenarioWrite{};

  casaWizard::CalibrationTargetCreator targetCreator(scenarioWrite, scenarioWrite.calibrationTargetManager());
  targetCreator.createFromExcel(excelFilename);

  CalibrationTargetSaver saver(scenarioWrite.calibrationTargetManager());
  saver.saveToExcel("testWritingWells.xlsx");

  casaWizard::StubCasaScenario scenarioRead{};

  casaWizard::CalibrationTargetCreator targetCreator2(scenarioRead, scenarioRead.calibrationTargetManager());
  targetCreator2.createFromExcel(excelFilename);

  const QVector<const CalibrationTarget*> calibrationTargetsRead = scenarioRead.calibrationTargetManager().calibrationTargets();
  const QVector<const CalibrationTarget*> calibrationTargetsWrite = scenarioWrite.calibrationTargetManager().calibrationTargets();
  EXPECT_EQ(calibrationTargetsRead.size(), calibrationTargetsWrite.size());
  EXPECT_EQ(calibrationTargetsRead.size(), 8);

  for (int i = 0; i < calibrationTargetsRead.size(); i++)
  {
    EXPECT_EQ(calibrationTargetsRead[i]->name(), calibrationTargetsWrite[i]->name());
    EXPECT_DOUBLE_EQ(calibrationTargetsRead[i]->z(), calibrationTargetsWrite[i]->z());
    EXPECT_DOUBLE_EQ(calibrationTargetsRead[i]->value(), calibrationTargetsWrite[i]->value());
  }

  const QVector<const Well*> wellsRead = scenarioRead.calibrationTargetManager().wells();
  const QVector<const Well*> wellsWrite = scenarioWrite.calibrationTargetManager().wells();

  EXPECT_EQ(wellsRead.size(), wellsWrite.size());
  EXPECT_EQ(wellsRead.size(), 2);

  EXPECT_EQ(wellsRead[0]->metaData(), wellsWrite[0]->metaData());
  EXPECT_EQ(wellsRead[0]->metaData(), "SomeMetaData");
}
