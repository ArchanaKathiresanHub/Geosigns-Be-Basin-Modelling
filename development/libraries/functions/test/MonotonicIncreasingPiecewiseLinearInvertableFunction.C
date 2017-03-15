//
// Copyright (C) 2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "MonotonicIncreasingPiecewiseLinearInvertableFunction.h"
#include <vector>
#include <gtest/gtest.h>

namespace
{
    const static int s_size = 2;
    const static double s_double = 23.;
    
   struct CompareX
   {
      double m_x;

      CompareX(const double& x):
         m_x(x)
      {}

      bool operator<(const functions::MonotonicIncreasingPiecewiseLinearInvertableFunction::element_type& xy) const
      {
         return m_x < xy[0];
      }
   };
}

TEST( MonotonicIncreasingPiecewiseLinearInvertableFunction, ConstructorAndAccess )
{
   std::vector< functions::Tuple2<double> > v( s_size, functions::Tuple2<double>(s_double,2.*s_double) );
   ++v[1][0];
   ++v[1][1];
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   EXPECT_EQ( f.size(), s_size-1 );

   EXPECT_DOUBLE_EQ( f.piece(0)[0][0], s_double );
   EXPECT_DOUBLE_EQ( f.piece(0)[0][1], 2.*s_double );
   EXPECT_DOUBLE_EQ( f.piece(0)[1][0], s_double+1 );
   EXPECT_DOUBLE_EQ( f.piece(0)[1][1], 2.*s_double+1 );
   
   EXPECT_DOUBLE_EQ( f.begin(0)[0], s_double );
   EXPECT_DOUBLE_EQ( f.begin(0)[1], 2.*s_double );
   EXPECT_DOUBLE_EQ( f.end(0)[0],   s_double+1 );
   EXPECT_DOUBLE_EQ( f.end(0)[1],   2.*s_double+1 );
}

TEST( MonotonicIncreasingPiecewiseLinearInvertableFunction, AccessOperator )
{
   std::vector< functions::Tuple2<double> > v( s_size, functions::Tuple2<double>(s_double,s_double) );
   v[1][0] += 1.;
   v[1][1] *= 2;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   
   // Lower than the X of the first element
   EXPECT_DOUBLE_EQ( f(s_double-2.), v[0][1] );
   // First element
   EXPECT_DOUBLE_EQ( f(s_double),    v[0][1] );
   // Last element
   EXPECT_DOUBLE_EQ( f(s_double+1.), v[1][1] );
   // Greater than X of the last element
   EXPECT_DOUBLE_EQ( f(s_double+2.), v[1][1] );
   // Average of first and last
   const double val = s_double+.5;
   EXPECT_DOUBLE_EQ( f(val), ((v[1][1]-v[0][1]) / (v[1][0]-v[0][0])) * (val-v[0][0]) + v[0][1] );
}

TEST( MonotonicIncreasingPiecewiseLinearInvertableFunction, Invert )
{
   std::vector< functions::Tuple2<double> > v( s_size, functions::Tuple2<double>(s_double,s_double) );
   v[1][0] += 1.;
   v[1][1] *= 2;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   
   // Lower than the Y of the first element
   EXPECT_DOUBLE_EQ( f.invert(s_double-2.), v[0][0] );
   // First element
   EXPECT_DOUBLE_EQ( f.invert(s_double),    v[0][0] );
   // Last element
   EXPECT_DOUBLE_EQ( f.invert(2.*s_double), v[1][0] );
   // Greater than the Y of the last element
   EXPECT_DOUBLE_EQ( f.invert(3.*s_double), v[1][0] );
   // Average of first and last
   const double val = 1.5 * s_double;
   EXPECT_DOUBLE_EQ( f.invert(val), s_double + 0.5 );
}

