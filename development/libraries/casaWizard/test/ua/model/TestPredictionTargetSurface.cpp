//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <gtest/gtest.h>
#include "model/predictionTargetSurface.h"

using namespace casaWizard::ua;

TEST( TestPredictionTargetSurface, TestStringOutput )
{
   //const QVector<QString>& properties, const double x, const double y, const QString& surfaceName, const double age, const ToDepthConverter* todepthConverter = nullptr, const QString& locationName = "");
   PredictionTargetSurface surfaceTarget({"Temperature","VRe"},23,46,"some surface name",100);
   surfaceTarget.setSurfaceAndLayerName("some surface name","some layer name");
   const QString id = surfaceTarget.identifier("Temperature");
   EXPECT_EQ(id,"XYPointLayerTopSurface_Temperature_23_46_some_layer_name_100.0_1.0_1.0");

   const QString casaCommand = surfaceTarget.casaCommand("VRe");
   EXPECT_EQ(casaCommand,"XYPointLayerTopSurface Vr 23 46 \"some layer name\" 100.0 1.0 1.0");
}
