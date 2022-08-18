#include "rmseCalibrationTargets.h"

#include <model/calibrationTargetManager.h>
#include "model/logger.h"

#include "ConstantsNumerical.h"

#include <cmath>
#include <numeric>

namespace casaWizard
{

namespace functions
{

bool isNotUndefined(double value)
{
   return std::abs(value - Utilities::Numerical::IbsNoDataValue) > Utilities::Numerical::DefaultNumericalTolerance;
}

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
   int numConsideredTargets = 0;
   for (const CalibrationTarget* activeTarget : calibrationTargetManager.activeCalibrationTargets())
   {
      double targetValue = targetValues[i];
      if (isNotUndefined(targetValue))
      {
         const double v = (activeTarget->value() - targetValue) / (activeTarget->standardDeviation() + std::numeric_limits<double>::epsilon());
         rmse += v * v;
         ++numConsideredTargets;
      }
      ++i;
   }
   return std::sqrt(rmse / numConsideredTargets);
}

} // namespace functions

} // namespace casaWizard
