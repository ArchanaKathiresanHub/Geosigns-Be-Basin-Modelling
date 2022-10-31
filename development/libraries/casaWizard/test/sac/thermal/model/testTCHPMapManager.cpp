//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/ThermalMapManager.h"

#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

#include "stubProjectReader.h"
#include "stubmapreader.h"

#include "expectFileEq.h"

#include <gtest/gtest.h>

using namespace casaWizard::sac::thermal;
using namespace casaWizard;

TEST(TCHPMapManager, testExportOptimizedTCHPMapToZycor)
{
  std::remove("Interpolated_TopCrustHeatProd.zyc");

  ThermalMapManager mapsManager;
  StubProjectReader projectReader;
  StubMapReader mapReader;
  mapsManager.exportOptimizedMapsToZycor(projectReader, mapReader, ".");

  expectFileEq("Interpolated_TopCrustHeatProdExpected.zyc",
               "Interpolated_TopCrustHeatProd.zyc");
}

