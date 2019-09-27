#include "model/calibrationTargetManager.h"
#include "model/functions/rmseCalibrationTargets.h"

#include <gtest/gtest.h>

namespace
{

const double epsilon = 1e-6;

} // namespace


class CalibrationTargetTest : public ::testing::Test
{
protected:
  void SetUp() final
  {
    const int wellID = ctManager_.addWell("testWell0", 0.0, 0.0);
    ctManager_.addCalibrationTarget("Target1", "Temperature", wellID, 0.0, 1.1);
    ctManager_.addCalibrationTarget("Target2", "Temperature", wellID, 1.0, 2.3);
    ctManager_.addCalibrationTarget("Target2", "Temperature", wellID, 2.0, 4.5);
  }

  casaWizard::CalibrationTargetManager ctManager_;
};

const QVector<double> targetValues = {1.2, 2.35, 4.555};

TEST_F(CalibrationTargetTest, testRmse)
{
  const double rmseValueExpected = 0.071937;
  const double rmseValueActual = casaWizard::functions::rmseCalibrationTargets(targetValues, ctManager_);

  EXPECT_NEAR(rmseValueExpected, rmseValueActual, epsilon);
}
