//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <gtest/gtest.h>
#include "model/predictionTargetDepth.h"

using namespace casaWizard::ua;

TEST( TestPredictionTargetDepth, TestName )
{
   const PredictionTargetDepth depthTarget({"Temperature","VRe"},23,46,20,100,"locName");
   const QString name = depthTarget.name("Temperature");
   EXPECT_EQ(name,"Temperature (23, 46, 20, 100.0)");
   const QString nameWithoutAge = depthTarget.nameWithoutAge();
   EXPECT_EQ(nameWithoutAge,"|Temperature|VRe (23, 46, 20)");
}
