//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/functions/cauldronPropertyUnits.h"

#include <gtest/gtest.h>

TEST(cauldronPropertyUnitsTest, testgetUnits)
{
  EXPECT_EQ(casaWizard::functions::getUnit("TwoWayTime"), "ms");
  EXPECT_EQ(casaWizard::functions::getUnit("Velocity"), "m/s");
  EXPECT_EQ(casaWizard::functions::getUnit("BulkDensity"), "kg/m3");
  EXPECT_EQ(casaWizard::functions::getUnit("GammaRay"), "API");
  EXPECT_EQ(casaWizard::functions::getUnit("Temperature"), "C");
  EXPECT_EQ(casaWizard::functions::getUnit("Pressure"), "MPa");
  EXPECT_EQ(casaWizard::functions::getUnit("VRe"), "%");
  EXPECT_EQ(casaWizard::functions::getUnit("SonicSlowness"), "us/m");
}


TEST(cauldronPropertyUnitsTest, testgetUnitsOfNonExistingProperty)
{
  EXPECT_EQ(casaWizard::functions::getUnit("NonExistent"), "");
}
