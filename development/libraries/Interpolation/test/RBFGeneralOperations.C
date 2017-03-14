//
// Copyright (C) 2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "RBFGeneralOperations.h"
#include <gtest/gtest.h>

TEST( RBFGeneralOperations, numberOfPolynomialTerms )
{
   // 1D
   EXPECT_EQ( Numerics::numberOfPolynomialTerms<1>(-1), 0 );
   for( int i = 0; i < 10; ++i )
   {
      EXPECT_EQ( Numerics::numberOfPolynomialTerms<1>(i), i+1 );
   }

   // 2D
   EXPECT_EQ( Numerics::numberOfPolynomialTerms<2>(-1),  0 );
   EXPECT_EQ( Numerics::numberOfPolynomialTerms<2>( 0),  1 );
   EXPECT_EQ( Numerics::numberOfPolynomialTerms<2>( 1),  3 );
   EXPECT_EQ( Numerics::numberOfPolynomialTerms<2>( 2),  6 );
   EXPECT_EQ( Numerics::numberOfPolynomialTerms<2>( 3), 10 );
   for( int i = 4; i < 10; ++i )
   {
      EXPECT_EQ( Numerics::numberOfPolynomialTerms<2>(i), 0 );
   }

   // 3D
   EXPECT_EQ( Numerics::numberOfPolynomialTerms<3>(-1),  0 );
   EXPECT_EQ( Numerics::numberOfPolynomialTerms<3>( 0),  1 );
   EXPECT_EQ( Numerics::numberOfPolynomialTerms<3>( 1),  4 );
   EXPECT_EQ( Numerics::numberOfPolynomialTerms<3>( 2), 10 );
   for( int i = 3; i < 10; ++i )
   {
      EXPECT_EQ( Numerics::numberOfPolynomialTerms<3>(i), 0 );
   }

   // Higher dimensions (only constant interpolation is supported)
   EXPECT_EQ( Numerics::numberOfPolynomialTerms<4>(-1), 0 );
   EXPECT_EQ( Numerics::numberOfPolynomialTerms<4>( 0), 1 );
   for( int i = 1; i < 10; ++i )
   {
      EXPECT_EQ( Numerics::numberOfPolynomialTerms<4>(i), 0 );
   }
   EXPECT_EQ( Numerics::numberOfPolynomialTerms<5>(-1), 0 );
   EXPECT_EQ( Numerics::numberOfPolynomialTerms<5>( 0), 1 );
   for( int i = 1; i < 10; ++i )
   {
      EXPECT_EQ( Numerics::numberOfPolynomialTerms<5>(i), 0 );
   }
   EXPECT_EQ( Numerics::numberOfPolynomialTerms<6>(-1), 0 );
   EXPECT_EQ( Numerics::numberOfPolynomialTerms<6>( 0), 1 );
   for( int i = 1; i < 10; ++i )
   {
      EXPECT_EQ( Numerics::numberOfPolynomialTerms<6>(i), 0 );
   }
}
