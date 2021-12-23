#include "decompositionCalculator.h"

#include <gtest/gtest.h>

using namespace DataAccess::Interface;
class DecompositionCalculatorTest : public ::testing::Test
{

public:
  void testInternalConsistency(const std::vector<std::vector<int>>& domainShape)
  {
    if (domainShape[0].size() < domainShape.size())
    {
      ASSERT_EQ(m_, numberOfCores_);
      ASSERT_EQ(n_, 1);
    }
    else
    {
      ASSERT_EQ(m_, 1);
      ASSERT_EQ(n_, numberOfCores_);
    }

    ASSERT_EQ(subdomainWidths_.size(), m_);
    ASSERT_EQ(subdomainHeights_.size(), n_);

    int sumOfDomainWidths = 0;
    for (int subdomainWidth : subdomainWidths_)
    {
      sumOfDomainWidths += subdomainWidth;
    }
    ASSERT_EQ(sumOfDomainWidths, domainShape.size());

    int sumOfDomainHeights = 0;
    for (int subdomainHeight : subdomainHeights_)
    {
      sumOfDomainHeights += subdomainHeight;
    }

    ASSERT_EQ(sumOfDomainHeights, domainShape[0].size());
  }

  void SetUp() override
  {
    m_ = 0;
    n_ = 0;
    subdomainWidths_.clear();
    subdomainHeights_.clear();
    numberOfCores_ = 0;
  }

  int m_;
  int n_;
  std::vector<int> subdomainWidths_;
  std::vector<int> subdomainHeights_;
  int numberOfCores_;
};

TEST_F(DecompositionCalculatorTest, testDecompose)
{
  // Given
  std::vector<std::vector<int>> domainShape = {{1, 1, 1},
                                               {1, 1, 1},
                                               {1, 1, 1},
                                               {1, 1, 1}};
  numberOfCores_ = 4;

  // When
  DecompositionCalculator calculator(domainShape, numberOfCores_);
  calculator.calculateDomain(m_, n_, subdomainWidths_, subdomainHeights_);
  testInternalConsistency(domainShape);

  for (int subdomainWidth : subdomainWidths_)
  {
    EXPECT_EQ(subdomainWidth, 1);
  }

  EXPECT_EQ(subdomainHeights_[0], 3);
}

TEST_F(DecompositionCalculatorTest, testDecomposeTransposed)
{
  // Given
  std::vector<std::vector<int>> domainShape = {{1, 1, 1, 1},
                                               {1, 1, 1, 1},
                                               {1, 1, 1, 1}};
  numberOfCores_ = 4;

  // When
  DecompositionCalculator calculator(domainShape, numberOfCores_);
  calculator.calculateDomain(m_, n_, subdomainWidths_, subdomainHeights_);
  testInternalConsistency(domainShape);

  for (int subdomainHeight : subdomainHeights_)
  {
    EXPECT_EQ(subdomainHeight, 1);
  }

  EXPECT_EQ(subdomainWidths_[0], 3);
}

TEST_F(DecompositionCalculatorTest, testDecompose2)
{
  std::vector<std::vector<int>> domainShape = {{1, 1, 1},
                                               {1, 1, 1},
                                               {1, 1, 1},
                                               {1, 1, 1}};
  numberOfCores_ = 2;

  DecompositionCalculator calculator(domainShape, numberOfCores_);
  calculator.calculateDomain(m_, n_, subdomainWidths_, subdomainHeights_);


  testInternalConsistency(domainShape);
  for (int subdomainWidth : subdomainWidths_)
  {
    EXPECT_EQ(subdomainWidth, 2);
  }
  EXPECT_EQ(subdomainHeights_[0], 3);
}

TEST_F(DecompositionCalculatorTest, testDecomposeIntoUnevenParts)
{
  std::vector<std::vector<int>> domainShape = {{1, 1, 1},
                                               {1, 1, 1},
                                               {1, 1, 1},
                                               {1, 1, 1}};
  numberOfCores_ = 3;
  DecompositionCalculator calculator(domainShape, numberOfCores_);
  calculator.calculateDomain(m_, n_, subdomainWidths_, subdomainHeights_);

  testInternalConsistency(domainShape);

  EXPECT_EQ(subdomainWidths_[0], 1);
  EXPECT_EQ(subdomainWidths_[1], 1);
  EXPECT_EQ(subdomainWidths_[2], 2);

  EXPECT_EQ(subdomainHeights_[0], 3);
}

