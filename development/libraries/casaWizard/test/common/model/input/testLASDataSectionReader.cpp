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

TEST(LASDataSectionReaderTest, testReadDepthWithCorrection)
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
  options.correctForElevation = true;
  options.elevationCorrection = 5;
  options.elevationCorrectionUnit = "";
  options.referenceCorrection = 3;
  options.referenceCorrectionUnit = "ft";

  LASDataSectionReader reader(section, wellData, options);
  reader.readSection();

  ASSERT_EQ(wellData.depth_.size(), 5);
  EXPECT_DOUBLE_EQ(wellData.depth_[0], 93.4214 - 5 - 3 * 0.3048);
  EXPECT_DOUBLE_EQ(wellData.depth_[1], 236.6774 - 5 - 3 * 0.3048);
  EXPECT_DOUBLE_EQ(wellData.depth_[2], 236.6774 - 5 - 3 * 0.3048);
  EXPECT_DOUBLE_EQ(wellData.depth_[3], 236.6774 - 5 - 3 * 0.3048);
  EXPECT_DOUBLE_EQ(wellData.depth_[4], 236.6774 - 5 - 3 * 0.3048);
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

TEST(LASDataSectionReaderTest, testReadWrappedData)
{
  std::vector<std::string> section = {{"~A Log data section"},
                             {"910.000000"},
                             {" -999.2500 2692.7075 0.3140 19.4086 19.4086 13.1709 12.2681"},
                             {" -1.5010 96.5306 204.7177 30.5822 -999.2500 -999.2500 3.2515"},
                             {" -999.2500 4.7177 3025.0264 3025.0264 -1.5010 93.1378 0.1641"},
                             {" 0.0101 0.1641 0.3140 0.1641 11.1397 0.3304 0.9529"},
                             {" 0.0000 0.1564 0.0000 11.1397 0.0000 0.0000 0.0000"},
                             {"909.875000"},
                             {" -999.2500 2712.6460 0.2886 23.3987 23.3987 13.6129 12.4744"},
                             {" -1.4720 90.2803 203.1093 18.7566 -999.2500 -999.2500 3.7058"},
                             {" -999.2500 3.1093 3004.6050 3004.6050 -1.4720 86.9078 0.1456"},
                             {" -0.0015 0.1456 0.2886 0.1456 14.1428 0.2646 1.0000"},
                             {" 0.0000 0.1456 0.0000 14.1428 0.0000 0.0000 0.0000"},
                             {"909.750000"},
                             {" -999.2500 2692.8137 0.2730 22.5909 22.5909 13.6821 12.6146"},
                             {" -1.4804 89.8492 201.9287 3.1551 -999.2500 -999.2500 4.3124"},
                             {" -999.2500 1.9287 2976.4451 2976.4451 -1.4804 86.3465 0.1435"},
                             {" 0.0101 0.1435 0.2730 0.1435 14.5674 0.2598 1.0000"},
                             {" 0.0000 0.1435 0.0000 14.5674 0.0000 0.0000 0.0000"},
                             {"909.625000"},
                             {" -999.2500 2644.3650 0.2765 18.4831 18.4831 13.4159 12.6900"},
                             {" -1.5010 93.3999 201.5826 -6.5861 -999.2500 -999.2500 4.3822"},
                             {" -999.2500 1.5826 2955.3528 2955.3528 -1.5010 89.7142 0.1590"},
                             {" 0.0384 0.1590 0.2765 0.1590 11.8600 0.3210 0.9667"},
                             {" 0.0000 0.1538 0.0000 11.8600 0.0000 0.0000 0.0000"},
                             {"909.500000"},
                             {" -999.2500 2586.2822 0.2996 13.9187 13.9187 12.9195 12.7016"},
                             {" -1.4916 98.1214 201.7126 -4.5574 -999.2500 -999.2500 3.5967"},
                             {" -999.2500 1.7126 2953.5940 2953.5940 -1.4916 94.2670 0.1880"},
                             {" 0.0723 0.1880 0.2996 0.1880 8.4863 0.4490 0.8174"},
                             {" 0.0000 0.1537 0.0000 8.4863 0.0000 0.0000 0.0000"}};

  WellData wellData;
  wellData.calibrationTargetVarsUserName_ = {"", "", "", "", "", "", "", "", "", "",
                                             "", "", "", "", "", "", "", "", "", "",
                                             "", "", "", "", "", "", "", "", "", "",
                                             "", "", "", "", ""};
  wellData.nCalibrationTargetVars_ = wellData.calibrationTargetVarsUserName_.size();
  wellData.nDataPerTargetVar_.resize(wellData.nCalibrationTargetVars_);
  std::fill(wellData.nDataPerTargetVar_.begin(), wellData.nDataPerTargetVar_.end(), 0);
  ImportOptions options;
  options.depthUserPropertyName = "DEPTH";
  options.undefinedValue = -999.25;
  options.depthColumn = 0;
  options.wrapped = true;

  LASDataSectionReader reader(section, wellData, options);
  reader.readSection();

  int numberOfUndefinedValues = 20;
  int numberOfProperties = 35;
  int numberOfDepthValues = 5;
  ASSERT_EQ(wellData.depth_.size(), numberOfProperties * numberOfDepthValues - numberOfUndefinedValues);
  ASSERT_EQ(wellData.calibrationTargetValues_.size(), numberOfProperties * numberOfDepthValues - numberOfUndefinedValues);

  EXPECT_DOUBLE_EQ(wellData.depth_[0], 910.000000);
  EXPECT_DOUBLE_EQ(wellData.depth_[1], 909.875000);
  EXPECT_DOUBLE_EQ(wellData.depth_[2], 909.750000);
  EXPECT_DOUBLE_EQ(wellData.depth_[3], 909.625000);
  EXPECT_DOUBLE_EQ(wellData.depth_[4], 909.500000);

  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[0], 2692.7075);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[1], 2712.6460);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[2], 2692.8137);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[3], 2644.3650);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[4], 2586.2822);

  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[50], 3.2515);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[51], 3.7058);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[52], 4.3124);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[53], 4.3822);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[54], 3.5967);
}

