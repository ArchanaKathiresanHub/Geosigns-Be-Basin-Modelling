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
#include "Point.h"
#include <gtest/gtest.h>

TEST( Point, DefaultCtr )
{
   const Numerics::Point p;
   EXPECT_DOUBLE_EQ( p.x(), 0. );
   EXPECT_DOUBLE_EQ( p.y(), 0. );
   EXPECT_DOUBLE_EQ( p.z(), 0. );
}

TEST( Point, CopyCtr )
{
   const Numerics::Point p1(1.,2.,3.);
   const Numerics::Point p2(p1);
   EXPECT_DOUBLE_EQ( p2.x(), 1. );
   EXPECT_DOUBLE_EQ( p2.y(), 2. );
   EXPECT_DOUBLE_EQ( p2.z(), 3. );
}

TEST( Point, InputCtr )
{
   const Numerics::Point p(1.,2.,3.);
   EXPECT_DOUBLE_EQ( p.x(), 1. );
   EXPECT_DOUBLE_EQ( p.y(), 2. );
   EXPECT_DOUBLE_EQ( p.z(), 3. );
}

TEST( Point, Zero )
{
   Numerics::Point p(1.,2.,3.);
   p.zero();
   EXPECT_DOUBLE_EQ( p.x(), 0. );
   EXPECT_DOUBLE_EQ( p.y(), 0. );
   EXPECT_DOUBLE_EQ( p.z(), 0. );
}

TEST( Point, AccessOperator )
{
   const Numerics::Point p0(1.,2.,3.);
   EXPECT_DOUBLE_EQ( p0(0), 1. );
   EXPECT_DOUBLE_EQ( p0(1), 2. );
   EXPECT_DOUBLE_EQ( p0(2), 3. );
   
   Numerics::Point p1(1.,2.,3.);
   ++p1(0);
   ++p1(1);
   ++p1(2);
   EXPECT_DOUBLE_EQ( p1(0), 2. );
   EXPECT_DOUBLE_EQ( p1(1), 3. );
   EXPECT_DOUBLE_EQ( p1(2), 4. );
}

#ifndef NDEBUG
TEST( Point, AccessOperatorFail )
{
   const Numerics::Point p0(1.,2.,3.);
   try
   {
      const double v = p0(Numerics::Point::DIMENSION);
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
   
   Numerics::Point p1(1.,2.,3.);
   try
   {
      ++p1(Numerics::Point::DIMENSION);
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
#endif

TEST( Point, AssignementOperatorPlus )
{
   Numerics::Point p0;
   Numerics::GeometryVector gv;
   gv(0) = 1.;
   gv(1) = 2.;
   gv(2) = 3.;
   p0 += gv;
   EXPECT_DOUBLE_EQ( p0(0), 1. );
   EXPECT_DOUBLE_EQ( p0(1), 2. );
   EXPECT_DOUBLE_EQ( p0(2), 3. );
}

TEST( Point, AssignementOperatorMinus )
{
   Numerics::Point p0;
   Numerics::GeometryVector gv;
   gv(0) = 1.;
   gv(1) = 2.;
   gv(2) = 3.;
   p0 -= gv;
   EXPECT_DOUBLE_EQ( p0(0), -1. );
   EXPECT_DOUBLE_EQ( p0(1), -2. );
   EXPECT_DOUBLE_EQ( p0(2), -3. );
}

TEST( Point, AssignementOperatorProduct1 )
{
   Numerics::Point p(1.,1.,1.);
   Numerics::GeometryVector gv;
   gv(0) = 1.;
   gv(1) = 2.;
   gv(2) = 3.;
   p *= gv;
   EXPECT_DOUBLE_EQ( p(0), 1. );
   EXPECT_DOUBLE_EQ( p(1), 2. );
   EXPECT_DOUBLE_EQ( p(2), 3. );
}

TEST( Point, AssignementOperatorProduct2 )
{
   Numerics::Point p(1.,1.,1.);
   p *= 7;
   EXPECT_DOUBLE_EQ( p(0), 7. );
   EXPECT_DOUBLE_EQ( p(1), 7. );
   EXPECT_DOUBLE_EQ( p(2), 7. );
}

TEST( Point, OperatorPlus )
{
   Numerics::Point p0;
   Numerics::Point p1(1.,1.,1.);
   Numerics::GeometryVector gv;
   gv(0) = 1.;
   gv(1) = 2.;
   gv(2) = 3.;
   p0 = p1 + gv;
   EXPECT_DOUBLE_EQ( p0(0), 2. );
   EXPECT_DOUBLE_EQ( p0(1), 3. );
   EXPECT_DOUBLE_EQ( p0(2), 4. );
}

TEST( Point, OperatorMinus )
{
   Numerics::Point p0;
   Numerics::Point p1(1.,1.,1.);
   Numerics::GeometryVector gv;
   gv(0) = 1.;
   gv(1) = 2.;
   gv(2) = 3.;
   p0 = p1 - gv;
   EXPECT_DOUBLE_EQ( p0(0), 0. );
   EXPECT_DOUBLE_EQ( p0(1), -1. );
   EXPECT_DOUBLE_EQ( p0(2), -2. );
}

TEST( Point, OperatorProduct )
{
   Numerics::Point p0;
   Numerics::Point p1(1.,1.,1.);
   Numerics::GeometryVector gv;
   gv(0) = 1.;
   gv(1) = 2.;
   gv(2) = 3.;
   p0 = gv * p1;
   EXPECT_DOUBLE_EQ( p0(0), 1. );
   EXPECT_DOUBLE_EQ( p0(1), 2. );
   EXPECT_DOUBLE_EQ( p0(2), 3. );
}

TEST( Point, separationDistanceSquared )
{
   Numerics::Point p0(0.,1.,2.);
   Numerics::Point p1(3.,5.,14.);
   EXPECT_DOUBLE_EQ( Numerics::separationDistanceSquared(p0,p1) , 169. );
   EXPECT_DOUBLE_EQ( Numerics::separationDistance(p0,p1) , 13. );
}