TEST_F(DecompositionCalculatorTest, testDecomposeIntoUnevenParts2)
{
  std::vector<std::vector<int>> domainShape = {{1, 1, 1},
                                               {1, 1, 1},
                                               {1, 1, 1},
                                               {1, 1, 1},
                                               {1, 1, 1},
                                               {1, 1, 1}};
  numberOfCores_ = 4;

  DecompositionCalculator calculator(domainShape, numberOfCores_);
  calculator.calculateDomain(m_, n_, subdomainWidths_, subdomainHeights_);

  testInternalConsistency(domainShape);

  EXPECT_EQ(subdomainWidths_[0], 1);
  EXPECT_EQ(subdomainWidths_[1], 2);
  EXPECT_EQ(subdomainWidths_[2], 1);
  EXPECT_EQ(subdomainWidths_[3], 2);

  EXPECT_EQ(subdomainHeights_[0], 3);
}

TEST_F(DecompositionCalculatorTest, testDecomposeIntoUnevenParts2Rotated)
{
  std::vector<std::vector<int>> domainShape = {{1, 1, 1, 1, 1, 1},
                                               {1, 1, 1, 1, 1, 1},
                                               {1, 1, 1, 1, 1, 1}};
  numberOfCores_ = 4;

  DecompositionCalculator calculator(domainShape, numberOfCores_);
  calculator.calculateDomain(m_, n_, subdomainWidths_, subdomainHeights_);

  testInternalConsistency(domainShape);

  EXPECT_EQ(subdomainHeights_[0], 1);
  EXPECT_EQ(subdomainHeights_[1], 2);
  EXPECT_EQ(subdomainHeights_[2], 1);
  EXPECT_EQ(subdomainHeights_[3], 2);

  EXPECT_EQ(subdomainWidths_[0], 3);
}

TEST_F(DecompositionCalculatorTest, testDecomposeWithInvalidPoints)
{
  std::vector<std::vector<int>> domainShape = {{1, 1, 1},
                                               {1, 0, 1},
                                               {1, 1, 0},
                                               {0, 1, 1}};
  numberOfCores_ = 4;
  DecompositionCalculator calculator(domainShape, numberOfCores_);
  calculator.calculateDomain(m_, n_, subdomainWidths_, subdomainHeights_);

  testInternalConsistency(domainShape);

  EXPECT_EQ(subdomainWidths_[0], 1);
  EXPECT_EQ(subdomainWidths_[1], 1);
  EXPECT_EQ(subdomainWidths_[2], 1);
  EXPECT_EQ(subdomainWidths_[3], 1);

  EXPECT_EQ(subdomainHeights_[0], 3);
}

TEST_F(DecompositionCalculatorTest, testDecomposeWithInvalidPoints2)
{
  std::vector<std::vector<int>> domainShape = {{1, 1, 1, 1},
                                               {1, 0, 0, 0},
                                               {1, 0, 0, 0},
                                               {1, 0, 0, 0},
                                               {1, 0, 0, 0}};
  numberOfCores_ = 4;
  DecompositionCalculator calculator(domainShape, numberOfCores_);
  calculator.calculateDomain(m_, n_, subdomainWidths_, subdomainHeights_);

  testInternalConsistency(domainShape);

  EXPECT_EQ(subdomainWidths_[0], 1);
  EXPECT_EQ(subdomainWidths_[1], 1);
  EXPECT_EQ(subdomainWidths_[2], 1);
  EXPECT_EQ(subdomainWidths_[3], 2);

  EXPECT_EQ(subdomainHeights_[0], 4);
}

TEST_F(DecompositionCalculatorTest, testDecomposeWithInvalidPoints3)
{
  std::vector<std::vector<int>> domainShape = {{1, 0, 0, 0},
                                               {1, 0, 0, 0},
                                               {1, 0, 0, 0},
                                               {1, 0, 0, 0},
                                               {1, 1, 1, 1}};
  numberOfCores_ = 4;
  DecompositionCalculator calculator(domainShape, numberOfCores_);
  calculator.calculateDomain(m_, n_, subdomainWidths_, subdomainHeights_);

  testInternalConsistency(domainShape);

  EXPECT_EQ(subdomainWidths_[0], 2);
  EXPECT_EQ(subdomainWidths_[1], 1);
  EXPECT_EQ(subdomainWidths_[2], 1);
  EXPECT_EQ(subdomainWidths_[3], 1);

  EXPECT_EQ(subdomainHeights_[0], 4);
}

