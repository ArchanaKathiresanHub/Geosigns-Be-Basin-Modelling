//
// Copyright (C) 2012-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "../src/wellReader.h"

#include <gtest/gtest.h>

TEST(WellReaderTest, readWells)
{
  const double tol = 1.0;
  const char* wellFile = "wellData.in";

  std::vector<double> xWells;
  std::vector<double> yWells;
  WellReader::readWells( wellFile, xWells, yWells );

  std::vector<double> xWells_expected{186943.0, 189100.0, 179050.0, 200056.0, 189400.0};
  std::vector<double> yWells_expected{612333.0, 615467.1, 609000.0, 609000.0, 615800.0};

  for ( unsigned int i = 0; i<xWells.size(); ++i)
  {
    EXPECT_NEAR(xWells[i], xWells_expected[i], tol);
    EXPECT_NEAR(yWells[i], yWells_expected[i], tol);
  }
}
