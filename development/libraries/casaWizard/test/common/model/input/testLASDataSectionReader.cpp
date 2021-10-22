//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/input/lasDataSectionReader.h"
#include "model/input/calibrationTargetCreator.h"

#include "model/wellData.h"

#include <gtest/gtest.h>

using namespace casaWizard;

TEST(LASDataSectionReaderTest, testRead)
{
  std::vector<std::string> section = {"~A          DEN         DENCOR      DTC         GR          \r",
                                      "93.4214     1.5         -999.25     -999.25     -999.25     \r",
                                      "236.6774    1.7394      0.109       121.7292    46.1679     \r"};

  WellData wellData;
  wellData.calibrationTargetVarsUserName_ = {"DEN", "DENCOR", "DTC", "GR"};
  wellData.nCalibrationTargetVars_ = wellData.calibrationTargetVarsUserName_.size();
  wellData.nDataPerTargetVar_.resize(wellData.nCalibrationTargetVars_);
  std::fill(wellData.nDataPerTargetVar_.begin(), wellData.nDataPerTargetVar_.end(), 0);
  ImportOptions options;
  options.undefinedValue = -999.25;
  options.depthColumn = 0;

  LASDataSectionReader reader(section, wellData, options);
  reader.readSection();

  ASSERT_EQ(wellData.depth_.size(), 5);
  EXPECT_DOUBLE_EQ(wellData.depth_[0], 93.4214);
  EXPECT_DOUBLE_EQ(wellData.depth_[1], 236.6774);
  EXPECT_DOUBLE_EQ(wellData.depth_[2], 236.6774);
  EXPECT_DOUBLE_EQ(wellData.depth_[3], 236.6774);
  EXPECT_DOUBLE_EQ(wellData.depth_[4], 236.6774);

  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[0], 1.5);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[1], 1.7394);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[2], 0.109);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[3], 121.7292);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[4], 46.1679);
}


TEST(LASDataSectionReaderTest, testReadWithDifferentDepthColumn)
{
  std::vector<std::string> section = {"~A          TVD         DENCOR      DTC         GR          \r",
                                      "93.4214     70.0        -999.25     -999.25     58.4        \r",
                                      "236.6774    180.0       0.109       121.7292    46.1679     \r"};

  WellData wellData;
  wellData.calibrationTargetVarsUserName_ = {"DEPT", "DENCOR", "DTC", "GR"};
  wellData.nCalibrationTargetVars_ = wellData.calibrationTargetVarsUserName_.size();
  wellData.nDataPerTargetVar_.resize(wellData.nCalibrationTargetVars_);
  std::fill(wellData.nDataPerTargetVar_.begin(), wellData.nDataPerTargetVar_.end(), 0);
  ImportOptions options;
  options.undefinedValue = -999.25;
  options.depthColumn = 1;

  LASDataSectionReader reader(section, wellData, options);
  reader.readSection();

  ASSERT_EQ(wellData.depth_.size(), 6);
  EXPECT_DOUBLE_EQ(wellData.depth_[0], 70.0);
  EXPECT_DOUBLE_EQ(wellData.depth_[1], 180.0 );
  EXPECT_DOUBLE_EQ(wellData.depth_[2], 180.0 );
  EXPECT_DOUBLE_EQ(wellData.depth_[3], 180.0 );
  EXPECT_DOUBLE_EQ(wellData.depth_[4], 70.0 );
  EXPECT_DOUBLE_EQ(wellData.depth_[5], 180.0 );

  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[0], 93.4214);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[1], 236.6774);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[2], 0.109);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[3], 121.7292);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[4], 58.4);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[5], 46.1679);
}

