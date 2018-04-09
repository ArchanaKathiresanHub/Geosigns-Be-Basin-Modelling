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

#include "MudstoneToMultipointPermeability.h"

//std library
#include <cmath>

static constexpr double permAnisotropy = 0.1;
static constexpr double depositionalPerm = 0.01;
static constexpr double mudPermSensitivityCoeff = 1.5;
static constexpr double compactionCoeff = 7.27e-02;
static constexpr double depositionalPoro = 70;
static constexpr double minPoro = 2.5;

using namespace Prograde;

TEST( MudstoneToMultipointPermeability, compute_noChemicalCompaction )
{
   const std::vector<double> expectedModelPrms = {permAnisotropy};
   const std::vector<double> expectedmpPor = { 1.8702049651098986e-30,
                                               0.048727839563193147,
                                               0.10081130690584807,
                                               0.20856495365211536,
                                               0.89269994897878602,
                                               3.8209353247141036,
                                               33.83498214748024,
                                               52.336563727350345,
                                               56.283152320002038,
                                               60.52734473702381,
                                               65.091582647771489,
                                               depositionalPoro };
   const std::vector<double> expectedmpPerm = { -6.500651116218978,
                                                -5.0064820606739637,
                                                -4.9385620884816408,
                                                -4.8627275283179747,
                                                -4.6779947525161507,
                                                -4.4191757850796032,
                                                -3.5620890277373376,
                                                -3.0484550065040281,
                                                -2.9030899869919438,
                                                -2.7156818820794935,
                                                -2.4515449934959719,
                                                std::log10(depositionalPerm) };

   std::vector<double> modelPrms; // perm. model parameters
   std::vector<double> mpPor;     // multi-point perm. model (porosity values)
   std::vector<double> mpPerm;    // multi-point perm. model (log of perm. values)

   const bool chemicalCompaction = false;
   MudstoneToMultipointPermeability converter({permAnisotropy,
                                               depositionalPerm,
                                               mudPermSensitivityCoeff,
                                               depositionalPoro,
                                               compactionCoeff,
                                               minPoro,
                                               chemicalCompaction});
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

TEST( MudstoneToMultipointPermeability, compute_ChemicalCompaction )
{
   const std::vector<double> expectedModelPrms = {permAnisotropy};
   const std::vector<double> expectedmpPor = { minPoro,
                                               2.5469875595787932,
                                               2.5972109030877819,
                                               2.7011162053073972,
                                               3.3608178079438291,
                                               6.1844733488314567,
                                               35.126589927927377,
                                               52.967400737087836,
                                               56.773039737144821,
                                               60.865653853558676,
                                               65.266883267493938,
                                               depositionalPoro };
   const std::vector<double> expectedmpPerm = { -6.500651116218978,
                                                -5.0064820606739637,
                                                -4.9385620884816408,
                                                -4.8627275283179747,
                                                -4.6779947525161507,
                                                -4.4191757850796032,
                                                -3.5620890277373376,
                                                -3.0484550065040281,
                                                -2.9030899869919438,
                                                -2.7156818820794935,
                                                -2.4515449934959719,
                                                std::log10(depositionalPerm) };

   std::vector<double> modelPrms; // perm. model parameters
   std::vector<double> mpPor;     // multi-point perm. model (porosity values)
   std::vector<double> mpPerm;    // multi-point perm. model (log of perm. values)

   const bool chemicalCompaction = true;
   MudstoneToMultipointPermeability converter({permAnisotropy,
                                               depositionalPerm,
                                               mudPermSensitivityCoeff,
                                               depositionalPoro,
                                               compactionCoeff,
                                               minPoro,
                                               chemicalCompaction});
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