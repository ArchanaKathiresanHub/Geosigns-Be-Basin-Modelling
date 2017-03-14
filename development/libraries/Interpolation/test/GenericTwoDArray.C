//
// Copyright (C) 2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "GenericTwoDArray.h"
#include <gtest/gtest.h>

static const int m = 3;
static const int n = 5;

TEST( GenericTwoDArray, DefaultCtr )
{
   const GenericTwoDArray<double> arr;
   EXPECT_TRUE( arr.data() == nullptr );
   EXPECT_EQ( arr.numberOfEntries(), 0 );
}

TEST( GenericTwoDArray, InputCtr1 )
{
   const GenericTwoDArray<double> arr(m,n);
   EXPECT_TRUE( arr.data() != nullptr );
   EXPECT_EQ( arr.numberOfEntries(), m*n );
   EXPECT_EQ( arr.length(1), m );
   EXPECT_EQ( arr.length(2), n );
   EXPECT_EQ( arr.first(1), 1 );
   EXPECT_EQ( arr.first(2), 1 );
   EXPECT_EQ( arr.last(1), m );
   EXPECT_EQ( arr.last(2), n );
}

TEST( GenericTwoDArray, InputCtr2 )
{
   const GenericTwoDArray<double> arr(1,m,1,n);
   EXPECT_TRUE( arr.data() != nullptr );
   EXPECT_EQ( arr.numberOfEntries(), m*n );
   EXPECT_EQ( arr.length(1), m );
   EXPECT_EQ( arr.length(2), n );
   EXPECT_EQ( arr.first(1), 1 );
   EXPECT_EQ( arr.first(2), 1 );
   EXPECT_EQ( arr.last(1), m );
   EXPECT_EQ( arr.last(2), n );
}

TEST( GenericTwoDArray, SetSize1 )
{
   GenericTwoDArray<double> arr;
   arr.setSize(m,n);
   EXPECT_TRUE( arr.data() != nullptr );
   EXPECT_EQ( arr.numberOfEntries(), m*n );
   EXPECT_EQ( arr.length(1), m );
   EXPECT_EQ( arr.length(2), n );
   EXPECT_EQ( arr.first(1), 1 );
   EXPECT_EQ( arr.first(2), 1 );
   EXPECT_EQ( arr.last(1), m );
   EXPECT_EQ( arr.last(2), n );
}

TEST( GenericTwoDArray, SetSize2 )
{
   GenericTwoDArray<double> arr;
   arr.setSize(1,m,1,n);
   EXPECT_TRUE( arr.data() != nullptr );
   EXPECT_EQ( arr.numberOfEntries(), m*n );
   EXPECT_EQ( arr.length(1), m );
   EXPECT_EQ( arr.length(2), n );
   EXPECT_EQ( arr.first(1), 1 );
   EXPECT_EQ( arr.first(2), 1 );
   EXPECT_EQ( arr.last(1), m );
   EXPECT_EQ( arr.last(2), n );
}

TEST( GenericTwoDArray, AccessOperator )
{
   GenericTwoDArray<double> arr(m,n);
   double v = 0;
   for(int i = 1; i <= m; ++i)
   {
      for(int j = 1; j <= n; ++j)
      {
         arr(i,j) = v;
         ++v;
      }
   }
   v = 0;
   for(int i = 1; i <= m; ++i)
   {
      for(int j = 1; j <= n; ++j)
      {
         EXPECT_DOUBLE_EQ( arr(i,j), v++);
      }
   }
   
   const GenericTwoDArray<double> arrCopy(arr);
   v = 0;
   for(int i = 1; i <= m; ++i)
   {
      for(int j = 1; j <= n; ++j)
      {
         EXPECT_DOUBLE_EQ( arr(i,j), v++);
      }
   }
}

TEST( GenericTwoDArray, Data )
{
   GenericTwoDArray<double> arr(m,n);
   double v = 0;
   for(int i = 1; i <= m; ++i)
   {
      for(int j = 1; j <= n; ++j)
      {
         arr(i,j) = v;
         ++v;
      }
   }

   double * dataConst = arr.data();
   v = 0;
   for(int i = 0; i < m*n; ++i)
   {
      EXPECT_DOUBLE_EQ( dataConst[i], v++);
   }
   
   const GenericTwoDArray<double> arrCopy(arr);
   const double * data = arrCopy.data();
   v = 0;
   for(int i = 0; i < m*n; ++i)
   {
      EXPECT_DOUBLE_EQ( data[i], v++);
   }
}

TEST( GenericTwoDArray, Fill )
{
   GenericTwoDArray<double> arr(m,n);
   arr.fill(23.);

   const double * dataConst = arr.data();
   for(int i = 0; i < m*n; ++i)
   {
      EXPECT_DOUBLE_EQ( dataConst[i],23.);
   }
}
