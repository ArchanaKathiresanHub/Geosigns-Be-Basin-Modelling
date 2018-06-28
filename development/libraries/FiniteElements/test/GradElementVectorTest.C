//
// Copyright (C) 2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

//
// Unit tests for FiniteElementMethod::GradElementVector
//

#include "../src/FiniteElementTypes.h"

#include <gtest/gtest.h>

TEST( GradElementVectorTest, methodsTest )
{
   FiniteElementMethod::GradElementVector grdvec1;
   
   // init test grad vector
   for ( int i = 1; i <= 8; ++i ) grdvec1( i, 1 ) = static_cast<double>( i );
   for ( int i = 1; i <= 8; ++i ) grdvec1( i, 2 ) = static_cast<double>( 8 + i );
   for ( int i = 1; i <= 8; ++i ) grdvec1( i, 3 ) = static_cast<double>( 16 + i );
      
   // test access operator
   for ( int i = 1; i <= 8; ++i ) EXPECT_DOUBLE_EQ( grdvec1( i, 1 ), static_cast<double>( i ) );
   for ( int i = 1; i <= 8; ++i ) EXPECT_DOUBLE_EQ( grdvec1( i, 2 ), static_cast<double>( 8 + i ) );
   for ( int i = 1; i <= 8; ++i ) EXPECT_DOUBLE_EQ( grdvec1( i, 3 ), static_cast<double>( 16 + i ) );   

   // test numberOfRows & numberOfColumns
   EXPECT_EQ( grdvec1.numberOfRows(), 8 );
   EXPECT_EQ( grdvec1.numberOfColumns(), 3 );
   
   // test .zero()
   grdvec1.zero();
   
   for ( int i = 1; i <= 8; ++i ) EXPECT_DOUBLE_EQ( grdvec1( i, 1 ), 0.0 );
   for ( int i = 1; i <= 8; ++i ) EXPECT_DOUBLE_EQ( grdvec1( i, 2 ), 0.0 );
   for ( int i = 1; i <= 8; ++i ) EXPECT_DOUBLE_EQ( grdvec1( i, 3 ), 0.0 ); 
}

TEST( GradElementVectorTest, addTest )
{
   FiniteElementMethod::GradElementVector grdvec1;
   FiniteElementMethod::GradElementVector grdvec2;
   FiniteElementMethod::GradElementVector resgrdvec;
   
   // init grad vector1 and vector2
   for ( int i = 1; i <= 8; ++i ) grdvec1( i, 1 ) = static_cast<double>( i );
   for ( int i = 1; i <= 8; ++i ) grdvec2( i, 1 ) = static_cast<double>( 9 - i );
   for ( int i = 1; i <= 8; ++i ) grdvec1( i, 2 ) = static_cast<double>( 8 + i );
   for ( int i = 1; i <= 8; ++i ) grdvec2( i, 1 ) = static_cast<double>( 17 - i );
   for ( int i = 1; i <= 8; ++i ) grdvec1( i, 3 ) = static_cast<double>( 16 + i );
   for ( int i = 1; i <= 8; ++i ) grdvec2( i, 1 ) = static_cast<double>( 25 - i );
   
   // test C = grad vector1 + grad vector2
   add( grdvec1, grdvec2, resgrdvec );

   for ( int i = 1; i <= 8; ++i ) EXPECT_DOUBLE_EQ( resgrdvec( i, 1 ), grdvec1( i, 1 ) + grdvec2( i, 1 ) );
   for ( int i = 1; i <= 8; ++i ) EXPECT_DOUBLE_EQ( resgrdvec( i, 2 ), grdvec1( i, 2 ) + grdvec2( i, 2 ) );
   for ( int i = 1; i <= 8; ++i ) EXPECT_DOUBLE_EQ( resgrdvec( i, 3 ), grdvec1( i, 3 ) + grdvec2( i, 3 ) ); 
   
   // test C = grad vector1 + 3.14 * grad vector2
   add( grdvec1, 3.14, grdvec2, resgrdvec );

   for ( int i = 1; i <= 8; ++i ) EXPECT_DOUBLE_EQ( resgrdvec( i, 1 ), grdvec1( i, 1 ) + 3.14 * grdvec2( i, 1 ) );
   for ( int i = 1; i <= 8; ++i ) EXPECT_DOUBLE_EQ( resgrdvec( i, 2 ), grdvec1( i, 2 ) + 3.14 * grdvec2( i, 2 ) );
   for ( int i = 1; i <= 8; ++i ) EXPECT_DOUBLE_EQ( resgrdvec( i, 3 ), grdvec1( i, 3 ) + 3.14 * grdvec2( i, 3 ) ); 
   
   // test C = 3.14 * grad vector1 + 3.14 * grad vector2
   add( 3.14, grdvec1, 3.14, grdvec2, resgrdvec );

   for ( int i = 1; i <= 8; ++i ) EXPECT_DOUBLE_EQ( resgrdvec( i, 1 ), 3.14 * grdvec1( i, 1 ) + 3.14 * grdvec2( i, 1 ) );
   for ( int i = 1; i <= 8; ++i ) EXPECT_DOUBLE_EQ( resgrdvec( i, 2 ), 3.14 * grdvec1( i, 2 ) + 3.14 * grdvec2( i, 2 ) );
   for ( int i = 1; i <= 8; ++i ) EXPECT_DOUBLE_EQ( resgrdvec( i, 3 ), 3.14 * grdvec1( i, 3 ) + 3.14 * grdvec2( i, 3 ) );   
}

