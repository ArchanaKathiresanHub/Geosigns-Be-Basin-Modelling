// Function for calculating root mean square error (rmse) of calibration targets.
#pragma once

#include <QVector>

class CalibrationTargetManager;

namespace casaWizard
{

namespace functions
{

double rmseCalibrationTargets(const QVector<double> targetValues, const CalibrationTargetManager& calibrationTargetManager);

} // namespace functions

} // namespace casaWizard
