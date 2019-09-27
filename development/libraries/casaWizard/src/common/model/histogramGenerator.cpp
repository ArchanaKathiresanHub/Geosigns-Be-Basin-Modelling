#include "histogramGenerator.h"

#include <algorithm>

namespace casaWizard
{

namespace histogramGenerator
{

QVector<int> generateBins(const QVector<double>& values, int bins)
{
  QVector<double> valuesSorted{values};
  std::sort(valuesSorted.begin(), valuesSorted.end());
  const double minValue = valuesSorted.first();
  const double maxValue = valuesSorted.last();
  const double binSize = (maxValue - minValue)/bins;
  double edgeValue = minValue + binSize;

  QVector<int> dataBin(bins, 0);
  int bin = 1;
  for (const double& value : valuesSorted)
  {
    while (edgeValue <= value && bin < bins)
    {
      edgeValue += binSize;
      ++bin;
    }
    ++dataBin[bin - 1];
  }
  return dataBin;
}

} // namespace histogramGenerator

} // namespace casaWizard
