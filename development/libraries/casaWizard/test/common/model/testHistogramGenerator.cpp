#include "model/histogramGenerator.h"

#include <gtest/gtest.h>

TEST(HistogramGeneratorTest, testGenerateBinsAllZeros)
{
  int bins = 10;
  QVector<double> values(10, 0.0);
  QVector<int> dataBins{casaWizard::histogramGenerator::generateBins(values, bins)};
  EXPECT_EQ(dataBins.size(), bins);
  EXPECT_EQ(dataBins.first(), 0);
  EXPECT_EQ(dataBins.last(), bins); // All zeros are in the last bin
}

TEST(HistogramGeneratorTest, testGenerateBinsSorted)
{
  int bins = 10;
  QVector<double> values{9,8,7,6,5,4,3,2,1,0};
  QVector<int> dataBins{casaWizard::histogramGenerator::generateBins(values, bins)};
  for (const int& value : dataBins)
  {
    EXPECT_EQ(value, 1);
  }
}

TEST(HistogramGeneratorTest, testGenerateBinsUnsorted)
{
  int bins = 10;
  QVector<double> values{9,8,7,6,5,4,3,2,1,0,-1,-2,-3,-4,-5,10,11,12,13,14};
  QVector<int> dataBins{casaWizard::histogramGenerator::generateBins(values, bins)};
  for (const int& value : dataBins)
  {
    EXPECT_EQ(value, 2);
  }
}

TEST(HistogramGeneratorTest, testGenerateBins5Bins)
{
  int bins = 5;
  QVector<double> values{9,8,7,6,5,4,3,2,1,0,-1,-2,-3,-4,-5,10,11,12,13,14};
  QVector<int> dataBins{casaWizard::histogramGenerator::generateBins(values, bins)};
  for (const int& value : dataBins)
  {
    EXPECT_EQ(value, 4);
  }
}
