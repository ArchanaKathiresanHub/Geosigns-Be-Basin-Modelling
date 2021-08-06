//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/output/zycorWriter.h"

#include "expectFileEq.h"

#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <stdio.h>

using namespace casaWizard;

TEST(testZycorWriter, testOutputFileExists)
{
  ZycorWriter writer;
  std::string fileName = "testFileExists.zyc";
  MapMetaData metaData;
  writer.writeToFile(fileName, {{0.0}}, metaData);

  std::ifstream outputFile(fileName);
  EXPECT_TRUE(outputFile.good());

  std::remove(fileName.c_str());
}

TEST(testZycorWriter, testWritingEmptyDataDoesNotGenerateFile)
{
  std::vector<std::vector<double>> data;

  ZycorWriter writer;
  std::string fileName = "test.zyc";
  MapMetaData metaData(416500, 508500, 6697000, 6759000, 0,0);
  writer.writeToFile(fileName, data, metaData);

  std::ifstream outputFile(fileName);
  EXPECT_FALSE(outputFile.good());
}

TEST(testZycorWriter, testOutputFileContent)
{
  std::vector<std::vector<double>> data;
  data.push_back({3290.412,   3057.17334, 2979.94116, 2907.48462});
  data.push_back({3357.61719, 3089.0437,  3002.877,   2902.54321});
  data.push_back({3430.68481, 3124.77368, 3016.0083,  2907.47949});
  data.push_back({3483.66260, 3175.269,   3012.47388, 2954.82666});
  data.push_back({3527.12769, 3219.81885, 3003.12231, 2992.963});

  ZycorWriter writer;
  std::string fileName = "test.zyc";
  MapMetaData metaData(416500, 508500, 6697000, 6759000, data[0].size(), data.size());
  writer.writeToFile(fileName, data, metaData);

  expectFileEq("test.zyc", "testZycorWriter.zyc");
  std::remove(fileName.c_str());
}

TEST(testZycorWriter, testWritingTwiceOverwritesFirstFile)
{
  std::vector<std::vector<double>> data;
  data.push_back({3290.412,   3057.17334, 2979.94116, 2907.48462});

  ZycorWriter writer;
  std::string fileName = "test.zyc";
  MapMetaData metaData(416500, 508500, 6697000, 6759000, data[0].size(), data.size());
  writer.writeToFile(fileName, data, metaData);

  std::vector<std::vector<double>> dataNew;
  dataNew.push_back({3290.412,   3057.17334, 2979.94116, 2907.48462});
  dataNew.push_back({3357.61719, 3089.0437,  3002.877,   2902.54321});
  dataNew.push_back({3430.68481, 3124.77368, 3016.0083,  2907.47949});
  dataNew.push_back({3483.66260, 3175.269,   3012.47388, 2954.82666});
  dataNew.push_back({3527.12769, 3219.81885, 3003.12231, 2992.963});
  MapMetaData metaDataNew(416500, 508500, 6697000, 6759000, dataNew[0].size(), dataNew.size());
  writer.writeToFile(fileName, dataNew, metaDataNew);

  expectFileEq("test.zyc", "testZycorWriter.zyc");
  std::remove(fileName.c_str());
}

TEST(testZycorWriter, testOutputFileContentLongNumber)
{
  std::vector<std::vector<double>> data;
  data.push_back({35271234443242141276912344124.1234124124124124, 12347189567341896578194623789141234.123412356312947283149});
  data.push_back({35271234443242141276912344124.1234124124124124, 12347189567341896578194623789141234.123412356312947283149});

  ZycorWriter writer;
  std::string fileName = "test.zyc";
  MapMetaData metaData(416500, 508500, 6697000, 6759000, data[0].size(), data.size());
  writer.writeToFile(fileName, data, metaData);

  expectFileEq("test.zyc", "testZycorWriterLongNumbers.zyc");
  std::remove(fileName.c_str());
}
