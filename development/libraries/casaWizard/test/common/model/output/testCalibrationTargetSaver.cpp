//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/output/calibrationTargetSaver.h"
#include "model/input/calibrationTargetCreator.h"
#include "model/input/extractWellDataXlsx.h"

#include "stubCasaScenario.h"

#include <QString>
#include <gtest/gtest.h>
#include <expectFileEq.h>

using namespace casaWizard;

TEST(CalibrationTargetSaverTest, testSave)
{
  const QString excelFilename{"testWellData.xlsx"};
  const QString excelFilename2{"testWritingWells.xlsx"};
  casaWizard::StubCasaScenario scenarioWrite{};

  casaWizard::ExtractWellDataXlsx extractor(excelFilename);
  casaWizard::CalibrationTargetCreator targetCreator(scenarioWrite, scenarioWrite.calibrationTargetManager(), extractor);
  targetCreator.readMetaDataFromFile();
  targetCreator.createFromFile();

  CalibrationTargetSaver saver(scenarioWrite.calibrationTargetManager());
  saver.saveToExcel(excelFilename2);

  casaWizard::StubCasaScenario scenarioRead{};

  casaWizard::ExtractWellDataXlsx extractor2(excelFilename2);
  casaWizard::CalibrationTargetCreator targetCreator2(scenarioRead, scenarioRead.calibrationTargetManager(), extractor2);
  targetCreator2.readMetaDataFromFile();
  targetCreator2.createFromFile();

  QVector<const CalibrationTarget*> calibrationTargetsRead = scenarioRead.calibrationTargetManager().calibrationTargets();
  QVector<const CalibrationTarget*> calibrationTargetsWrite = scenarioWrite.calibrationTargetManager().calibrationTargets();
  EXPECT_EQ(calibrationTargetsRead.size(), calibrationTargetsWrite.size());
  EXPECT_EQ(calibrationTargetsRead.size(), 8);

  std::sort(calibrationTargetsRead.begin(), calibrationTargetsRead.end(), [=](const CalibrationTarget* a, const CalibrationTarget* b){return a->name()>b->name();});
  std::sort(calibrationTargetsWrite.begin(), calibrationTargetsWrite.end(), [=](const CalibrationTarget* a, const CalibrationTarget* b){return a->name()>b->name();});

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

TEST(CalibrationTargetSaverTest, exportTest)
{
   casaWizard::StubCasaScenario scenario{};
   std::remove("testExportASCII.csv");

   scenario.calibrationTargetManager().addWell("name", 100, 100);
   scenario.calibrationTargetManager().addWell("name2", 200, 200);
   scenario.calibrationTargetManager().addWell("", 300.3, 2e13);

   CalibrationTargetSaver saver(scenario.calibrationTargetManager());

   saver.saveRawLocationsToCSV("testExportASCII.csv");
   expectFileEq("testExportASCII.csv", "testExpectedASCII.csv");
}
