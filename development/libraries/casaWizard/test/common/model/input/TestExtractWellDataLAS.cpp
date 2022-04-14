//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/input/extractWellDataLAS.h"

#include "model/input/calibrationTargetCreator.h"

#include <gtest/gtest.h>

using namespace casaWizard;

TEST(ExtractWellDataLAS, testReadFile)
{
  ImportOptionsLAS options;
  ExtractWellDataLAS reader({"./Test1.las"}, options);
  while (reader.hasNextWell())
  {
    reader.extractMetaDataNextWell();
    EXPECT_EQ(reader.wellName().toStdString(), "test1");
  }

  reader.resetExtractor();

  while (reader.hasNextWell())
  {
    reader.extractDataNextWell();
    EXPECT_EQ(reader.wellName().toStdString(), "test1");
  }
}

TEST(ExtractWellDataLASTest, testSpecialCharactersWellName )
{
   const QString inputFileName{"./specialCharactersInWellName.las"};

   casaWizard::ImportOptionsLAS options;
   options.depthUserPropertyName = "DEPT";
   QStringList names = {inputFileName};
   casaWizard::ExtractWellDataLAS wellDataExtract{names,options};

   wellDataExtract.resetExtractor();

   while (wellDataExtract.hasNextWell())
   {
     wellDataExtract.extractDataNextWell();
     QString wellName = wellDataExtract.wellName();
     EXPECT_EQ(wellName,"S_A_P_E_L_E_1ST");
   }

}

TEST(ExtractWellDataLAS, testReadMultipleFiles)
{
  ImportOptionsLAS options;
  ExtractWellDataLAS reader({"./Test1.las", "./testWrap.las"}, options);
  std::vector<std::string> expectedWellNames = {"test1", "ANY ET AL 12-34-12-34"};

  int counter = 0;
  while (reader.hasNextWell())
  {
    reader.extractMetaDataNextWell();
    EXPECT_EQ(reader.wellName().toStdString(), expectedWellNames[counter]);
    counter++;
  }

  reader.resetExtractor();

  counter = 0;
  while (reader.hasNextWell())
  {
    reader.extractDataNextWell();
    EXPECT_EQ(reader.wellName().toStdString(), expectedWellNames[counter]);
    counter++;
  }

  EXPECT_EQ(options.wellNamesWithoutXYCoordinates.size(), 1);
  EXPECT_EQ(options.wellNamesWithoutXYCoordinates[0], "ANY ET AL 12-34-12-34");
  EXPECT_TRUE(options.allLasFilesAreTheCorrectVersion);
}

TEST(ExtractWellDataLAS, testReadMultipleFilesWithDifferentVersionsResultsInCorrectBoolean)
{
  ImportOptionsLAS options;
  ExtractWellDataLAS reader({"./Test1.las", "./Version3.las"}, options);

  while (reader.hasNextWell())
  {
    reader.extractMetaDataNextWell();
  }
  EXPECT_FALSE(options.allLasFilesAreTheCorrectVersion);
}