TEST( GradElementVectorTest, subtractTest )
{
   FiniteElementMethod::GradElementVector grdvec1;
   FiniteElementMethod::GradElementVector grdvec2;
   FiniteElementMethod::GradElementVector resgrdvec;
   
   // init grad vector1 and vector2
   for ( int i = 1; i <= 8; ++i ) grdvec1( i, 1 ) = static_cast<double>( i );
   for ( int i = 1; i <= 8; ++i ) grdvec2( i, 1 ) = static_cast<double>( 9 - i );
   for ( int i = 1; i <= 8; ++i ) grdvec1( i, 2 ) = static_cast<double>( 8 + i );
   for ( int i = 1; i <= 8; ++i ) grdvec2( i, 1 ) = static_cast<double>( 17 - i );
   for ( int i = 1; i <= 8; ++i ) grdvec1( i, 3 ) = static_cast<double>( 16 + i );
   for ( int i = 1; i <= 8; ++i ) grdvec2( i, 1 ) = static_cast<double>( 25 - i );
   
   // test C = grad vector1 - grad vector2
   subtract( grdvec1, grdvec2, resgrdvec );

   for ( int i = 1; i <= 8; ++i ) EXPECT_DOUBLE_EQ( resgrdvec( i, 1 ), grdvec1( i, 1 ) - grdvec2( i, 1 ) );
   for ( int i = 1; i <= 8; ++i ) EXPECT_DOUBLE_EQ( resgrdvec( i, 2 ), grdvec1( i, 2 ) - grdvec2( i, 2 ) );
   for ( int i = 1; i <= 8; ++i ) EXPECT_DOUBLE_EQ( resgrdvec( i, 3 ), grdvec1( i, 3 ) - grdvec2( i, 3 ) ); 
}

TEST( GradElementVectorTest, matrixVectorProductTest )
{
   FiniteElementMethod::GradElementVector grdvec1;
   FiniteElementMethod::ThreeVector vec1;
   FiniteElementMethod::ElementVector resvec;
  
   // init test grad vector
   for ( int i = 1; i <= 8; ++i ) grdvec1( i, 1 ) = static_cast<double>( i );
   for ( int i = 1; i <= 8; ++i ) grdvec1( i, 2 ) = static_cast<double>( 8 + i );
   for ( int i = 1; i <= 8; ++i ) grdvec1( i, 3 ) = static_cast<double>( 16 + i );
   // init test three vector
   vec1( 1 ) = 10;
   vec1( 2 ) = 20;
   vec1( 3 ) = 30;   
   
   // test C = grad vector * three vector
   matrixVectorProduct( grdvec1, vec1, resvec );

   for ( int i = 1; i <= 8; ++i ) EXPECT_DOUBLE_EQ( resvec( i ), grdvec1( i, 1 ) * 10 + grdvec1( i, 2 ) * 20 + grdvec1( i, 3 ) * 30  );
}

TEST( GradElementVectorTest, matrixTransposeVectorProductTest )
{
   FiniteElementMethod::GradElementVector grdvec1;
   FiniteElementMethod::ElementVector vec1;   
   FiniteElementMethod::ThreeVector resvec;
  
   // init test grad vector
   for ( int i = 1; i <= 8; ++i ) grdvec1( i, 1 ) = static_cast<double>( i );
   for ( int i = 1; i <= 8; ++i ) grdvec1( i, 2 ) = static_cast<double>( 8 + i );
   for ( int i = 1; i <= 8; ++i ) grdvec1( i, 3 ) = static_cast<double>( 16 + i );
   // init test element vector
   for ( int i = 1; i <= 8; ++i ) vec1( i ) = static_cast<double>( i );   
   
   // test C = grad vector * element vector
   matrixTransposeVectorProduct( grdvec1, vec1, resvec );

   double IP;
   for ( int i = 1; i <= 3; ++i ) {
      IP = 0.0;
      for ( int j = 1; j <= 8; j++ )
         IP = IP + grdvec1 ( j, i ) * vec1 ( j );

      EXPECT_DOUBLE_EQ( resvec( i ), IP  );
   }
}

