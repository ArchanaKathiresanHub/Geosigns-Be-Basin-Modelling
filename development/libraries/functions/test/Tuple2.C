//
// Copyright (C) 2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Tuple2.h"
#include <vector>
#include <gtest/gtest.h>

namespace
{
    const static int s_int = 8;
    const static double s_double = 23.;
}

TEST( Tuple2, DefaultCtr )
{
   functions::Tuple2<int> tInt;
   EXPECT_EQ( tInt[0], -std::numeric_limits<int>::max() );
   EXPECT_EQ( tInt[1], -std::numeric_limits<int>::max() );
   
   functions::Tuple2<double> tDouble;
   EXPECT_DOUBLE_EQ( tDouble[0], -std::numeric_limits<double>::max() );
   EXPECT_DOUBLE_EQ( tDouble[1], -std::numeric_limits<double>::max() );
}

TEST( Tuple2, InputCtr )
{
   functions::Tuple2<int> tInt( s_int, s_int+1 );
   EXPECT_EQ( tInt[0], s_int );
   EXPECT_EQ( tInt[1], s_int+1 );
   
   functions::Tuple2<double> tDouble( s_double, s_double+1 );
   EXPECT_DOUBLE_EQ( tDouble[0], s_double );
   EXPECT_DOUBLE_EQ( tDouble[1], s_double+1 );
}

TEST( Tuple2, ArrayCtr )
{
   std::vector<int> vInt(2,s_int);
   functions::Tuple2<int> tInt( vInt.data() );
   EXPECT_EQ( tInt[0], s_int );
   EXPECT_EQ( tInt[1], s_int );
   
   std::vector<double> vDouble(2,s_double);
   functions::Tuple2<double> tDouble( vDouble.data() );
   EXPECT_DOUBLE_EQ( tDouble[0], s_double );
   EXPECT_DOUBLE_EQ( tDouble[1], s_double );
}

TEST( Tuple2, ArrayCtrAssertFail )
{
   const int * vInt = nullptr;
   try
   {
      functions::Tuple2<int> t( vInt );
      FAIL() << "Failure was expected";
   }
   catch(...)
   {
      SUCCEED();
   }
}

#ifndef NDEBUG
TEST( Tuple2, AccessFail )
{
   functions::Tuple2<int> tInt;
   ASSERT_DEATH( tInt[5], "" );
}
#endif

TEST( Tuple2, reverse )
{
   functions::Tuple2<int> tInt( s_int, s_int+1 );
   tInt = tInt.reverse();
   EXPECT_EQ( tInt[0], s_int+1 );
   EXPECT_EQ( tInt[1], s_int );
}

TEST( Tuple2, operatorPlusEq )
{
   functions::Tuple2<int> tInt( s_int, s_int );
   tInt += functions::Tuple2<int>( s_int, s_int );
   EXPECT_EQ( tInt[0], 2*s_int );
   EXPECT_EQ( tInt[1], 2*s_int );
}

TEST( Tuple2, operatorMinusEq )
{
   functions::Tuple2<int> tInt( s_int, s_int );
   tInt -= functions::Tuple2<int>( s_int, s_int );
   EXPECT_EQ( tInt[0], 0 );
   EXPECT_EQ( tInt[1], 0 );
}

TEST( Tuple2, operatorProdEq )
{
   functions::Tuple2<int> tInt( s_int, s_int );
   tInt *= functions::Tuple2<int>( s_int, s_int );
   EXPECT_EQ( tInt[0], s_int*s_int );
   EXPECT_EQ( tInt[1], s_int*s_int );
}

TEST( Tuple2, operatorDivEq )
{
   functions::Tuple2<int> tInt( s_int, s_int );
   tInt /= functions::Tuple2<int>( s_int, s_int );
   EXPECT_EQ( tInt[0], 1 );
   EXPECT_EQ( tInt[1], 1 );
}
