#include "model/percentileGenerator.h"

#include <gtest/gtest.h>


TEST(PercentileGeneratorTest, testGetPercentileValues)
{
  QVector<double> values{9,8,7,6,5,4,3,2,1,0};
  QVector<double> pTargets{0.1,0.5,0.9};

  QVector<double> pValuesActual{casaWizard::percentileGenerator::getPercentileValues(values, pTargets)};
  QVector<double> pValuesExpected{0.5,4.5,8.5};

  for ( int i = 0; i< pValuesActual.size(); ++i)
  {
    EXPECT_EQ(pValuesExpected[i], pValuesActual[i]);
  }
}

TEST(PercentileGeneratorTest, testGetPercentileValuesSmallVector)
{
  QVector<double> values{9,3,2,1,0};
  QVector<double> pTargets{0.1,0.5,0.9};

  QVector<double> pValuesActual{casaWizard::percentileGenerator::getPercentileValues(values, pTargets)};
  QVector<double> pValuesExpected{0.0,2.0,9.0};

  for ( int i = 0; i< pValuesActual.size(); ++i)
  {
    EXPECT_EQ(pValuesExpected[i], pValuesActual[i]);
  }
}