TEST( MonotonicIncreasingPiecewiseLinearInvertableFunction, OperatorPlusEq1 )
{
   std::vector< functions::Tuple2<double> > v( s_size, functions::Tuple2<double>(s_double,2.*s_double) );
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   const double val = 2.;
   f += functions::Tuple2<double>(val,val);

   EXPECT_DOUBLE_EQ( f.begin(0)[0], v[0][0]+val );
   EXPECT_DOUBLE_EQ( f.begin(0)[1], v[0][1]+val );
   EXPECT_DOUBLE_EQ( f.end(0)[0],   v[1][0]+val );
   EXPECT_DOUBLE_EQ( f.end(0)[1],   v[1][1]+val );
}

TEST( MonotonicIncreasingPiecewiseLinearInvertableFunction, OperatorPlusEq2 )
{
   std::vector< functions::Tuple2<double> > v( s_size, functions::Tuple2<double>(s_double,2.*s_double) );
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   const double val = 2.;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f2 = f + functions::Tuple2<double>(val,val);

   EXPECT_DOUBLE_EQ( f2.begin(0)[0], v[0][0]+val );
   EXPECT_DOUBLE_EQ( f2.begin(0)[1], v[0][1]+val );
   EXPECT_DOUBLE_EQ( f2.end(0)[0],   v[1][0]+val );
   EXPECT_DOUBLE_EQ( f2.end(0)[1],   v[1][1]+val );
}

TEST( MonotonicIncreasingPiecewiseLinearInvertableFunction, OperatorMinusEq )
{
   std::vector< functions::Tuple2<double> > v( s_size, functions::Tuple2<double>(s_double,2.*s_double) );
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   const double val = 2.;
   f -= functions::Tuple2<double>(val,val);

   EXPECT_DOUBLE_EQ( f.begin(0)[0], v[0][0]-val );
   EXPECT_DOUBLE_EQ( f.begin(0)[1], v[0][1]-val );
   EXPECT_DOUBLE_EQ( f.end(0)[0],   v[1][0]-val );
   EXPECT_DOUBLE_EQ( f.end(0)[1],   v[1][1]-val );
}

TEST( MonotonicIncreasingPiecewiseLinearInvertableFunction, OperatorMinus )
{
   std::vector< functions::Tuple2<double> > v( s_size, functions::Tuple2<double>(s_double,2.*s_double) );
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   const double val = 2.;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f2 = f - functions::Tuple2<double>(val,val);

   EXPECT_DOUBLE_EQ( f2.begin(0)[0], v[0][0]-val );
   EXPECT_DOUBLE_EQ( f2.begin(0)[1], v[0][1]-val );
   EXPECT_DOUBLE_EQ( f2.end(0)[0],   v[1][0]-val );
   EXPECT_DOUBLE_EQ( f2.end(0)[1],   v[1][1]-val );
}

TEST( MonotonicIncreasingPiecewiseLinearInvertableFunction, OperatorProdEq )
{
   std::vector< functions::Tuple2<double> > v( s_size, functions::Tuple2<double>(s_double,2.*s_double) );
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   const double val = 2.;
   f *= functions::Tuple2<double>(val,val);

   EXPECT_DOUBLE_EQ( f.begin(0)[0], val*v[0][0] );
   EXPECT_DOUBLE_EQ( f.begin(0)[1], val*v[0][1] );
   EXPECT_DOUBLE_EQ( f.end(0)[0],   val*v[1][0] );
   EXPECT_DOUBLE_EQ( f.end(0)[1],   val*v[1][1] );
}

TEST( MonotonicIncreasingPiecewiseLinearInvertableFunction, OperatorProd )
{
   std::vector< functions::Tuple2<double> > v( s_size, functions::Tuple2<double>(s_double,2.*s_double) );
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   const double val = 2.;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f2 = f * functions::Tuple2<double>(val,val);

   EXPECT_DOUBLE_EQ( f2.begin(0)[0], val*v[0][0] );
   EXPECT_DOUBLE_EQ( f2.begin(0)[1], val*v[0][1] );
   EXPECT_DOUBLE_EQ( f2.end(0)[0],   val*v[1][0] );
   EXPECT_DOUBLE_EQ( f2.end(0)[1],   val*v[1][1] );
}