TEST(LASDataSectionReaderTest, testReadWithoutColumnNames)
{
  std::vector<std::string> section = {"~A                   \r",
                                      "93.4214     1.5         -999.25     -999.25     -999.25     \r",
                                      "236.6774    1.7394      0.109       121.7292    46.1679     \r"};

  WellData wellData;
  wellData.calibrationTargetVarsUserName_ = {"DEN", "DENCOR", "DTC", "GR"};
  wellData.nCalibrationTargetVars_ = wellData.calibrationTargetVarsUserName_.size();
  wellData.nDataPerTargetVar_.resize(wellData.nCalibrationTargetVars_);
  std::fill(wellData.nDataPerTargetVar_.begin(), wellData.nDataPerTargetVar_.end(), 0);
  ImportOptions options;
  options.undefinedValue = -999.25;
  options.depthColumn = 0;

  LASDataSectionReader reader(section, wellData, options);
  reader.readSection();

  ASSERT_EQ(wellData.depth_.size(), 5);
  EXPECT_DOUBLE_EQ(wellData.depth_[0], 93.4214);
  EXPECT_DOUBLE_EQ(wellData.depth_[1], 236.6774);
  EXPECT_DOUBLE_EQ(wellData.depth_[2], 236.6774);
  EXPECT_DOUBLE_EQ(wellData.depth_[3], 236.6774);
  EXPECT_DOUBLE_EQ(wellData.depth_[4], 236.6774);

  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[0], 1.5);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[1], 1.7394);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[2], 0.109);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[3], 121.7292);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[4], 46.1679);
}


TEST(LASDataSectionReaderTest, testReadWithInvalidLine)
{
  std::vector<std::string> section = {"~A          TVD         DENCOR      DTC         GR          \r",
                                      "93.4214     70.0        -999.25     -999.25     58.4        \r",
                                      "236.6774    180.0       121.7292    46.1679     \r"};

  WellData wellData;
  wellData.calibrationTargetVarsUserName_ = {"DEPT", "DENCOR", "DTC", "GR"};
  wellData.nCalibrationTargetVars_ = wellData.calibrationTargetVarsUserName_.size();
  wellData.nDataPerTargetVar_.resize(wellData.nCalibrationTargetVars_);
  std::fill(wellData.nDataPerTargetVar_.begin(), wellData.nDataPerTargetVar_.end(), 0);
  ImportOptions options;
  options.undefinedValue = -999.25;
  options.depthColumn = 1;

  LASDataSectionReader reader(section, wellData, options);

  EXPECT_THROW(reader.readSection(), std::runtime_error);

  try
  {
    reader.readSection();
  } catch (std::runtime_error e)
  {
    std::string message = e.what();
    EXPECT_EQ(message, "Invalid line in the LAS data section (~A)");
  }
}

TEST(LASDataSectionReaderTest, testReadWithUnitConversions)
{
  std::vector<std::string> section = {"~A          DEN         DENCOR      DTC         GR          \r",
                                      "93.4214     1.5         -999.25     -999.25     -999.25     \r",
                                      "236.6774    1.7394      0.109       121.7292    46.1679     \r"};

  WellData wellData;
  wellData.calibrationTargetVarsUserName_ = {"DEN", "DENCOR", "DTC", "GR"};
  wellData.nCalibrationTargetVars_ = wellData.calibrationTargetVarsUserName_.size();
  wellData.nDataPerTargetVar_.resize(wellData.nCalibrationTargetVars_);
  std::fill(wellData.nDataPerTargetVar_.begin(), wellData.nDataPerTargetVar_.end(), 0);
  ImportOptions options;
  options.depthUserPropertyName = "DEPTH";
  options.userPropertyNameToUnitConversion = {{"DEN", 1000}, {"DENCOR", 1000}, {"DTC", 3.28083989501}, {"DEPTH", 3}};
  options.undefinedValue = -999.25;
  options.depthColumn = 0;

  LASDataSectionReader reader(section, wellData, options);
  reader.readSection();

  ASSERT_EQ(wellData.depth_.size(), 5);
  EXPECT_DOUBLE_EQ(wellData.depth_[0], 3 * 93.4214);
  EXPECT_DOUBLE_EQ(wellData.depth_[1], 3 * 236.6774);
  EXPECT_DOUBLE_EQ(wellData.depth_[2], 3 * 236.6774);
  EXPECT_DOUBLE_EQ(wellData.depth_[3], 3 * 236.6774);
  EXPECT_DOUBLE_EQ(wellData.depth_[4], 3 * 236.6774);

  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[0], 1000 * 1.5);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[1], 1000 * 1.7394);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[2], 1000 * 0.109);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[3], 3.28083989501 * 121.7292);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[4], 46.1679);
}

