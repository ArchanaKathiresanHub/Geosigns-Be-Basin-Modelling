#include "model/functions/interpolateVector.h"


#include <gtest/gtest.h>

TEST( InterpolateVectorTest, testOutputsize)
{
  QVector<double> x = {0, 1};
  QVector<double> y = {0, 1};
  QVector<double> xIn = {0.5};

  QVector<double> yOut = casaWizard::functions::interpolateVector(x, y, xIn);

  ASSERT_EQ(xIn.size(), yOut.size());
}

TEST( InterpolateVectorTest, testInterpolateOneValue)
{
  QVector<double> x = {0, 1};
  QVector<double> y = {0, 1};
  QVector<double> xIn = {0.5};

  QVector<double> yOut = casaWizard::functions::interpolateVector(x, y, xIn);

  ASSERT_EQ(xIn.size(), yOut.size());
  EXPECT_DOUBLE_EQ(0.5, yOut[0]);
}

TEST( InterpolateVectorTest, testInterpolateTwoValues)
{
  QVector<double> x = {0, 1};
  QVector<double> y = {1, 2};
  QVector<double> xIn = {0.4, 0.7};

  QVector<double> yOut = casaWizard::functions::interpolateVector(x, y, xIn);

  ASSERT_EQ(2, yOut.size());
  EXPECT_DOUBLE_EQ(1.4, yOut[0]);
  EXPECT_DOUBLE_EQ(1.7, yOut[1]);
}

TEST( InterpolateVectorTest, testInterpolateBeforeFirst)
{
  QVector<double> x = {2, 3};
  QVector<double> y = {4, 6};
  QVector<double> xIn = {1};

  QVector<double> yOut = casaWizard::functions::interpolateVector(x, y, xIn);

  ASSERT_EQ(1, yOut.size());
  EXPECT_DOUBLE_EQ(4, yOut[0]);
}

TEST( InterpolateVectorTest, testInterpolateAfterLast)
{
  QVector<double> x = {2, 3};
  QVector<double> y = {4, 6};
  QVector<double> xIn = {10};

  QVector<double> yOut = casaWizard::functions::interpolateVector(x, y, xIn);

  ASSERT_EQ(1, yOut.size());
  EXPECT_DOUBLE_EQ(6, yOut[0]);
}

TEST( InterpolateVectorTest, testLongerVector)
{
  QVector<double> x = {2, 3, 5, 8, 10};
  QVector<double> y = {5, 4, 0, 9, 15};
  QVector<double> xIn = {2.5, 4, 7, 9, 11};

  QVector<double> yOutExpected = {4.5, 2, 6, 12, 15};
  QVector<double> yOutActual = casaWizard::functions::interpolateVector(x, y, xIn);

  ASSERT_EQ(yOutExpected.size(), yOutActual.size());
  for (int i = 0; i<yOutExpected.size(); ++i)
  {
    EXPECT_DOUBLE_EQ(yOutExpected[i], yOutActual[i]) << " mismatch at index " << i;
  }
}

TEST( InterpolateVectorTest, testEmptyInputVector )
{
  QVector<double> x(0, 0);
  QVector<double> y(0, 0);
  QVector<double> xIn = {0, 1};

  QVector<double> yOutExpected = {0, 0};
  QVector<double> yOutActual = casaWizard::functions::interpolateVector(x, y, xIn);

  ASSERT_EQ(yOutExpected.size(), yOutActual.size());
  for (int i = 0; i<yOutExpected.size(); ++i)
  {
    EXPECT_DOUBLE_EQ(yOutExpected[i], yOutActual[i]) << " mismatch at index " << i;
  }
}

TEST( InterpolateVectorTest, testSize1Input )
{
  QVector<double> x = {0.5};
  QVector<double> y = {2};
  QVector<double> xIn = {0, 1};

  QVector<double> yOutExpected = {2, 2};
  QVector<double> yOutActual = casaWizard::functions::interpolateVector(x, y, xIn);

  ASSERT_EQ(yOutExpected.size(), yOutActual.size());
  for (int i = 0; i<yOutExpected.size(); ++i)
  {
    EXPECT_DOUBLE_EQ(yOutExpected[i], yOutActual[i]) << " mismatch at index " << i;
  }
}