TEST( MonotonicIncreasingPiecewiseLinearInvertableFunction, OperatorDivEq )
{
   std::vector< functions::Tuple2<double> > v( s_size, functions::Tuple2<double>(s_double,2.*s_double) );
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   const double val = 2.;
   f /= functions::Tuple2<double>(val,val);

   EXPECT_DOUBLE_EQ( f.begin(0)[0], v[0][0]/val );
   EXPECT_DOUBLE_EQ( f.begin(0)[1], v[0][1]/val );
   EXPECT_DOUBLE_EQ( f.end(0)[0],   v[1][0]/val );
   EXPECT_DOUBLE_EQ( f.end(0)[1],   v[1][1]/val );
}

TEST( MonotonicIncreasingPiecewiseLinearInvertableFunction, OperatorDiv )
{
   std::vector< functions::Tuple2<double> > v( s_size, functions::Tuple2<double>(s_double,2.*s_double) );
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   const double val = 2.;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f2 = f / functions::Tuple2<double>(val,val);

   EXPECT_DOUBLE_EQ( f2.begin(0)[0], v[0][0]/val );
   EXPECT_DOUBLE_EQ( f2.begin(0)[1], v[0][1]/val );
   EXPECT_DOUBLE_EQ( f2.end(0)[0],   v[1][0]/val );
   EXPECT_DOUBLE_EQ( f2.end(0)[1],   v[1][1]/val );
}

TEST( MonotonicIncreasingPiecewiseLinearInvertableFunction, shiftXBy1 )
{
   std::vector< functions::Tuple2<double> > v( s_size, functions::Tuple2<double>(s_double,2.*s_double) );
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   const double val = 2.;
   f.shiftXBy(val);

   EXPECT_DOUBLE_EQ( f.begin(0)[0], v[0][0] + val );
   EXPECT_DOUBLE_EQ( f.begin(0)[1], v[0][1] );
   EXPECT_DOUBLE_EQ( f.end(0)[0],   v[1][0] + val );
   EXPECT_DOUBLE_EQ( f.end(0)[1],   v[1][1] );
}

TEST( MonotonicIncreasingPiecewiseLinearInvertableFunction, shiftXBy2 )
{
   std::vector< functions::Tuple2<double> > v( s_size, functions::Tuple2<double>(s_double,2.*s_double) );
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   const double val = 2.;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f2 = functions::shiftX( f, val );

   EXPECT_DOUBLE_EQ( f2.begin(0)[0], v[0][0] + val );
   EXPECT_DOUBLE_EQ( f2.begin(0)[1], v[0][1] );
   EXPECT_DOUBLE_EQ( f2.end(0)[0],   v[1][0] + val );
   EXPECT_DOUBLE_EQ( f2.end(0)[1],   v[1][1] );
}

TEST( MonotonicIncreasingPiecewiseLinearInvertableFunction, widenXBy1 )
{
   std::vector< functions::Tuple2<double> > v( s_size, functions::Tuple2<double>(s_double,2.*s_double) );
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   const double val = 2.;
   f.widenXBy(val);

   EXPECT_DOUBLE_EQ( f.begin(0)[0], val * v[0][0] );
   EXPECT_DOUBLE_EQ( f.begin(0)[1], v[0][1] );
   EXPECT_DOUBLE_EQ( f.end(0)[0],   val * v[1][0] );
   EXPECT_DOUBLE_EQ( f.end(0)[1],   v[1][1] );
}

TEST( MonotonicIncreasingPiecewiseLinearInvertableFunction, widenXBy2 )
{
   std::vector< functions::Tuple2<double> > v( s_size, functions::Tuple2<double>(s_double,2.*s_double) );
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   const double val = 2.;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f2 = functions::widenX( f, val );

   EXPECT_DOUBLE_EQ( f2.begin(0)[0], val * v[0][0] );
   EXPECT_DOUBLE_EQ( f2.begin(0)[1], v[0][1] );
   EXPECT_DOUBLE_EQ( f2.end(0)[0],   val * v[1][0] );
   EXPECT_DOUBLE_EQ( f2.end(0)[1],   v[1][1] );
}

