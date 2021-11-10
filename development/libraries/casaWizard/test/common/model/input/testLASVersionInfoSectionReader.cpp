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
  ImportOptionsLAS options;

  LASVersionInfoSectionReader reader(section, wellData, options);
  EXPECT_NO_THROW(reader.readSection());
  EXPECT_FALSE(options.wrapped);
  EXPECT_TRUE(options.allLasFilesAreTheCorrectVersion);
}

TEST(LasVersionInfoSectionReaderTest, testReadDifferentVersion)
{
  std::vector<std::string> section = {"~VERSION INFORMATION",
                                        "VERS.            1.2                  :CWLS LOG ASCII STANDARD -VERSION 1.2",
                                        "WRAP.            NO                   :ONE LINE PER DEPTH STEP",
                                      };
  WellData wellData;
  ImportOptionsLAS options;

  LASVersionInfoSectionReader reader(section, wellData, options);
  EXPECT_NO_THROW(reader.readSection());
  EXPECT_FALSE(options.wrapped);
  EXPECT_FALSE(options.allLasFilesAreTheCorrectVersion);
}

TEST(LasVersionInfoSectionReaderTest, testReadWithWrapping)
{
  std::vector<std::string> section = {"~VERSION INFORMATION",
                                        "VERS.            2.0                  :CWLS LOG ASCII STANDARD -VERSION 2.0",
                                        "WRAP.            YES                  :WRAPPING LINES AROUND 80 CHARACTERS",
                                      };
  WellData wellData;
  ImportOptionsLAS options;

  LASVersionInfoSectionReader reader(section, wellData, options);
  EXPECT_NO_THROW(reader.readSection());
  EXPECT_TRUE(options.wrapped);
}

TEST(LasVersionInfoSectionReaderTest, testNoVersionGivenThrows)
{
  std::vector<std::string> section = {"~VERSION INFORMATION",
                                        "WRAP.            YES                  :WRAPPING LINES AROUND 80 CHARACTERS",
                                      };
  WellData wellData;
  ImportOptionsLAS options;
  LASVersionInfoSectionReader reader(section, wellData, options);
  EXPECT_THROW(reader.readSection(), std::runtime_error);

  try
  {
    reader.readSection();
  }
  catch (std::runtime_error e)
  {
    std::string message = e.what();
    EXPECT_EQ(message, "No LAS version detected.");
  }
}

TEST(LasVersionInfoSectionReaderTest, testNoWrappingSpecificationGivenThrows)
{
  std::vector<std::string> section = {"~VERSION INFORMATION",
                                        "VERS.            2.0                  :CWLS LOG ASCII STANDARD -VERSION 3.0",
                                      };
  WellData wellData;
  ImportOptionsLAS options;
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
  ImportOptionsLAS options;
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

TEST(LasVersionInfoSectionReaderTest, testInvalidLineThrows)
{
  std::vector<std::string> section = {"~VERSION INFORMATION",
                                        "VERS.",
                                        "WRAP.            NO                   :ONE LINE PER DEPTH STEP",
                                      };
  WellData wellData;
  ImportOptionsLAS options;
  LASVersionInfoSectionReader reader(section, wellData, options);
  EXPECT_THROW(reader.readSection(), std::runtime_error);

  try
  {
    reader.readSection();
  }
  catch (std::runtime_error e)
  {
    std::string message = e.what();
    EXPECT_EQ(message, "Invalid line in Version Info Section (~V).");
  }
}

TEST(LasVersionInfoSectionReaderTest, testLineWithoutCommentDoesNotThrow)
{
  std::vector<std::string> section = {"~VERSION INFORMATION",
                                        "VERS.            2.0",
                                        "WRAP.            NO                   :ONE LINE PER DEPTH STEP",
                                      };
  WellData wellData;
  ImportOptionsLAS options;
  LASVersionInfoSectionReader reader(section, wellData, options);
  EXPECT_NO_THROW(reader.readSection());
  EXPECT_FALSE(options.wrapped);
}
