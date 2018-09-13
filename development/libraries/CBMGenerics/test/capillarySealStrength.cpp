//
// Copyright (C) 2012-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "../src/capillarySealStrength.h"
#include <gtest/gtest.h>


using namespace CBMGenerics;
using namespace capillarySealStrength;


/// Testing the 11th order polynomial fit to IFT

const double relativeError = 1.0e-10;

/// Test for exact values to make sure the code doesn't change
TEST( capillarySealStrength, ExactValues )
{
   double capTension = capTension_H2O_HC(1000, 640, 500, 600);
   EXPECT_NEAR (capTension, 33.408653240628, relativeError);

   capTension = capTension_H2O_HC(1100, 495, 300, 700);
   EXPECT_NEAR (capTension, 189.225323845686, relativeError);

   capTension = capTension_H2O_HC(1203, 467, 708, 453);
   EXPECT_NEAR (capTension, 46.5172999582344, relativeError);

   capTension = capTension_H2O_HC(998, 997, 420, 530);
   EXPECT_NEAR (capTension, 18.8396618913307, relativeError);

   capTension = capTension_H2O_HC(1111, 120, 550, 640);
   EXPECT_NEAR (capTension, 150.345507114949, relativeError);

   capTension = capTension_H2O_HC(1050, 140, 520, 460);
   EXPECT_NEAR (capTension, 85.5270198837387, relativeError);

   capTension = capTension_H2O_HC(1020, 101, 400, 400);
   EXPECT_NEAR (capTension, 101.574505737616, relativeError);

   capTension = capTension_H2O_HC(1103, 230, 509, 443);
   EXPECT_NEAR (capTension, 77.9969387429366, relativeError);
}

/// For various values of rho_HC, make sure that as Tc drops, capSealStrength drops too
TEST( capillarySealStrength, Monotonic_const_Tc )
{
   const double rhoWater     = 1200.0;
   double rhoHC              = 1190.0;
   double T                  = 300.0;
   double Tc                 = 1000.0;
   double newCapTension      = 0.0;
   double previousCapTension = 1.0e10; // sufficiently large number for the first (meaningless) comparison

   for (; rhoHC > 0.0; rhoHC -= 30)
   {
      for (; Tc > 0.0; Tc -= 30 )
      {
         newCapTension = capTension_H2O_HC (rhoWater, rhoHC, T, Tc);
         EXPECT_GE (previousCapTension, newCapTension);
         previousCapTension = newCapTension;
      }
   }
}

/// For a few different negative density differences make sure that the IFT is numeric_limits<double>::max();
TEST( capillarySealStrength, NegativeDensityDifferences )
{
   double capTension = capTension_H2O_HC(1030, 1640, 500, 600);
   EXPECT_NEAR (capTension, std::numeric_limits<double>::max(), relativeError);

   capTension = capTension_H2O_HC(1200, 1295, 300, 700);
   EXPECT_NEAR (capTension, std::numeric_limits<double>::max(), relativeError);

   capTension = capTension_H2O_HC(1203, 1267, 708, 453);
   EXPECT_NEAR (capTension, std::numeric_limits<double>::max(), relativeError);

   capTension = capTension_H2O_HC(998, 999, 420, 530);
   EXPECT_NEAR (capTension, std::numeric_limits<double>::max(), relativeError);

   capTension = capTension_H2O_HC(1111, 1120, 550, 640);
   EXPECT_NEAR (capTension, std::numeric_limits<double>::max(), relativeError);

   capTension = capTension_H2O_HC(1040, 1060, 520, 460);
   EXPECT_NEAR (capTension, std::numeric_limits<double>::max(), relativeError);

   capTension = capTension_H2O_HC(1020, 1030, 400, 400);
   EXPECT_NEAR (capTension, std::numeric_limits<double>::max(), relativeError);

   capTension = capTension_H2O_HC(1103, 1230, 509, 443);
   EXPECT_NEAR (capTension, std::numeric_limits<double>::max(), relativeError);
}
