//
// Copyright (C) 2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "RBFCubic.h"
#include "RBFGaussian.h"
#include "RBFThinPlateSpline.h"
#include <gtest/gtest.h>

TEST( RBF, Cubic )
{
   const Numerics::RBFCubic rbf;
   EXPECT_DOUBLE_EQ( rbf(-1.), 0. );
   EXPECT_DOUBLE_EQ( rbf(0.), 0. );

   // Check it's increasing
   for(int i = 0; i < 100; ++i)
   {
      EXPECT_GT( rbf(static_cast<double>(i+1)), rbf(static_cast<double>(i)) );
   }

   // Exact values
   EXPECT_DOUBLE_EQ( rbf(1.), 1. );
   EXPECT_DOUBLE_EQ( rbf(4.), 8. );
   EXPECT_DOUBLE_EQ( rbf(9.), 27. );
}

TEST( RBF, Gaussian )
{
   Numerics::RBFGaussian rbf;
   rbf.setParameter(3.);

   // Check it's decreasing
   for(int i = -10; i <= 10; ++i)
   {
      EXPECT_GT( rbf(static_cast<double>(i)), rbf(static_cast<double>(i+1)) );
   }

   // Exact values
   EXPECT_DOUBLE_EQ( rbf(0.), 1. );
   EXPECT_NEAR( rbf(1.), 1.234098041E-04, 1.e-12);
   EXPECT_NEAR( rbf(2.), 1.522997974E-08, 1.e-17);
   EXPECT_NEAR( rbf(3.), 1.879528817E-12, 1.e-21);
}

TEST( RBF, ThinPlateSpline )
{
   const Numerics::RBFThinPlateSpline rbf;
   
   EXPECT_DOUBLE_EQ( rbf(0.), 0. );

   // Check it's increasing for values greater than 1
   for(int i = 1; i <= 100; ++i)
   {
      EXPECT_GT( rbf(static_cast<double>(i+1)), rbf(static_cast<double>(i)) );
   }

   // Exact values
   EXPECT_DOUBLE_EQ( rbf(1.), 0. );
   EXPECT_NEAR( rbf(2.), 6.93147180559945E-01, 1.e-14);
   EXPECT_NEAR( rbf(3.), 1.64791843300216E+00, 1.e-13);
}
