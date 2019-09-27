#include "interpolateVector.h"

#include <assert.h>

namespace casaWizard
{

namespace functions
{

QVector<double> interpolateVector(const QVector<double> x,
                                  const QVector<double> y,
                                  const QVector<double> xIn)
{
  assert(x.size() == y.size());
  for (int i = 1; i < x.size(); ++i)
  {
    assert(x[i] >= x[i-1]);
  }

  const int n = xIn.size();
  const int m = x.size();
  QVector<double> yOut(xIn.size(), 0);
  if (x.size() == 0)
  {
    return yOut;
  }

  for (int i = 0; i < n; i++)
  {
    int j = 0;
    while (j < m && x[j] < xIn[i])
    {
      j++;
    }

    if (j == 0)
    {
      yOut[i] = y[0];
      continue;
    }

    if (j == m)
    {
      yOut[i] = y[m-1];
      continue;
    }

    yOut[i] = y[j-1] + (y[j]-y[j-1]) * (xIn[i]- x[j-1])
                                      /(x[j]  - x[j-1]);
  }

  return yOut;
}

}  // namespace functions

}  // namespace casaWizard
