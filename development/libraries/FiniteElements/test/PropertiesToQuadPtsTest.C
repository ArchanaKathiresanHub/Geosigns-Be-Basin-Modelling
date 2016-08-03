#include "PropertiesToQuadPts.h"
#include "FiniteElementTypes.h"
#include "AlignedDenseMatrix.h"
#include <gtest/gtest.h>
#include <algorithm>
#include <cmath>


TEST( PropertiesToQuadPts, DefaultTest )
{
   const int numDofs = 8;
   const int numQuadPts = 12;

   // Create basis function matrix
   Numerics::AlignedDenseMatrix basisFunMat( numDofs, numQuadPts );
   double column [ numDofs ] = { 1.0, -2.0, 3.0, -4.0, 5.0, -6.0, 7.0, -8.0 };
   for( unsigned int i = 0; i < numQuadPts; ++i )
   {
      std::copy( column, column + numDofs, basisFunMat.getColumn( i ) );
   }

   for( unsigned int k = 0; k < 2; ++k )
   {
      FiniteElementMethod::ElementVector Ph;
      Ph(1) = 100.0;
      Ph(2) = 99.0;
      Ph(3) = 106.0;
      Ph(4) = 105.0;
      Ph(5) = 98.0;
      Ph(6) = 102.0;
      Ph(7) = 100.0;
      Ph(8) = 101.0;
      FiniteElementMethod::ElementVector Pl;
      Pl(1) = 200.0;
      Pl(2) = 199.0;
      Pl(3) = 206.0;
      Pl(4) = 205.0;
      Pl(5) = 198.0;
      Pl(6) = 202.0;
      Pl(7) = 200.0;
      Pl(8) = 201.0;

      FiniteElementMethod::PropertiesToQuadPts test( basisFunMat );
      if( k == 0 )
      {
         test.compute( FiniteElementMethod::Ph, Ph,
                        FiniteElementMethod::Pl, Pl );
         EXPECT_EQ( test.getPropertyNames().size(), 2 );
         EXPECT_EQ( test.getPropertyNames()[0], FiniteElementMethod::Ph );
         EXPECT_EQ( test.getPropertyNames()[1], FiniteElementMethod::Pl );
         EXPECT_EQ( test.getNumberOfProperties(), 2 );
         ASSERT_DEATH( test.getProperty( FiniteElementMethod::Ves ), "" );
      }
      else
      {
         FiniteElementMethod::ElementVector ves;
         ves(1) = 300.0;
         ves(2) = 299.0;
         ves(3) = 306.0;
         ves(4) = 305.0;
         ves(5) = 298.0;
         ves(6) = 302.0;
         ves(7) = 300.0;
         ves(8) = 301.0;
         test.compute( FiniteElementMethod::Ph, Ph,
                       FiniteElementMethod::Pl, Pl,
                       FiniteElementMethod::Ves, ves);
         EXPECT_EQ( test.getPropertyNames().size(), 3 );
         EXPECT_EQ( test.getPropertyNames()[0], FiniteElementMethod::Ph );
         EXPECT_EQ( test.getPropertyNames()[1], FiniteElementMethod::Pl );
         EXPECT_EQ( test.getPropertyNames()[2], FiniteElementMethod::Ves );
         EXPECT_EQ( test.getNumberOfProperties(), 3 );
      }

      const double * const PhDofs = test.getProperty( FiniteElementMethod::Ph );
      EXPECT_DOUBLE_EQ( PhDofs[ 0], -430.0 );
      EXPECT_DOUBLE_EQ( PhDofs[ 1], -430.0 );
      EXPECT_DOUBLE_EQ( PhDofs[ 2], -430.0 );
      EXPECT_DOUBLE_EQ( PhDofs[ 3], -430.0 );
      EXPECT_DOUBLE_EQ( PhDofs[ 4], -430.0 );
      EXPECT_DOUBLE_EQ( PhDofs[ 5], -430.0 );
      EXPECT_DOUBLE_EQ( PhDofs[ 6], -430.0 );
      EXPECT_DOUBLE_EQ( PhDofs[ 7], -430.0 );
      EXPECT_DOUBLE_EQ( PhDofs[ 8], -430.0 );
      EXPECT_DOUBLE_EQ( PhDofs[ 9], -430.0 );
      EXPECT_DOUBLE_EQ( PhDofs[10], -430.0 );
      EXPECT_DOUBLE_EQ( PhDofs[11], -430.0 );

      const double * const PlDofs = test.getProperty( 1 ); // aka FiniteElementMethod::Lithostatic_Pressure
      EXPECT_DOUBLE_EQ( PlDofs[ 0], -830.0 );
      EXPECT_DOUBLE_EQ( PlDofs[ 1], -830.0 );
      EXPECT_DOUBLE_EQ( PlDofs[ 2], -830.0 );
      EXPECT_DOUBLE_EQ( PlDofs[ 3], -830.0 );
      EXPECT_DOUBLE_EQ( PlDofs[ 4], -830.0 );
      EXPECT_DOUBLE_EQ( PlDofs[ 5], -830.0 );
      EXPECT_DOUBLE_EQ( PlDofs[ 6], -830.0 );
      EXPECT_DOUBLE_EQ( PlDofs[ 7], -830.0 );
      EXPECT_DOUBLE_EQ( PlDofs[ 8], -830.0 );
      EXPECT_DOUBLE_EQ( PlDofs[ 9], -830.0 );
      EXPECT_DOUBLE_EQ( PlDofs[10], -830.0 );
      EXPECT_DOUBLE_EQ( PlDofs[11], -830.0 );
      
      if( k == 1 )
      {
         const double * const vesDofs = test.getProperty( FiniteElementMethod::Ves );
         EXPECT_DOUBLE_EQ( vesDofs[ 0], -1230.0 );
         EXPECT_DOUBLE_EQ( vesDofs[ 1], -1230.0 );
         EXPECT_DOUBLE_EQ( vesDofs[ 2], -1230.0 );
         EXPECT_DOUBLE_EQ( vesDofs[ 3], -1230.0 );
         EXPECT_DOUBLE_EQ( vesDofs[ 4], -1230.0 );
         EXPECT_DOUBLE_EQ( vesDofs[ 5], -1230.0 );
         EXPECT_DOUBLE_EQ( vesDofs[ 6], -1230.0 );
         EXPECT_DOUBLE_EQ( vesDofs[ 7], -1230.0 );
         EXPECT_DOUBLE_EQ( vesDofs[ 8], -1230.0 );
         EXPECT_DOUBLE_EQ( vesDofs[ 9], -1230.0 );
         EXPECT_DOUBLE_EQ( vesDofs[10], -1230.0 );
         EXPECT_DOUBLE_EQ( vesDofs[11], -1230.0 );
      }
   }
}
