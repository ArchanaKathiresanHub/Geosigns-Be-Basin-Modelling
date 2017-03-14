//
// Copyright (C) 2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "GeometryVector.h"
#include <gtest/gtest.h>

TEST( GeometryVector, AccessOperator )
{
   Numerics::GeometryVector gv;
   gv(0) = 1.;
   gv(1) = 2.;
   gv(2) = 3.;
   EXPECT_DOUBLE_EQ( gv(0), 1. );
   EXPECT_DOUBLE_EQ( gv(1), 2. );
   EXPECT_DOUBLE_EQ( gv(2), 3. );
}

TEST( Point, AccessOperatorFail )
{
   Numerics::GeometryVector gv0;
   try
   {
      const double v = gv0(Numerics::GeometryVector::DIMENSION);
      (void) v;
      FAIL() << "Expected std::runtime_error";
   }
   catch(std::runtime_error & ex)
   {
      EXPECT_EQ(ex.what(),std::string("Out of bounds access"));
   }
   catch(...)
   {
      FAIL() << "Expected std::runtime_error";
   }
   
   Numerics::GeometryVector gv1;
   try
   {
      ++gv1(Numerics::GeometryVector::DIMENSION);
      FAIL() << "Expected std::runtime_error";
   }
   catch(std::runtime_error & ex)
   {
      EXPECT_EQ(ex.what(),std::string("Out of bounds access"));
   }
   catch(...)
   {
      FAIL() << "Expected std::runtime_error";
   }
}

TEST( GeometryVector, Data )
{
   Numerics::GeometryVector gv;
   gv(0) = 1.;
   gv(1) = 2.;
   gv(2) = 3.;
   const Numerics::FloatingPoint * d = gv.data();
   EXPECT_DOUBLE_EQ( d[0], 1. );
   EXPECT_DOUBLE_EQ( d[1], 2. );
   EXPECT_DOUBLE_EQ( d[2], 3. );
}