TEST_F(DecompositionCalculatorTest, testDecomposeWithInvalidPointsWithoutRestriction)
{
  std::vector<std::vector<int>> domainShape = {{1, 1, 1, 1, 1, 1, 1, 1},
                                               {1, 0, 0, 0, 0, 0, 0, 0},
                                               {1, 0, 0, 0, 0, 0, 0, 0},
                                               {1, 0, 0, 0, 0, 0, 0, 0},
                                               {1, 0, 0, 0, 0, 0, 0, 0},
                                               {1, 0, 0, 0, 0, 0, 0, 0},
                                               {1, 0, 0, 0, 0, 0, 0, 0},
                                               {1, 0, 0, 0, 0, 0, 0, 0},
                                               {1, 0, 0, 0, 0, 0, 0, 0},
                                               {1, 1, 1, 1, 1, 1, 1, 1}};
  numberOfCores_ = 3;
  DecompositionCalculator calculator(domainShape, numberOfCores_);
  calculator.calculateDomain(m_, n_, subdomainWidths_, subdomainHeights_);

  testInternalConsistency(domainShape);

  EXPECT_EQ(subdomainWidths_[0], 1);
  EXPECT_EQ(subdomainWidths_[1], 8);
  EXPECT_EQ(subdomainWidths_[2], 1);

  EXPECT_EQ(subdomainHeights_[0], 8);
}

TEST_F(DecompositionCalculatorTest, testDecomposeWithInvalidPointsWithRestriction)
{
  std::vector<std::vector<int>> domainShape = {{1, 1, 1, 1, 1, 1, 1, 1},
                                               {1, 0, 0, 0, 0, 0, 0, 0},
                                               {1, 0, 0, 0, 0, 0, 0, 0},
                                               {1, 0, 0, 0, 0, 0, 0, 0},
                                               {1, 0, 0, 0, 0, 0, 0, 0},
                                               {1, 0, 0, 0, 0, 0, 0, 0},
                                               {1, 0, 0, 0, 0, 0, 0, 0},
                                               {1, 0, 0, 0, 0, 0, 0, 0},
                                               {1, 0, 0, 0, 0, 0, 0, 0},
                                               {1, 1, 1, 1, 1, 1, 1, 1}};
  numberOfCores_ = 3;
  double maxPercentageDeviationFromAverageWidth = 30;
  DecompositionCalculator calculator(domainShape, numberOfCores_, maxPercentageDeviationFromAverageWidth);
  calculator.calculateDomain(m_, n_, subdomainWidths_, subdomainHeights_);

  testInternalConsistency(domainShape);

  EXPECT_EQ(subdomainWidths_[0], 2);
  EXPECT_EQ(subdomainWidths_[1], 5);
  EXPECT_EQ(subdomainWidths_[2], 3);

  EXPECT_EQ(subdomainHeights_[0], 8);
}

TEST_F(DecompositionCalculatorTest, testDecomposeWithInvalidPointsWithRestriction2)
{
  std::vector<std::vector<int>> domainShape = {{1, 1, 1, 1, 1, 1, 1, 1},
                                               {1, 0, 0, 0, 0, 0, 0, 0},
                                               {1, 0, 0, 0, 0, 0, 0, 0},
                                               {1, 0, 0, 0, 0, 0, 0, 0},
                                               {1, 0, 0, 0, 0, 0, 0, 0},
                                               {1, 0, 0, 0, 0, 0, 0, 0},
                                               {1, 0, 0, 0, 0, 0, 0, 0},
                                               {1, 0, 0, 0, 0, 0, 0, 0},
                                               {1, 0, 0, 0, 0, 0, 0, 0},
                                               {1, 1, 1, 1, 1, 1, 1, 1}};
  numberOfCores_ = 3;
  double maxPercentageDeviationFromAverageWidth = 0;
  DecompositionCalculator calculator(domainShape, numberOfCores_, maxPercentageDeviationFromAverageWidth);
  calculator.calculateDomain(m_, n_, subdomainWidths_, subdomainHeights_);

  testInternalConsistency(domainShape);

  EXPECT_EQ(subdomainWidths_[0], 3);
  EXPECT_EQ(subdomainWidths_[1], 4);
  EXPECT_EQ(subdomainWidths_[2], 3);

  EXPECT_EQ(subdomainHeights_[0], 8);
}

TEST_F(DecompositionCalculatorTest, testDecomposeLargerModel)
{
  std::vector<std::vector<int>> domainShape;

  for (int i = 0; i < 185; i++)
  {
    std::vector<int> domainrow;
    for (int j = 0; j < 125; j++)
    {
      domainrow.push_back(1);
    }
    domainShape.push_back(domainrow);
  }

  numberOfCores_ = 7;
  DecompositionCalculator calculator(domainShape, numberOfCores_);
  calculator.calculateDomain(m_, n_, subdomainWidths_, subdomainHeights_);

  testInternalConsistency(domainShape);
}











