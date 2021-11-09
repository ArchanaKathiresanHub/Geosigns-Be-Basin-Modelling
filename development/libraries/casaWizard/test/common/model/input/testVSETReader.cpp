//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/input/extractWellDataVSET.h"

#include "model/input/calibrationTargetCreator.h"

#include <gtest/gtest.h>

using namespace casaWizard;

TEST(VSETReaderTest, testReadFile)
{
  ExtractWellDataVSET reader("./VSETdata.vs");

  ASSERT_TRUE(reader.hasNextWell());
  reader.extractMetaDataNextWell();
  ASSERT_FALSE(reader.hasNextWell());

  ImportOptionsVSET options = reader.getImportOptions();
  EXPECT_TRUE(options.depthNotTWT);
  EXPECT_EQ(options.undefinedValue, -99999);

  options.distance = 0;
  options.interval = 1;
  reader.setImportOptions(options);

  reader.resetExtractor();

  const std::vector<double> expectedX{293316, 293183};
  const std::vector<double> expectedY{1782381, 1782169};
  const std::vector<int> expectedDepthSize{192, 193};
  const std::vector<std::string> expectedWellNames{"PSW_XXX_1", "PSW_XXX_2"};

  for (int i = 0; i < 2; ++i)
  {
    ASSERT_TRUE(reader.hasNextWell());
    reader.extractDataNextWell();

    EXPECT_DOUBLE_EQ(reader.xCoord(), expectedX[i]);
    EXPECT_DOUBLE_EQ(reader.yCoord(), expectedY[i]);
    EXPECT_EQ(reader.depth().size(), expectedDepthSize[i]);
    EXPECT_EQ(reader.wellName().toStdString(), expectedWellNames[i]);
  }
}

TEST(VSETReaderTest, testReadFileUsingInterval)
{  
  ExtractWellDataVSET reader("./VSETdata.vs");

  ASSERT_TRUE(reader.hasNextWell());
  reader.extractMetaDataNextWell();
  ASSERT_FALSE(reader.hasNextWell());

  ImportOptionsVSET options = reader.getImportOptions();
  EXPECT_TRUE(options.depthNotTWT);
  EXPECT_EQ(options.undefinedValue, -99999);

  options.distance = 0;
  options.interval = 2;
  reader.setImportOptions(options);

  reader.resetExtractor();

  const double expectedX{293183};
  const double expectedY{1782169};
  const int expectedDepthSize{193};
  const std::string expectedWellName{"PSW_XXX_1"};

  ASSERT_TRUE(reader.hasNextWell());
  reader.extractDataNextWell();

  EXPECT_DOUBLE_EQ(reader.xCoord(), expectedX);
  EXPECT_DOUBLE_EQ(reader.yCoord(), expectedY);
  EXPECT_EQ(reader.depth().size(), expectedDepthSize);
  EXPECT_EQ(reader.wellName().toStdString(), expectedWellName);

  EXPECT_FALSE(reader.hasNextWell());
}

TEST(VSETReaderTest, testReadFileUsingDistance)
{
  ExtractWellDataVSET reader("./VSETdata.vs");

  ASSERT_TRUE(reader.hasNextWell());
  reader.extractMetaDataNextWell();
  ASSERT_FALSE(reader.hasNextWell());

  ImportOptionsVSET options = reader.getImportOptions();
  EXPECT_TRUE(options.depthNotTWT);
  EXPECT_EQ(options.undefinedValue, -99999);

  options.distance = 10000;
  options.interval = 0;
  reader.setImportOptions(options);

  reader.resetExtractor();

  const double expectedX{293316};
  const double expectedY{1782381};
  const int expectedDepthSize{192};
  const std::string expectedWellName{"PSW_XXX_1"};

  ASSERT_TRUE(reader.hasNextWell());
  reader.extractDataNextWell();

  EXPECT_DOUBLE_EQ(reader.xCoord(), expectedX);
  EXPECT_DOUBLE_EQ(reader.yCoord(), expectedY);
  EXPECT_EQ(reader.depth().size(), expectedDepthSize);
  EXPECT_EQ(reader.wellName().toStdString(), expectedWellName);

  EXPECT_FALSE(reader.hasNextWell());
}

TEST(VSETReaderTest, testReadFileUsingXYvalues)
{
  ExtractWellDataVSET reader("./VSETdata.vs");

  ASSERT_TRUE(reader.hasNextWell());
  reader.extractMetaDataNextWell();
  ASSERT_FALSE(reader.hasNextWell());

  ImportOptionsVSET options = reader.getImportOptions();
  EXPECT_TRUE(options.depthNotTWT);
  EXPECT_EQ(options.undefinedValue, -99999);

  options.distance = 0;
  options.interval = 0;
  options.xyPairs.push_back({293317,1782382});
  reader.setImportOptions(options);

  reader.resetExtractor();

  const double expectedX{293316};
  const double expectedY{1782381};
  const int expectedDepthSize{192};
  const std::string expectedWellName{"PSW_XXX_1"};

  ASSERT_TRUE(reader.hasNextWell());
  reader.extractDataNextWell();

  EXPECT_DOUBLE_EQ(reader.xCoord(), expectedX);
  EXPECT_DOUBLE_EQ(reader.yCoord(), expectedY);
  EXPECT_EQ(reader.depth().size(), expectedDepthSize);
  EXPECT_EQ(reader.wellName().toStdString(), expectedWellName);

  EXPECT_FALSE(reader.hasNextWell());
}