TEST( MonotonicIncreasingPiecewiseLinearInvertableFunction, findIndex )
{
   std::vector< functions::Tuple2<double> > v( s_size, functions::Tuple2<double>(s_double,s_double) );
   v[1][0] += 1.;
   v[1][1] *= 2;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);

   // all entries are larger than pred
   EXPECT_EQ( f.findIndex(CompareX(0.)), -1 );
   // first entry
   EXPECT_EQ( f.findIndex(CompareX(s_double)), 0 );
   // between first and second entry
   EXPECT_EQ( f.findIndex(CompareX(s_double+.5)), 0 );
   // equal or larger than the last entry
   EXPECT_EQ( f.findIndex(CompareX(s_double+1.)), 2 );
   EXPECT_EQ( f.findIndex(CompareX(s_double+2.)), 2 );
}

#ifndef NDEBUG
TEST( MonotonicIncreasingPiecewiseLinearInvertableFunction, PieceAccessFail1 )
{
   std::vector< functions::Tuple2<double> > v( s_size );
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   ASSERT_DEATH( f.piece(-1), "" );
}

TEST( MonotonicIncreasingPiecewiseLinearInvertableFunction, PieceAccessFail2 )
{
   std::vector< functions::Tuple2<double> > v( s_size );
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   ASSERT_DEATH( f.piece(s_size-1), "" );
}

TEST( MonotonicIncreasingPiecewiseLinearInvertableFunction, BeginAccessFail1 )
{
   std::vector< functions::Tuple2<double> > v( s_size );
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   ASSERT_DEATH( f.begin(-1), "" );
}

TEST( MonotonicIncreasingPiecewiseLinearInvertableFunction, BeginAccessFail2 )
{
   std::vector< functions::Tuple2<double> > v( s_size );
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   ASSERT_DEATH( f.begin(s_size-1), "" );
}

TEST( MonotonicIncreasingPiecewiseLinearInvertableFunction, EndAccessFail1 )
{
   std::vector< functions::Tuple2<double> > v( s_size );
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   ASSERT_DEATH( f.end(-1), "" );
}

TEST( MonotonicIncreasingPiecewiseLinearInvertableFunction, EndAccessFail2 )
{
   std::vector< functions::Tuple2<double> > v( s_size );
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   ASSERT_DEATH( f.end(s_size-1), "" );
}

TEST( MonotonicIncreasingPiecewiseLinearInvertableFunction, OperatorProdEqFail )
{
   // Negative product factor are not allowed
   std::vector< functions::Tuple2<double> > v( s_size, functions::Tuple2<double>(s_double,2.*s_double) );
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   const double val = -2.;
   functions::Tuple2<double> t(val,val);
   ASSERT_DEATH( f*=t;, "" );
}

TEST( MonotonicIncreasingPiecewiseLinearInvertableFunction, OperatorDivEqFail1 )
{
   // Cannot divide by zero
   std::vector< functions::Tuple2<double> > v( s_size, functions::Tuple2<double>(s_double,2.*s_double) );
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   const double val = 0.;
   functions::Tuple2<double> t(val,val);
   ASSERT_DEATH( f/=t;, "" );
}

TEST( MonotonicIncreasingPiecewiseLinearInvertableFunction, OperatorDivEqFail2 )
{
   // Negative divide factor are not allowed
   std::vector< functions::Tuple2<double> > v( s_size, functions::Tuple2<double>(s_double,2.*s_double) );
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   const double val = -2.;
   functions::Tuple2<double> t(val,val);
   ASSERT_DEATH( f/=t;, "" );
}

TEST( MonotonicIncreasingPiecewiseLinearInvertableFunction, widenXByFail )
{
   // Negative widen factors are not allowed
   std::vector< functions::Tuple2<double> > v( s_size, functions::Tuple2<double>(s_double,2.*s_double) );
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   const double val = -2.;
   ASSERT_DEATH( f.widenXBy(val), "" );
}
#endif
