#include "percentileGenerator.h"

#include <algorithm>

namespace casaWizard
{

namespace percentileGenerator
{

QVector<double> getPercentileValues(const QVector<double>& values, const QVector<double>& pTargets)
{
  QVector<double> pValues;
  QVector<double> valuesSorted{values};
  std::sort(valuesSorted.begin(), valuesSorted.end());
  const int s = valuesSorted.size();
  for (const double& p : pTargets)
  {
    const double target{s*p - 0.5};
    const int iLow{target};
    const int iHigh{iLow + 1};
    if (target<1.0/s) // For very small vectors
    {
      pValues.push_back(valuesSorted.front());
    }
    else if (iHigh == s) // For very small vectors
    {
      pValues.push_back(valuesSorted.back());
    }
    else
    {
      const double diff{target - iLow};
      pValues.push_back(valuesSorted[iLow]*(1.0 - diff) + valuesSorted[iHigh]*diff);
    }
  }
  return pValues;
}

} // namespace percentileGenerator

} // namespace casaWizard
