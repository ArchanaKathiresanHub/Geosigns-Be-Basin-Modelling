//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/oneDModelDataExtractor.h"

#include "stubProjectReader.h"
#include "model/casaScenario.h"

#include <gtest/gtest.h>

using namespace casaWizard;

TEST(OneDModelDataExtractorTest, testExtraction)
{
  CasaScenario scenario(new casaWizard::StubProjectReader);
  scenario.setProject3dFilePath("./Project.project3d");
  scenario.setWorkingDirectory(".");
  CalibrationTargetManager& calibrationTargetManager = scenario.calibrationTargetManagerWellPrep();
  calibrationTargetManager.addWell("10_AML2_AV", 184550, 608300);
  calibrationTargetManager.addWell("11_AMN1_AV", 192000, 615000);
  calibrationTargetManager.setWellIsActive(false, 1);

  OneDModelDataExtractor extractor(scenario);

  const auto data = extractor.extract("SonicSlowness");

  EXPECT_EQ(data.size(), 1); // data is only extracted for active wells
  EXPECT_EQ(data.at("10_AML2_AV").first.size(), 42); // scenario has 42 nodes in z-direction
  std::vector<double> sonicSlownessData = data.at("10_AML2_AV").second;

  for (const double sonicSlowness : sonicSlownessData)
  {
    EXPECT_TRUE(sonicSlowness > 0);
  }

}

