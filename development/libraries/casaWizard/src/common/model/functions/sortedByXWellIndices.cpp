#include "sortedByXWellIndices.h"

#include "model/well.h"

#include <QVector>

#include <algorithm>
#include <numeric>

namespace casaWizard
{

namespace functions
{

QVector<int> sortedByXWellIndices(const QVector<const Well*>& wells)
{
  QVector<int> sortedIndex(wells.size(), 0);

  std::iota(sortedIndex.begin(), sortedIndex.end(), 0);

  std::sort(sortedIndex.begin(), sortedIndex.end(), [&wells](int i1, int i2){return wells[i1]->x() < wells[i2]->x();});

  return sortedIndex;
}

} // namespace functions

} // namespace casaWizard
