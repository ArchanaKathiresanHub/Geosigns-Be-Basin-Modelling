#include "decompositionCalculator.h"

#include <gtest/gtest.h>

using namespace DataAccess::Interface;
class DecompositionCalculatorTest : public ::testing::Test
{

public:
  void testInternalConsistencyNewMethod(const std::vector<std::vector<int>>& domainShape)
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

  void TearDown() override
  {
  }

  bool calculateDomain(const std::vector<std::vector<int>>& domainShape, const double maxDeviationFromAverage = 1e10)
  {
    DecompositionCalculator calculator(domainShape, numberOfCores_, domainShape.size(), domainShape[0].size(), maxDeviationFromAverage);
    return calculator.calculateDecomposition(m_, n_, subdomainWidths_, subdomainHeights_);
  }

  bool calculateDomainLowRes(const std::vector<std::vector<int>>& domainShape, const int lowResNumI, const int lowResNumJ, const double maxDeviationFromAverage = 1e10)
  {
    DecompositionCalculator calculator(domainShape, numberOfCores_, domainShape.size(), domainShape[0].size(), maxDeviationFromAverage, lowResNumI, lowResNumJ);
    return calculator.calculateDecomposition(m_, n_, subdomainWidths_, subdomainHeights_);
  }

  std::vector<std::vector<int>> getLargeCShapedModel()
  {
    return {{1, 1, 1, 1, 1, 1, 1, 1},
            {1, 0, 0, 0, 0, 0, 0, 0},
            {1, 0, 0, 0, 0, 0, 0, 0},
            {1, 0, 0, 0, 0, 0, 0, 0},
            {1, 0, 0, 0, 0, 0, 0, 0},
            {1, 0, 0, 0, 0, 0, 0, 0},
            {1, 0, 0, 0, 0, 0, 0, 0},
            {1, 0, 0, 0, 0, 0, 0, 0},
            {1, 0, 0, 0, 0, 0, 0, 0},
            {1, 1, 1, 1, 1, 1, 1, 1}};
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
  numberOfCores_ = 2;

  // When
  calculateDomain(domainShape);

  // Then
  EXPECT_TRUE(subdomainWidths_.empty());
  EXPECT_TRUE(subdomainHeights_.empty());
  EXPECT_EQ(m_, 2);
  EXPECT_EQ(n_, 1);
}

TEST_F(DecompositionCalculatorTest, testDecomposeRotated)
{
  // Given
  std::vector<std::vector<int>> domainShape = {{1, 1, 1, 1},
                                               {1, 1, 1, 1},
                                               {1, 1, 1, 1}};
  numberOfCores_ = 2;

  // When
  calculateDomain(domainShape);

  // Then
  EXPECT_TRUE(subdomainWidths_.empty());
  EXPECT_TRUE(subdomainHeights_.empty());
  EXPECT_EQ(m_, 1);
  EXPECT_EQ(n_, 2);
}

TEST_F(DecompositionCalculatorTest, testDecomposeIntoUnevenParts2)
{
  std::vector<std::vector<int>> domainShape = {{1, 1, 1},
                                               {1, 1, 1},
                                               {1, 1, 1},
                                               {1, 1, 1},
                                               {1, 1, 1},
                                               {1, 1, 1},
                                               {1, 1, 1}};
  numberOfCores_ = 3;

  calculateDomain(domainShape);

  EXPECT_TRUE(subdomainWidths_.empty());
  EXPECT_TRUE(subdomainHeights_.empty());
  EXPECT_EQ(m_, 3);
  EXPECT_EQ(n_, 1);
}

TEST_F(DecompositionCalculatorTest, testDecomposeIntoUnevenParts2Rotated)
{
  std::vector<std::vector<int>> domainShape = {{1, 1, 1, 1, 1, 1, 1},
                                               {1, 1, 1, 1, 1, 1, 1},
                                               {1, 1, 1, 1, 1, 1, 1}};
  numberOfCores_ = 3;

  calculateDomain(domainShape);

  // Then
  EXPECT_TRUE(subdomainWidths_.empty());
  EXPECT_TRUE(subdomainHeights_.empty());
  EXPECT_EQ(m_, 1);
  EXPECT_EQ(n_, 3);
}

TEST_F(DecompositionCalculatorTest, testDecomposeWithInvalidPoints)
{
  std::vector<std::vector<int>> domainShape = {{1, 1, 1},
                                               {1, 0, 1},
                                               {1, 1, 0},
                                               {0, 1, 1}};
  numberOfCores_ = 2;
  calculateDomain(domainShape);

  testInternalConsistencyNewMethod(domainShape);

  EXPECT_EQ(subdomainWidths_[0], 2);
  EXPECT_EQ(subdomainWidths_[1], 2);

  EXPECT_EQ(subdomainHeights_[0], 3);
}

TEST_F(DecompositionCalculatorTest, testDecomposeWithInvalidPoints2)
{
  std::vector<std::vector<int>> domainShape = {{1, 1, 1, 1},
                                               {1, 0, 0, 0},
                                               {1, 0, 0, 0},
                                               {1, 0, 0, 0},
                                               {1, 0, 0, 0}};
  numberOfCores_ = 2;
  calculateDomain(domainShape);

  testInternalConsistencyNewMethod(domainShape);

  EXPECT_EQ(subdomainWidths_[0], 2);
  EXPECT_EQ(subdomainWidths_[1], 3);

  EXPECT_EQ(subdomainHeights_[0], 4);
}

TEST_F(DecompositionCalculatorTest, testDecomposeWithInvalidPoints3)
{
  std::vector<std::vector<int>> domainShape = {{1, 0, 0, 0},
                                               {1, 0, 0, 0},
                                               {1, 0, 0, 0},
                                               {1, 0, 0, 0},
                                               {1, 1, 1, 1}};
  numberOfCores_ = 2;
  calculateDomain(domainShape);

  testInternalConsistencyNewMethod(domainShape);

  EXPECT_EQ(subdomainWidths_[0], 3);
  EXPECT_EQ(subdomainWidths_[1], 2);

  EXPECT_EQ(subdomainHeights_[0], 4);
}

TEST_F(DecompositionCalculatorTest, testDecomposeWithInvalidPointsWithoutRestriction)
{
  std::vector<std::vector<int>> domainShape = getLargeCShapedModel();

  numberOfCores_ = 3;
  calculateDomain(domainShape);

  testInternalConsistencyNewMethod(domainShape);

  EXPECT_EQ(subdomainWidths_[0], 2);
  EXPECT_EQ(subdomainWidths_[1], 6);
  EXPECT_EQ(subdomainWidths_[2], 2);

  EXPECT_EQ(subdomainHeights_[0], 8);
}

TEST_F(DecompositionCalculatorTest, testDecomposeWithInvalidPointsWithRestriction)
{
  std::vector<std::vector<int>> domainShape = getLargeCShapedModel();

  numberOfCores_ = 3;

  calculateDomain(domainShape, 30);

  testInternalConsistencyNewMethod(domainShape);

  EXPECT_EQ(subdomainWidths_[0], 2);
  EXPECT_EQ(subdomainWidths_[1], 5);
  EXPECT_EQ(subdomainWidths_[2], 3);

  EXPECT_EQ(subdomainHeights_[0], 8);
}

TEST_F(DecompositionCalculatorTest, testDecomposeWithInvalidPointsWithRestriction2)
{
  std::vector<std::vector<int>> domainShape = getLargeCShapedModel();

  numberOfCores_ = 3;
  calculateDomain(domainShape, 0);

  testInternalConsistencyNewMethod(domainShape);

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
      if (j < 80)
      {
        domainrow.push_back(1);
      }
      else
      {
        domainrow.push_back(0);
      }
    }
    domainShape.push_back(domainrow);
  }

  numberOfCores_ = 7;
  calculateDomain(domainShape);

  testInternalConsistencyNewMethod(domainShape);
}

