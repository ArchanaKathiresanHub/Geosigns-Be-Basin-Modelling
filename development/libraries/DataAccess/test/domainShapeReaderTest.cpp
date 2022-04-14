// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.

// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.

#include "domainShapeReader.h"

#include <gtest/gtest.h>

using namespace DataAccess;

class DomainShapeReaderTest : public ::testing::Test
{
  public:
    DomainShapeReaderTest() {}
};


TEST_F(DomainShapeReaderTest, testReadDomainWithoutFileName)
{
  DomainShapeReader domainShapeReader("");
  std::vector<std::vector<int>> domainShape;
  Window window(0, 1, 0, 2);
  domainShapeReader.readShape(window, domainShape);

  EXPECT_TRUE(domainShape.empty());
}

TEST_F(DomainShapeReaderTest, testReadDomainWithValidFile)
{
  DomainShapeReader domainShapeReader("./testDomainShapeReader.h5");
  std::vector<std::vector<int>> domainShape;
  Window window(0, 2, 0, 3);
  domainShapeReader.readShape(window, domainShape);

  std::vector<std::vector<int>> expectedDomainShape = {{0, 1, 1, 0},
                                                       {0, 1, 1, 1},
                                                       {1, 1, 0, 1}};

  for (int i = 0; i < expectedDomainShape.size(); i++)
  {
    for (int j = 0; j < expectedDomainShape[i].size(); j++)
    {
      EXPECT_EQ(domainShape[i][j], expectedDomainShape[i][j]);
    }
  }
}

TEST_F(DomainShapeReaderTest, testReadDomainWithWrongDimensionsReturnsZero)
{
  DomainShapeReader domainShapeReader("./testDomainShapeReader.h5");
  std::vector<std::vector<int>> domainShape;
  Window window(-1, 2, 0, 1);
  domainShapeReader.readShape(window, domainShape);
  EXPECT_EQ(domainShape.size(), 0);

  Window window2(0, 3, 0, 1);
  domainShapeReader.readShape(window2, domainShape);
  EXPECT_EQ(domainShape.size(), 0);
}

TEST_F(DomainShapeReaderTest, testReadWindowedDomainWithValidFile)
{
  DomainShapeReader domainShapeReader("./testDomainShapeReader.h5");
  std::vector<std::vector<int>> domainShape;
  Window window(1, 2, 1, 3);
  domainShapeReader.readShape(window, domainShape);

  std::vector<std::vector<int>> expectedDomainShape = {{1, 1, 1},
                                                       {1, 0, 1}};

  EXPECT_EQ(domainShape.size(), expectedDomainShape.size());
  if (domainShape.size() > 0)
  {
    EXPECT_EQ(domainShape[0].size(), expectedDomainShape[0].size());
  }

  for (int i = 0; i < expectedDomainShape.size(); i++)
  {
    for (int j = 0; j < expectedDomainShape[i].size(); j++)
    {
      EXPECT_EQ(domainShape[i][j], expectedDomainShape[i][j]);
    }
  }
}
