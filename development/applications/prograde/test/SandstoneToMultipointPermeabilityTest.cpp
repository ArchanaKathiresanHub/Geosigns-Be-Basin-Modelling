//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <gtest/gtest.h>

#include "SandstoneToMultipointPermeability.h"

//std library
#include <cmath>

static constexpr double permAnisotropy = 0.1;
static constexpr double depositionalPerm = 6000;
static constexpr double permSandClayPercentage = 1.5;
static constexpr double depositionalPoro = 48;
static constexpr double minMechaPoro = 2.5;

using namespace Prograde;

TEST( SandstoneToMultipointPermeability, compute )
{
   const std::vector<double> expectedModelPrms = {permAnisotropy};
   const std::vector<double> expectedmpPor = { 0,
                                               minMechaPoro,
                                               depositionalPoro,
                                               100 };
   const std::vector<double> expectedmpPerm = { -3.0468487496163568,
                                                -3.0468487496163568,
                                                std::log10(depositionalPerm),
                                                std::log10(depositionalPerm) };

   std::vector<double> modelPrms; // perm. model parameters
   std::vector<double> mpPor;     // multi-point perm. model (porosity values)
   std::vector<double> mpPerm;    // multi-point perm. model (log of perm. values)

   SandstoneToMultipointPermeability converter({permAnisotropy,
                                                depositionalPerm,
                                                permSandClayPercentage,
                                                depositionalPoro,
                                                minMechaPoro});
   converter.compute(modelPrms, mpPor, mpPerm);

   EXPECT_EQ(expectedModelPrms.size(), modelPrms.size());
   EXPECT_EQ(expectedmpPor.size(), mpPor.size());
   EXPECT_EQ(expectedmpPerm.size(), mpPerm.size());
   EXPECT_EQ(mpPor.size(), mpPerm.size());

   for(size_t i = 0; i<modelPrms.size(); i++){
      EXPECT_DOUBLE_EQ(expectedModelPrms[i], modelPrms[i]);
   }

   for(size_t i = 0; i<mpPor.size(); i++){
      EXPECT_DOUBLE_EQ(expectedmpPor[i], mpPor[i]);
   }

   for(size_t i = 0; i<mpPerm.size(); i++){
      EXPECT_DOUBLE_EQ(expectedmpPerm[i], mpPerm[i]);
   }

}