TEST( GradElementVectorTest, matrixMatrixProductTest )
{
   FiniteElementMethod::GradElementVector grdvec1;
   FiniteElementMethod::Matrix3x3 mat;   
   FiniteElementMethod::GradElementVector resvec;
  
   // init test grad vector
   for ( int i = 1; i <= 8; ++i ) grdvec1( i, 1 ) = static_cast<double>( i );
   for ( int i = 1; i <= 8; ++i ) grdvec1( i, 2 ) = static_cast<double>( 8 + i );
   for ( int i = 1; i <= 8; ++i ) grdvec1( i, 3 ) = static_cast<double>( 16 + i );
   // init test matrix
   mat( 1, 1 ) = 1;
   mat( 1, 2 ) = 2;
   mat( 1, 3 ) = 3;
   mat( 2, 1 ) = 4;
   mat( 2, 2 ) = 5;
   mat( 2, 3 ) = 6;
   mat( 3, 1 ) = 7;
   mat( 3, 2 ) = 8;
   mat( 3, 3 ) = 9;  
   
   // test C = grad vector * 3x3 matrix
   matrixMatrixProduct( grdvec1, mat, resvec );

   for ( int i = 1; i <= 8; ++i ) {
      EXPECT_DOUBLE_EQ( resvec( i, 1 ), grdvec1 ( i, 1 ) * mat ( 1, 1 ) + grdvec1 ( i, 2 ) * mat ( 2, 1 ) + grdvec1 ( i, 3 ) * mat ( 3, 1 )  );
      EXPECT_DOUBLE_EQ( resvec( i, 2 ), grdvec1 ( i, 1 ) * mat ( 1, 2 ) + grdvec1 ( i, 2 ) * mat ( 2, 2 ) + grdvec1 ( i, 3 ) * mat ( 3, 2 )  );
      EXPECT_DOUBLE_EQ( resvec( i, 3 ), grdvec1 ( i, 1 ) * mat ( 1, 3 ) + grdvec1 ( i, 2 ) * mat ( 2, 3 ) + grdvec1 ( i, 3 ) * mat ( 3, 3 )  );
   }
}

TEST( GradElementVectorTest, matrixMatrixTransposeProductTest )
{
   FiniteElementMethod::GradElementVector grdvec1;
   FiniteElementMethod::Matrix3x3 mat;   
   FiniteElementMethod::GradElementVector resvec;
  
   // init test grad vector
   for ( int i = 1; i <= 8; ++i ) grdvec1( i, 1 ) = static_cast<double>( i );
   for ( int i = 1; i <= 8; ++i ) grdvec1( i, 2 ) = static_cast<double>( 8 + i );
   for ( int i = 1; i <= 8; ++i ) grdvec1( i, 3 ) = static_cast<double>( 16 + i );
   // init test matrix
   mat( 1, 1 ) = 1;
   mat( 1, 2 ) = 2;
   mat( 1, 3 ) = 3;
   mat( 2, 1 ) = 4;
   mat( 2, 2 ) = 5;
   mat( 2, 3 ) = 6;
   mat( 3, 1 ) = 7;
   mat( 3, 2 ) = 8;
   mat( 3, 3 ) = 9;  
   
   // test C = grad vector * 3x3 transpose matrix
   matrixMatrixTransposeProduct( grdvec1, mat, resvec );

   for ( int i = 1; i <= 8; ++i ) {
      EXPECT_DOUBLE_EQ( resvec( i, 1 ), grdvec1 ( i, 1 ) * mat ( 1, 1 ) + grdvec1 ( i, 2 ) * mat ( 1, 2 ) + grdvec1 ( i, 3 ) * mat ( 1, 3 )  );
      EXPECT_DOUBLE_EQ( resvec( i, 2 ), grdvec1 ( i, 1 ) * mat ( 2, 1 ) + grdvec1 ( i, 2 ) * mat ( 2, 2 ) + grdvec1 ( i, 3 ) * mat ( 2, 3 )  );
      EXPECT_DOUBLE_EQ( resvec( i, 3 ), grdvec1 ( i, 1 ) * mat ( 3, 1 ) + grdvec1 ( i, 2 ) * mat ( 3, 2 ) + grdvec1 ( i, 3 ) * mat ( 3, 3 )  );
   }
}
