//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "expectFileEq.h"
#include "model/output/wellTrajectoryWriter.h"
#include "model/sacScenario.h"
#include "stubProjectReader.h"

#include <gtest/gtest.h>

class WellTrajectoryWriterTest : public ::testing::Test
{
public:
  casaWizard::sac::SACScenario scenario{new casaWizard::StubProjectReader()};
  casaWizard::CalibrationTargetManager& ctManager{scenario.calibrationTargetManager()};
  const std::string wellName1{"Well1"};
  const std::string wellName2{"Well2"};
  const std::string folder{"./wells/"};
  const std::string file1Actual{folder + wellName1 + "_Temp_Temperature.in"};
  const std::string file2Actual{folder + wellName1 + "_TWTT_TwoWayTime.in"};
  const std::string file3Actual{folder + wellName2 + "_Temp_Temperature.in"};

  void SetUp() override
  {
    scenario.setWorkingDirectory(".");
    scenario.calibrationTargetManager().addToMapping("Temp", "Temperature");
    scenario.calibrationTargetManager().addToMapping("TWTT", "TwoWayTime");

    const int wellid1 = ctManager.addWell(QString::fromStdString(wellName1), 10, 11);
    const int wellid2 = ctManager.addWell(QString::fromStdString(wellName2), 20, 21);

    const QVector<double> depth1 = {10, 20, 30};
    const QVector<double> temperature1 = {100, 200, 300};
    const QVector<double> twoWayTime1 = {30, 60, 90};
    for (int i=0; i<3; ++i)
    {
      ctManager.addCalibrationTarget("", "Temp", wellid1, depth1[i], temperature1[i], 1);
      ctManager.addCalibrationTarget("", "TWTT", wellid1, depth1[i], twoWayTime1[i], 1);
    }

    ctManager.addCalibrationTarget("", "Temp", wellid2, 0, 273.15, 1);
    ctManager.addCalibrationTarget("", "Temp", wellid2, 1000, 473.15, 1);
  }
  void TearDown() override
  {
    std::remove(file1Actual.c_str());
    std::remove(file2Actual.c_str());
    std::remove(file3Actual.c_str());
  }
};

TEST_F( WellTrajectoryWriterTest, testWriteMultipleWells )
{
  casaWizard::sac::wellTrajectoryWriter::writeTrajectories(scenario);

  const std::string file1Expected{folder + wellName1 + "_TemperatureExpected.in"};
  const std::string file2Expected{folder + wellName1 + "_TwoWayTimeExpected.in"};
  const std::string file3Expected{folder + wellName2 + "_TemperatureExpected.in"};

  expectFileEq(file1Expected, file1Actual);
  expectFileEq(file2Expected, file2Actual);
  expectFileEq(file3Expected, file3Actual);
}
