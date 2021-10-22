//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/input/lasWellInfoSectionReader.h"
#include "model/input/calibrationTargetCreator.h"
#include "model/wellData.h"

#include <gtest/gtest.h>

using namespace casaWizard;

std::vector<std::string> getSection()
{
  std::vector<std::string> section = {"~WELL INFORMATION\r",
                                      "STRT  .m         91.5926              :START DEPTH\r",
                                      "STEP  .m         0.1524               :STEP DEPTH\r",
                                      "STOP  .m         4352.3918            :STOP DEPTH\r",
                                      "NULL  .          -999.25              :NULL VALUE\r",
                                      "COMP  .                               :COMPANY\r",
                                      "WELL  .          test 1                :WELL\r",
                                      "LOC   .                               :LOCATION - AREA\r",
                                      "FLD   .          UNKNOWN              :FIELD\r",
                                      "CTRY  .          SN                   :COUNTRY\r",
                                      "DATE  .                               :WELL DATE\r",
                                      "UWI   .          99000002255501                     :UWI",
                                      "XCOORD .m          242118.815           :X POS",
                                      "YCOORD .m          1578898.128          :Y POS",
                                      "EREF   .          9.144018173217773    :DERRICK FLOOR ELEVATION",
                                      };

  return section;
}

TEST(LasWellInfoSectionReaderTest, testRead)
{
  std::vector<std::string> section = getSection();
  WellData wellData;
  ImportOptions options;
  LASWellInfoSectionReader reader(section, wellData, options);
  reader.readSection();
  EXPECT_EQ(wellData.wellName_.toStdString(), "test 1");
  EXPECT_DOUBLE_EQ(wellData.xCoord_, 242118.815);
  EXPECT_DOUBLE_EQ(wellData.yCoord_, 1578898.128);
  EXPECT_DOUBLE_EQ(options.elevationCorrection, 9.144018173217773);
  EXPECT_DOUBLE_EQ(options.undefinedValue, -999.25);
}

void testSectionThrows(const std::vector<std::string>& section, const std::string& expectedMessage)
{
  WellData wellData;
  ImportOptions options;
  LASWellInfoSectionReader reader(section, wellData, options);
  EXPECT_THROW(reader.readSection(), std::runtime_error);

  try
  {
    reader.readSection();
  } catch (std::runtime_error e)
  {
    const std::string message = e.what();
    EXPECT_EQ(message, expectedMessage);
  }
}

TEST(LasWellInfoSectionReaderTest, testReadWithoutWellNameThrows)
{
  std::vector<std::string> section = getSection();
  section.erase(section.begin() + 6);
  const std::string expectedMessage = "Invalid LAS-file: No well name is provided.";

  testSectionThrows(section, expectedMessage);
}

TEST(LasWellInfoSectionReaderTest, testReadWithInvalidWellNameLineThrows)
{
  std::vector<std::string> section = getSection();
  section[6] = "WELL.     : Well Name";
  const std::string expectedMessage = "Invalid LAS-file: No well name is provided.";

  testSectionThrows(section, expectedMessage);
}

TEST(LasWellInfoSectionReaderTest, testReadWithInvalidWellNameLineThrows2)
{
  std::vector<std::string> section = getSection();
  section[6] = "WELL.    ";
  const std::string expectedMessage = "Invalid LAS-file: No well name is provided.";

  testSectionThrows(section, expectedMessage);
}

TEST(LasWellInfoSectionReaderTest, testReadWithoutXCoordinateThrows)
{
  std::vector<std::string> section = getSection();
  section.erase(section.begin() + 12);

  const std::string expectedMessage = "Invalid LAS-file: No x coordinate is provided.";

  testSectionThrows(section, expectedMessage);
}

TEST(LasWellInfoSectionReaderTest, testReadWithoutYCoordinateThrows)
{
  std::vector<std::string> section = getSection();
  section.erase(section.begin() + 13);

  const std::string expectedMessage = "Invalid LAS-file: No y coordinate is provided.";

  testSectionThrows(section, expectedMessage);
}



