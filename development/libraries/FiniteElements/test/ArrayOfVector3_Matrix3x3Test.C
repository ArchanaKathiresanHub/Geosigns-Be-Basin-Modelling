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
// Unit tests for FiniteElementMethod::ArrayOfMatrix3x3 & FiniteElementMethod::ArrayOfVector3
//

#include "../src/FiniteElementTypes.h"
#include "../src/FiniteElementArrayTypes.h"

#include <gtest/gtest.h>

TEST( ArrayOfVector3_Matrix3x3, ArrayOfMatrix3x3methodsTest )
{
   int matCnt = 3;
   // init test Array matrix
   FiniteElementMethod::ArrayOfMatrix3x3 matArray(matCnt);
   
   // To test number of matrices stored
   EXPECT_EQ( matArray.size(), matCnt );
   
   // Complete unit test for Matrix3x3 already done in Matrix3x3Test.C 
   // Test Matrix3x3 - only basic testing for different set of 3x3matrix (init, access and reset to 0)
   
   int kk = 0;
   for (int k = 0; k < matCnt; ++k)
   {
      // init test matrix
      FiniteElementMethod::Matrix3x3 mat = matArray[matCnt];
      int k = 0;
      for ( int i = 1; i <= 3; ++i ) 
         for ( int j = 1; j <= 3; ++j )
            mat( i, j ) = static_cast<double>( ++k + kk);  
      
      // test access operator
      k = 0;
      for ( int i = 1; i <= 3; ++i ) 
         for ( int j = 1; j <= 3; ++j )   
            EXPECT_DOUBLE_EQ( mat( i, j ), static_cast<double>( ++k  + kk) );

      // test A = 0
      mat.zero();
      for ( int i = 1; i <= 3; ++i ) 
         for ( int j = 1; j <= 3; ++j )   
            EXPECT_DOUBLE_EQ( mat( i, j ), 0 );   
	 
      kk += 9;
   }
}

TEST( ArrayOfVector3_Matrix3x3, ArrayOfVector3methodsTest )
{
   int matCnt = 3;
   FiniteElementMethod::ArrayOfVector3 matArray(matCnt);
   
   // Complete unit test for ThreeVector already done in ThreeVectorTest.C 
   // Test ThreeVector - only basic testing for different set of vectors (init, access and reset to 0)   
   
   int kk = 0;
   for (int k = 0; k < matCnt; ++k)
   {
      // init test vector
      FiniteElementMethod::ThreeVector vec = matArray[matCnt];
      vec( 1 ) = 16 + kk;
      vec( 2 ) = 10 + kk;
      vec( 3 ) = 70 + kk;
      
      // test access operator
      EXPECT_DOUBLE_EQ( vec( 1 ), 16 + kk );
      EXPECT_DOUBLE_EQ( vec( 2 ), 10 + kk );
      EXPECT_DOUBLE_EQ( vec( 3 ), 70 + kk );      
      
      // test .zero()
      vec.zero();
      for ( int i = 1; i <=3; ++i ) EXPECT_DOUBLE_EQ( vec(i), 0.0 );

      // test .fill()
      vec.fill( 3.14 );
      for ( int i = 1; i <=3; ++i ) EXPECT_DOUBLE_EQ( vec(i), 3.14 );   
	 
      kk += 5;
   }   
}