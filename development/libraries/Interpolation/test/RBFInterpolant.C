//
// Copyright (C) 2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "RBFInterpolant.h"
#include "RBFCubic.h"
#include "RBFGaussian.h"
#include "RBFThinPlateSpline.h"
#include <gtest/gtest.h>

namespace
{
   const static int s_degree = 5;
   const static double s_x = 17.0;
   const static int s_size = 3;
}

TEST( RBFInterpolant, DefaultConstructor )
{
   const Numerics::RBFInterpolant<Numerics::RBFCubic> interp;
   EXPECT_EQ( interp.getPolynomialDegree(), Numerics::NoPolynomial );
   EXPECT_TRUE( interp.getInterpolationPoints().empty() );
   EXPECT_TRUE( interp.getCoefficients().dimension() == -1 ); // Don't know why Numerics::Vector default size is -1...
}

TEST( RBFInterpolant, InputConstructor1 )
{
   const Numerics::RBFInterpolant<Numerics::RBFCubic> interp(s_degree);
   EXPECT_EQ( interp.getPolynomialDegree(), s_degree );
   EXPECT_TRUE( interp.getInterpolationPoints().empty() );
   EXPECT_TRUE( interp.getCoefficients().dimension() == -1 ); // Don't know why Numerics::Vector default size is -1...
}

TEST( RBFInterpolant, InputConstructor2 )
{
   Numerics::RBFCubic rbf2;
   const Numerics::RBFInterpolant<Numerics::RBFCubic> interp(rbf2,s_degree);
   EXPECT_EQ( interp.getPolynomialDegree(), s_degree );
   EXPECT_TRUE( interp.getInterpolationPoints().empty() );
   EXPECT_TRUE( interp.getCoefficients().dimension() == -1 ); // Don't know why Numerics::Vector default size is -1...
}

TEST( RBFInterpolant, InputConstructor3 )
{
   Numerics::RBFCubic rbf;
   std::vector<Numerics::Point> points(s_size,Numerics::Point(s_x,2*s_x,3*s_x));
   const Numerics::RBFInterpolant<Numerics::RBFCubic> interp(rbf,points,s_degree);
   EXPECT_EQ( interp.getPolynomialDegree(), s_degree );
   EXPECT_EQ( interp.getInterpolationPoints().size(), s_size );
   for( auto const & p : interp.getInterpolationPoints() )
   {
      EXPECT_DOUBLE_EQ( p.x(), s_x );
      EXPECT_DOUBLE_EQ( p.y(), 2*s_x );
      EXPECT_DOUBLE_EQ( p.z(), 3*s_x );
   }
   const Numerics::Vector & coeffs = interp.getCoefficients();
   EXPECT_TRUE( coeffs.dimension() == 3 );
}

TEST( RBFInterpolant, setPolynomialDegree )
{
   Numerics::RBFInterpolant<Numerics::RBFCubic> interp(s_degree);
   EXPECT_EQ( interp.getPolynomialDegree(), s_degree );
   interp.setPolynomialDegree(s_degree+1);
   EXPECT_EQ( interp.getPolynomialDegree(), s_degree+1 );
}

TEST( RBFInterpolant, setInterpolationPoints )
{
   Numerics::RBFInterpolant<Numerics::RBFCubic> interp(s_degree);
   EXPECT_TRUE( interp.getInterpolationPoints().empty() );
   std::vector<Numerics::Point> points(s_size,Numerics::Point(s_x,2*s_x,3*s_x));
   interp.setInterpolationPoints(points);
   EXPECT_EQ( interp.getInterpolationPoints().size(), s_size );
   for( auto const & p : interp.getInterpolationPoints() )
   {
      EXPECT_DOUBLE_EQ( p.x(), s_x );
      EXPECT_DOUBLE_EQ( p.y(), 2*s_x );
      EXPECT_DOUBLE_EQ( p.z(), 3*s_x );
   }
}

TEST( RBFInterpolant, setCoefficients )
{
   Numerics::RBFInterpolant<Numerics::RBFCubic> interp(s_degree);
   EXPECT_EQ( interp.getCoefficients().dimension(), -1 );
   Numerics::Vector coeffs(s_size);
   coeffs.fill(s_x);
   interp.setCoefficients(coeffs);
   EXPECT_EQ( interp.getCoefficients().dimension(), s_size );
   for( int i = 0; i < interp.getCoefficients().dimension(); ++i )
   {
      EXPECT_DOUBLE_EQ( interp.getCoefficients()(i), s_x );
   }
}
