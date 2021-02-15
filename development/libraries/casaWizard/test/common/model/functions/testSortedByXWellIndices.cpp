#include "model/functions/sortedByXWellIndices.h"

#include "model/well.h"

#include <QVector>

#include <gtest/gtest.h>

TEST( SortedByXWellIndices, testSorting )
{
  casaWizard::Well well1{0, "well1", 0, 1, true};
  casaWizard::Well well2{0, "well2", 2, 2, true};
  casaWizard::Well well3{0, "well3", 1, 4, true};

  QVector<const casaWizard::Well*> wells;
  wells.append(&well1);
  wells.append(&well2);
  wells.append(&well3);

  QVector<int> orderExpected{0, 2, 1};
  QVector<int> orderActual{ casaWizard::functions::sortedByXWellIndices(wells) };

  ASSERT_EQ( orderExpected.size(), orderActual.size());
  for (int i = 0; i < orderExpected.size(); ++i)
  {
    EXPECT_EQ( orderExpected[i], orderActual[i])
        << "Mismatch at index [" << i << "]";
  }
}
