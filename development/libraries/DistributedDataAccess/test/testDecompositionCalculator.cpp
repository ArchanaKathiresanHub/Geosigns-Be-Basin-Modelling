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
#ifdef WIN32
    _putenv_s("DYNAMIC_DECOMPOSITION_MAX_DEVIATION", "1e10");
#else
    putenv("DYNAMIC_DECOMPOSITION_MAX_DEVIATION=1e10");
#endif
  }

  void TearDown() override
  {
#ifdef WIN32
	  _putenv_s("DYNAMIC_DECOMPOSITION_MAX_DEVIATION","");
#else
	  putenv("DYNAMIC_DECOMPOSITION_MAX_DEVIATION=");
#endif
  }

  void calculateDomain(const std::vector<std::vector<int>>& domainShape)
  {
    DecompositionCalculator calculator(domainShape, numberOfCores_, domainShape.size(), domainShape[0].size());
    calculator.calculateDecomposition(m_, n_, subdomainWidths_, subdomainHeights_);
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

TEST_F(DecompositionCalculatorTest, testDecomposeTransposed)
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
  calculateDomain(domainShape);

  testInternalConsistencyNewMethod(domainShape);

  EXPECT_EQ(subdomainWidths_[0], 2);
  EXPECT_EQ(subdomainWidths_[1], 6);
  EXPECT_EQ(subdomainWidths_[2], 2);

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
#ifdef WIN32
  _putenv_s("DYNAMIC_DECOMPOSITION_MAX_DEVIATION","30");
#else
  putenv("DYNAMIC_DECOMPOSITION_MAX_DEVIATION=30");
#endif
  calculateDomain(domainShape);

  testInternalConsistencyNewMethod(domainShape);

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
#ifdef WIN32
  _putenv_s("DYNAMIC_DECOMPOSITION_MAX_DEVIATION","0");
#else
  putenv("DYNAMIC_DECOMPOSITION_MAX_DEVIATION=0");
#endif
  calculateDomain(domainShape);

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