TEST_F(DecompositionCalculatorTest, testDecomposeWithTooManyCoresForDynamicMethodHighRes)
{
  std::vector<std::vector<int>> domainShape = {{1, 1, 1, 1},
                                               {1, 0, 0, 0},
                                               {1, 0, 0, 0},
                                               {1, 0, 0, 0},
                                               {1, 0, 0, 0}};

  numberOfCores_ = 4;

  calculateDomain(domainShape);

  // Then it takes the old method.
  EXPECT_TRUE(subdomainWidths_.empty());
  EXPECT_TRUE(subdomainHeights_.empty());
  EXPECT_EQ(m_, 2);
  EXPECT_EQ(n_, 2);
}

TEST_F(DecompositionCalculatorTest, testDecomposeWithTooManyCoresForDynamicMethodLowRes)
{

  std::vector<std::vector<int>> domainShape = getLargeCShapedModel();

  numberOfCores_ = 4;

  calculateDomainLowRes(domainShape, 4, 4);

  // Then it takes the old method.
  EXPECT_TRUE(subdomainWidths_.empty());
  EXPECT_TRUE(subdomainHeights_.empty());
  EXPECT_EQ(m_, 2);
  EXPECT_EQ(n_, 2);
}

TEST_F(DecompositionCalculatorTest, testDecomposeWithTooManyCoresForBothMethodsHighRes)
{
  std::vector<std::vector<int>> domainShape = {{1, 0},
                                               {1, 1}};


  numberOfCores_ = 5;

  EXPECT_FALSE(calculateDomain(domainShape));
}

TEST_F(DecompositionCalculatorTest, testDecomposeWithTooManyCoresForBothMethodsLowRes)
{
  std::vector<std::vector<int>> domainShape = getLargeCShapedModel();

  numberOfCores_ = 5;

  EXPECT_TRUE(calculateDomain(domainShape));
  EXPECT_FALSE(calculateDomainLowRes(domainShape, 2, 2));
}

TEST_F(DecompositionCalculatorTest, testDecomposeForWhichDynamicMethodIsJustAllowedDueToSubsampling)
{
  std::vector<std::vector<int>> domainShape = {{1, 1, 1, 1},
                                               {1, 0, 0, 0},
                                               {1, 0, 0, 0},
                                               {1, 0, 0, 0},
                                               {1, 0, 0, 0},
                                               {1, 0, 0, 0},
                                               {1, 0, 0, 0},
                                               {1, 0, 0, 0},
                                               {1, 1, 1, 1}};



  numberOfCores_ = 2;

  calculateDomainLowRes(domainShape, 5, 3);

  testInternalConsistencyNewMethod(domainShape);
}