TEST(LASDataSectionReaderTest, testReadWrappedDataWithOnePropertyOnLastWrappedLine)
{
  std::vector<std::string> section = {{"~A Log data section"},
                             {"910.000000"},
                             {" -999.2500 2692.7075 0.3140 19.4086 19.4086 13.1709 12.2681"},
                             {" -1.5010 96.5306 204.7177 30.5822 -999.2500 -999.2500 3.2515"},
                             {" -999.2500 4.7177 3025.0264 3025.0264 -1.5010 93.1378 0.1641"},
                             {" 0.0101 0.1641 0.3140 0.1641 11.1397 0.3304 0.9529"},
                             {" 0.0000 0.1564 0.0000 11.1397 0.0000 0.0000 0.0000"},
                             {"0.4"},
                             {"909.875000"},
                             {" -999.2500 2712.6460 0.2886 23.3987 23.3987 13.6129 12.4744"},
                             {" -1.4720 90.2803 203.1093 18.7566 -999.2500 -999.2500 3.7058"},
                             {" -999.2500 3.1093 3004.6050 3004.6050 -1.4720 86.9078 0.1456"},
                             {" -0.0015 0.1456 0.2886 0.1456 14.1428 0.2646 1.0000"},
                             {" 0.0000 0.1456 0.0000 14.1428 0.0000 0.0000 0.0000"},
                             {"0.4"},
                             {"909.750000"},
                             {" -999.2500 2692.8137 0.2730 22.5909 22.5909 13.6821 12.6146"},
                             {" -1.4804 89.8492 201.9287 3.1551 -999.2500 -999.2500 4.3124"},
                             {" -999.2500 1.9287 2976.4451 2976.4451 -1.4804 86.3465 0.1435"},
                             {" 0.0101 0.1435 0.2730 0.1435 14.5674 0.2598 1.0000"},
                             {" 0.0000 0.1435 0.0000 14.5674 0.0000 0.0000 0.0000"},
                             {"0.4"},
                             {"909.625000"},
                             {" -999.2500 2644.3650 0.2765 18.4831 18.4831 13.4159 12.6900"},
                             {" -1.5010 93.3999 201.5826 -6.5861 -999.2500 -999.2500 4.3822"},
                             {" -999.2500 1.5826 2955.3528 2955.3528 -1.5010 89.7142 0.1590"},
                             {" 0.0384 0.1590 0.2765 0.1590 11.8600 0.3210 0.9667"},
                             {" 0.0000 0.1538 0.0000 11.8600 0.0000 0.0000 0.0000"},
                             {"0.4"},
                             {"909.500000"},
                             {" -999.2500 2586.2822 0.2996 13.9187 13.9187 12.9195 12.7016"},
                             {" -1.4916 98.1214 201.7126 -4.5574 -999.2500 -999.2500 3.5967"},
                             {" -999.2500 1.7126 2953.5940 2953.5940 -1.4916 94.2670 0.1880"},
                             {" 0.0723 0.1880 0.2996 0.1880 8.4863 0.4490 0.8174"},
                             {" 0.0000 0.1537 0.0000 8.4863 0.0000 0.0000 0.0000"},
                             {"0.4"}};

  WellData wellData;
  wellData.calibrationTargetVarsUserName_ = {"", "", "", "", "", "", "", "", "", "",
                                             "", "", "", "", "", "", "", "", "", "",
                                             "", "", "", "", "", "", "", "", "", "",
                                             "", "", "", "", "", ""};
  wellData.nCalibrationTargetVars_ = wellData.calibrationTargetVarsUserName_.size();
  wellData.nDataPerTargetVar_.resize(wellData.nCalibrationTargetVars_);
  std::fill(wellData.nDataPerTargetVar_.begin(), wellData.nDataPerTargetVar_.end(), 0);
  ImportOptions options;
  options.depthUserPropertyName = "DEPTH";
  options.undefinedValue = -999.25;
  options.depthColumn = 0;
  options.wrapped = true;

  LASDataSectionReader reader(section, wellData, options);
  reader.readSection();

  int numberOfUndefinedValues = 20;
  int numberOfProperties = 36;
  int numberOfDepthValues = 5;
  ASSERT_EQ(wellData.depth_.size(), numberOfProperties * numberOfDepthValues - numberOfUndefinedValues);
  ASSERT_EQ(wellData.calibrationTargetValues_.size(), numberOfProperties * numberOfDepthValues - numberOfUndefinedValues);

  EXPECT_DOUBLE_EQ(wellData.depth_[0], 910.000000);
  EXPECT_DOUBLE_EQ(wellData.depth_[1], 909.875000);
  EXPECT_DOUBLE_EQ(wellData.depth_[2], 909.750000);
  EXPECT_DOUBLE_EQ(wellData.depth_[3], 909.625000);
  EXPECT_DOUBLE_EQ(wellData.depth_[4], 909.500000);

  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[0], 2692.7075);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[1], 2712.6460);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[2], 2692.8137);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[3], 2644.3650);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[4], 2586.2822);

  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[50], 3.2515);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[51], 3.7058);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[52], 4.3124);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[53], 4.3822);
  EXPECT_DOUBLE_EQ(wellData.calibrationTargetValues_[54], 3.5967);
}


