//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <gtest/gtest.h>
#include "model/input/mapreader.h"

using namespace casaWizard;

class MapReaderTest : public ::testing::Test
{
public:
  MapReader reader_{};
  MapReaderTest()
  {
    reader_.load("MapReaderTest.project3d");
  }
};

TEST_F(MapReaderTest, testgetMapData)
{
  LithofractionMap lithoMap = reader_.getMapData("MAP-27109");

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



