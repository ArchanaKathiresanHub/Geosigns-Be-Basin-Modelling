//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/input/cmbMapReader.h"

#include <gtest/gtest.h>

using namespace casaWizard;

class MapReaderTest : public ::testing::Test
{
public:
  CMBMapReader reader_{};
  MapReaderTest()
  {
    reader_.load("MapReaderTest.project3d");
  }
};

TEST_F(MapReaderTest, testgetMapData)
{
  VectorVectorMap lithoMap = reader_.getMapData("MAP-27109");

  // Sample test
  EXPECT_DOUBLE_EQ(lithoMap.getData()[2][93], 1564.73046875);
  EXPECT_DOUBLE_EQ(lithoMap.getData()[0][0], 1212.9725341796875);
  EXPECT_DOUBLE_EQ(lithoMap.getData()[56][8], 1274.77978515625);
}

TEST_F(MapReaderTest, testMapDoesNotExist)
{
  EXPECT_FALSE(reader_.mapExists("Non-existent map"));
}

TEST_F(MapReaderTest, testMapExists)
{
  EXPECT_TRUE(reader_.mapExists("MAP-27103"));
}

TEST_F(MapReaderTest, testGetValue)
{
  EXPECT_DOUBLE_EQ(reader_.getValue(179000 + 93 * 250, 603500 + 2 * 250, "MAP-27109"), 1564.73046875);
}

TEST_F(MapReaderTest, testCheckIfPointIsInLayer)
{
  EXPECT_TRUE(reader_.checkIfPointIsInLayer(179000 + 2 * 250, 603500 + 2 * 250, 528, "Middle_North_Sea_Group_SL"));

  EXPECT_FALSE(reader_.checkIfPointIsInLayer(179000 + 2 * 250, 603500 + 2 * 250, 527, "Middle_North_Sea_Group_SL"));
  EXPECT_FALSE(reader_.checkIfPointIsInLayer(179000 + 2 * 250, 603500 + 2 * 250, 528, "Lower_North_Sea_Group_SL"));

}

TEST_F(MapReaderTest, testGetMapDimensions)
{
  double xMin = 0.0;
  double xMax = 0.0;
  double yMin = 0.0;
  double yMax = 0.0;
  long numI = 0;
  long numJ = 0;
  reader_.getMapDimensions(xMin, xMax, yMin, yMax, numI, numJ);

  EXPECT_DOUBLE_EQ(xMin, 179000);
  EXPECT_DOUBLE_EQ(xMax, 205500);
  EXPECT_DOUBLE_EQ(yMin, 603500);
  EXPECT_DOUBLE_EQ(yMax, 618500);
  EXPECT_EQ(numI, 107);
  EXPECT_EQ(numJ, 61);
}



