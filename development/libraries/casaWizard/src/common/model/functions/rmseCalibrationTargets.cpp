#include "rmseCalibrationTargets.h"

#include <model/calibrationTargetManager.h>
#include "model/logger.h"

#include <cmath>
#include <numeric>

namespace casaWizard
{

namespace functions
{

double rmseCalibrationTargets(const QVector<double> targetValues, const CalibrationTargetManager& calibrationTargetManager)
{
  if (targetValues.isEmpty())
  {
    return 0.0;
  }

  const int amountOfActiveCalibTargets = calibrationTargetManager.amountOfActiveCalibrationTargets();

  if (targetValues.size() != amountOfActiveCalibTargets)
  {
    Logger::log() << "Calculation of RMSE failed due to incompatible size of active calibration targets (" << amountOfActiveCalibTargets
                  << ") with obtained data (" << targetValues.size() << ")." << Logger::endl();
    return 0.0;
  }

  double rmse = 0.0;
  int i = 0;
  for (const CalibrationTarget* activeTarget : calibrationTargetManager.activeCalibrationTargets())
  {
    const double v = (activeTarget->value() - targetValues[i]) / (activeTarget->standardDeviation() + std::numeric_limits<double>::epsilon());
    rmse += v * v;
    ++i;
  }
  return std::sqrt(rmse / i);
}

} // namespace functions

} // namespace casaWizard
