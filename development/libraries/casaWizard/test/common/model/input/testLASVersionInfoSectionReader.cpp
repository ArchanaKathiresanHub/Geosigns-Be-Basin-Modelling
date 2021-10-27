//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/input/lasVersionInfoSectionReader.h"

#include "model/wellData.h"
#include "model/input/importOptions.h"

#include <gtest/gtest.h>

using namespace casaWizard;

TEST(LasVersionInfoSectionReaderTest, testRead)
{
  std::vector<std::string> section = {"~VERSION INFORMATION",
                                        "VERS.            2.0                  :CWLS LOG ASCII STANDARD -VERSION 2.0",
                                        "WRAP.            NO                   :ONE LINE PER DEPTH STEP",
                                      };
  WellData wellData;
  ImportOptions options;

  LASVersionInfoSectionReader reader(section, wellData, options);
  EXPECT_NO_THROW(reader.readSection());
  EXPECT_FALSE(options.wrapped);
}

TEST(LasVersionInfoSectionReaderTest, testReadWithWrapping)
{
  std::vector<std::string> section = {"~VERSION INFORMATION",
                                        "VERS.            2.0                  :CWLS LOG ASCII STANDARD -VERSION 2.0",
                                        "WRAP.            YES                  :WRAPPING LINES AROUND 80 CHARACTERS",
                                      };
  WellData wellData;
  ImportOptions options;

  LASVersionInfoSectionReader reader(section, wellData, options);
  EXPECT_NO_THROW(reader.readSection());
  EXPECT_TRUE(options.wrapped);
}

TEST(LasVersionInfoSectionReaderTest, testOtherVersionThan2Throws)
{
  std::vector<std::string> section = {"~VERSION INFORMATION",
                                        "VERS.            3.0                  :CWLS LOG ASCII STANDARD -VERSION 3.0",
                                        "WRAP.            YES                  :WRAPPING LINES AROUND 80 CHARACTERS",
                                      };
  WellData wellData;
  ImportOptions options;
  LASVersionInfoSectionReader reader(section, wellData, options);
  EXPECT_THROW(reader.readSection(), std::runtime_error);

  try
  {
    reader.readSection();
  }
  catch (std::runtime_error e)
  {
    std::string message = e.what();
    EXPECT_EQ(message, "Wrong LAS version, only 2.0 is supported.");
  }
}

TEST(LasVersionInfoSectionReaderTest, testNoVersionGivenThrows)
{
  std::vector<std::string> section = {"~VERSION INFORMATION",
                                        "WRAP.            YES                  :WRAPPING LINES AROUND 80 CHARACTERS",
                                      };
  WellData wellData;
  ImportOptions options;
  LASVersionInfoSectionReader reader(section, wellData, options);
  EXPECT_THROW(reader.readSection(), std::runtime_error);

  try
  {
    reader.readSection();
  }
  catch (std::runtime_error e)
  {
    std::string message = e.what();
    EXPECT_EQ(message, "No LAS version detected, only version 2.0 is supported.");
  }
}

TEST(LasVersionInfoSectionReaderTest, testNoWrappingSpecificationGivenThrows)
{
  std::vector<std::string> section = {"~VERSION INFORMATION",
                                        "VERS.            2.0                  :CWLS LOG ASCII STANDARD -VERSION 3.0",
                                      };
  WellData wellData;
  ImportOptions options;
  LASVersionInfoSectionReader reader(section, wellData, options);
  EXPECT_THROW(reader.readSection(), std::runtime_error);

  try
  {
    reader.readSection();
  }
  catch (std::runtime_error e)
  {
    std::string message = e.what();
    EXPECT_EQ(message, "No wrapping specification detected.");
  }
}

TEST(LasVersionInfoSectionReaderTest, testEmptySectionThrows)
{
  std::vector<std::string> section = {"~VERSION INFORMATION",
                                      };
  WellData wellData;
  ImportOptions options;
  LASVersionInfoSectionReader reader(section, wellData, options);
  EXPECT_THROW(reader.readSection(), std::runtime_error);

  try
  {
    reader.readSection();
  }
  catch (std::runtime_error e)
  {
    std::string message = e.what();
    EXPECT_EQ(message, "Empty Version Info Section (~V).");
  }
}
