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

TEST(LASReaderTest, testReadFile)
{
  ImportOptions options;
  ExtractWellDataLAS reader("./Test1.las", options);
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
