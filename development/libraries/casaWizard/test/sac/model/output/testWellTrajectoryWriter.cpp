#include "model/output/wellTrajectoryWriter.h"

#include "model/sacScenario.h"
#include "stubProjectReader.h"

#include "expectFileEq.h"

#include <gtest/gtest.h>
#include <fstream>

class WellTrajectoryWriterTest : public ::testing::Test
{
public:
  casaWizard::sac::SACScenario scenario{new casaWizard::StubProjectReader()};
  casaWizard::CalibrationTargetManager& ctManager{scenario.calibrationTargetManager()};
  const QString wellName1{"Well1"};
  const QString wellName2{"Well2"};
  const std::string folder{"./wells/"};
  const std::string file1Actual{folder + wellName1.toStdString() + "_Temperature.in"};
  const std::string file2Actual{folder + wellName1.toStdString() + "_TwoWayTime.in"};
  const std::string file3Actual{folder + wellName2.toStdString() + "_Temperature.in"};

  void SetUp() override
  {
    scenario.setWorkingDirectory(".");
    const int wellid1 = ctManager.addWell(wellName1, 10, 11);
    const int wellid2 = ctManager.addWell(wellName2, 20, 21);

    const QVector<double> depth1 = {10, 20, 30};
    const QVector<double> temperature1 = {100, 200, 300};
    const QVector<double> twoWayTime1 = {30, 60, 90};
    for (int i=0; i<3; ++i)
    {
      ctManager.addCalibrationTarget("","Temperature", wellid1, depth1[i], temperature1[i]);
      ctManager.addCalibrationTarget("","TwoWayTime",  wellid1, depth1[i], twoWayTime1[i]);
    }

    ctManager.addCalibrationTarget("", "Temperature", wellid2, 0, 273.15);
    ctManager.addCalibrationTarget("", "Temperature", wellid2, 1000, 473.15);
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

  const std::string file1Expected{folder + wellName1.toStdString() + "_TemperatureExpected.in"};
  const std::string file2Expected{folder + wellName1.toStdString() + "_TwoWayTimeExpected.in"};
  const std::string file3Expected{folder + wellName2.toStdString() + "_TemperatureExpected.in"};

  expectFileEq(file1Expected, file1Actual);
  expectFileEq(file2Expected, file2Actual);
  expectFileEq(file3Expected, file3Actual);
}
