//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/input/lasCurveInfoSectionReader.h"
#include "model/input/calibrationTargetCreator.h"

#include "model/wellData.h"

#include <gtest/gtest.h>

using namespace casaWizard;

TEST(LasCurveInfoSectionReaderTest, testRead)
{
  std::vector<std::string> section = {"~CURVE INFORMATION",
                                        "DEPT        .m                              : 1      DEPTH",
                                        "DEN         .g/cm3                          : 2      Rho",
                                        "DENCOR      .g/cm3                          : 3      Rho",
                                        "DTC         .us/ft                          : 4      p_Sonic",
                                        "GR          .API                            : 5      GR"
                                      };
  WellData wellData;
  ImportOptions options;
  options.depthUserPropertyName = "DEPT";

  LASCurveInfoSectionReader reader(section, wellData, options);
  reader.readSection();

  QVector<QString> calibrationTargetVarsUserNameExpected = {"DEN", "DENCOR", "DTC", "GR"};
  ASSERT_EQ(calibrationTargetVarsUserNameExpected.size(), wellData.calibrationTargetVarsUserName_.size());
  for (unsigned int i = 0; i < calibrationTargetVarsUserNameExpected.size(); i++)
  {
    EXPECT_EQ(calibrationTargetVarsUserNameExpected[i], wellData.calibrationTargetVarsUserName_[i]);
  }

  QVector<QString> unitsExpected = {"g/cm3", "g/cm3", "us/ft", "API"};
  ASSERT_EQ(unitsExpected.size(), wellData.units_.size());
  for (unsigned int i = 0; i < unitsExpected.size(); i++)
  {
    EXPECT_EQ(unitsExpected[i].toStdString(), wellData.units_[i].toStdString());
  }
}

TEST(LasCurveInfoSectionReaderTest, testReadWithoutSpacesBetweenPropertyAndUnit)
{
  std::vector<std::string> section = {"~CURVE INFORMATION",
                                        "DEPT.m                              : 1      DEPTH",
                                        "DEN.g/cm3                          : 2      Rho",
                                        "DENCOR.g/cm3                          : 3      Rho",
                                        "TVD.m                                   : 4 TVD",
                                        "DTC.us/ft                          : 5      p_Sonic",
                                        "GR.API                            : 6      GR"
                                      };
  WellData wellData;
  ImportOptions options;
  options.depthUserPropertyName = "TVD";
  LASCurveInfoSectionReader reader(section, wellData, options);
  reader.readSection();


  EXPECT_EQ(options.depthColumn, 3);
  QVector<QString> calibrationTargetVarsUserNameExpected = {"DEPT", "DEN", "DENCOR", "DTC", "GR"};
  ASSERT_EQ(calibrationTargetVarsUserNameExpected.size(), wellData.calibrationTargetVarsUserName_.size());
  for (unsigned int i = 0; i < calibrationTargetVarsUserNameExpected.size(); i++)
  {
    EXPECT_EQ(calibrationTargetVarsUserNameExpected[i], wellData.calibrationTargetVarsUserName_[i]);
  }

  QVector<QString> unitsExpected = {"m", "g/cm3", "g/cm3", "us/ft", "API"};
  ASSERT_EQ(unitsExpected.size(), wellData.units_.size());
  for (unsigned int i = 0; i < unitsExpected.size(); i++)
  {
    EXPECT_EQ(unitsExpected[i].toStdString(), wellData.units_[i].toStdString());
  }
}

TEST(LasCurveInfoSectionReaderTest, testReadWithMissingUnitThrows)
{
  std::vector<std::string> section = {"~CURVE INFORMATION",
                                        "DEPT.                              : 1      DEPTH",
                                        "DEN.g/cm3                          : 2      Rho"
                                      };
  WellData wellData;
  ImportOptions options;
  LASCurveInfoSectionReader reader(section, wellData, options);

  EXPECT_THROW(reader.readSection(), std::runtime_error);

  try
  {
    reader.readSection();
  }
  catch (std::runtime_error e)
  {
    std::string message = e.what();
    EXPECT_EQ(message, "Invalid line in the Curve Info Section (~C).");
  }
}

TEST(LasCurveInfoSectionReaderTest, testReadWithMissingPropertyNameThrows)
{
  std::vector<std::string> section = {"~CURVE INFORMATION",
                                        "DEPT.m                             : 1      DEPTH",
                                        ".g/cm3                          : 2      Rho",
                                      };
  WellData wellData;
  ImportOptions options;

  LASCurveInfoSectionReader reader(section, wellData, options);
  EXPECT_THROW(reader.readSection(), std::runtime_error);

  try
  {
    reader.readSection();
  }
  catch (std::runtime_error e)
  {
    std::string message = e.what();
    EXPECT_EQ(message, "Invalid line in the Curve Info Section (~C).");
  }
}

TEST(LasCurveInfoSectionReaderTest, testReadWithInvalidLineThrows)
{
  std::vector<std::string> section = {"~CURVE INFORMATION",
                                        "DEPT.m                             : 1      DEPTH",
                                        "DEN.g/cm3                 ",
                                      };
  WellData wellData;
  ImportOptions options;
  LASCurveInfoSectionReader reader(section, wellData, options);
  EXPECT_THROW(reader.readSection(), std::runtime_error);

  try
  {
    reader.readSection();
  }
  catch (std::runtime_error e)
  {
    std::string message = e.what();
    EXPECT_EQ(message, "Invalid line in the Curve Info Section (~C).");
  }
}

TEST(LasCurveInfoSectionReaderTest, testReadWithoutCurveInfoThrows)
{
  std::vector<std::string> section = {"~CURVE INFORMATION"
                                      };
  WellData wellData;
  ImportOptions options;
  LASCurveInfoSectionReader reader(section, wellData, options);
  EXPECT_THROW(reader.readSection(), std::runtime_error);

  try
  {
    reader.readSection();
  }
  catch (std::runtime_error e)
  {
    std::string message = e.what();
    EXPECT_EQ(message, "No data was found in the curve information section (~C).");
  }
}
