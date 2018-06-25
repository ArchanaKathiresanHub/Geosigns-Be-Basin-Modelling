//
// Unit tests for FiniteElementMethod::ElementMatrix
//

#include "../src/FiniteElementTypes.h"

#include <gtest/gtest.h>

class ElementMatrixTest: public ::testing::Test
{
public:
   ElementMatrixTest();
   ~ElementMatrixTest() {;};

   FiniteElementMethod::ElementMatrix mat;
   FiniteElementMethod::ElementMatrix mat1;
   FiniteElementMethod::ElementMatrix mat2;
   
   FiniteElementMethod::ElementVector vec1;
   FiniteElementMethod::ElementVector vec2;   
   
   FiniteElementMethod::GradElementVector grdvec1;
   FiniteElementMethod::GradElementVector grdvec2;   
};

ElementMatrixTest::ElementMatrixTest()
{
   int k = 0;
   int l = 0;
   int m = 0;
   // init test matrix 
   for ( int i = 1; i <= 8; ++i ) 
      for ( int j = 1; j <= 8; ++j ) {
         mat( i, j ) = static_cast<double>( ++k );
         mat1( i, j ) = static_cast<double>( ++l );
         mat2( i, j ) = static_cast<double>( ++m );
      }
      
   // init test vector 
   for ( int i = 1; i <= 8; ++i ) vec1( i ) = static_cast<double>( i );
   for ( int i = 1; i <= 8; ++i ) vec2( i ) = static_cast<double>( 9-i );  
   
   // init test grad vector
   for ( int i = 1; i <= 8; ++i ) grdvec1( i, 1 ) = static_cast<double>( i );
   for ( int i = 1; i <= 8; ++i ) grdvec2( i, 1 ) = static_cast<double>( 9 - i );
   for ( int i = 1; i <= 8; ++i ) grdvec1( i, 2 ) = static_cast<double>( 8 + i );
   for ( int i = 1; i <= 8; ++i ) grdvec2( i, 1 ) = static_cast<double>( 17 - i );
   for ( int i = 1; i <= 8; ++i ) grdvec1( i, 3 ) = static_cast<double>( 16 + i );
   for ( int i = 1; i <= 8; ++i ) grdvec2( i, 1 ) = static_cast<double>( 25 - i );
}

TEST_F( ElementMatrixTest, methodsTest )
{
   // test access operator
   int k = 0;
   for ( int i = 1; i <= 8; ++i ) 
      for ( int j = 1; j <= 8; ++j )   
         EXPECT_DOUBLE_EQ( mat2( i, j ), static_cast<double>( ++k ) );

   // test numberOfRows & numberOfColumns
   EXPECT_EQ( mat2.numberOfRows(), 8 );
   EXPECT_EQ( mat2.numberOfColumns(), 8 );
   
   // test .zero()
   mat2.zero();
   for ( int i = 1; i <= 8; ++i )
      for ( int j = 1; j <= 8; ++j )
         EXPECT_DOUBLE_EQ( 0.0, mat2( i, j ) );
      
   // test .fill
   mat2.fill(3.14);
   for ( int i = 1; i <= 8; ++i )
      for ( int j = 1; j <= 8; ++j )
         EXPECT_DOUBLE_EQ( mat2( i, j ), 3.14 );   
}

TEST_F( ElementMatrixTest, addTest )
{
   // test C = A + B
   FiniteElementMethod::ElementMatrix resMat;
   add( mat, mat1, resMat ); 
   
   for ( int i = 1; i <= 8; ++i ) 
      for ( int j = 1; j <= 8; ++j )	
         EXPECT_NEAR( resMat( i, j ), 2.0 * mat( i, j ), 1e-6 );    
      
   // test C = A + B * 3.14
   add ( mat, 3.14, mat1, resMat ); 
   
   for ( int i = 1; i <= 8; ++i ) 
      for ( int j = 1; j <= 8; ++j )	
         EXPECT_NEAR( resMat( i, j ), mat( i, j ) + mat1( i, j ) * 3.14, 1e-6 );    
      
   // test C = A * 2.14 + B * 3.14
   add ( 2.14, mat, 3.14, mat1, resMat ); 
   
   for ( int i = 1; i <= 8; ++i ) 
      for ( int j = 1; j <= 8; ++j )
         EXPECT_NEAR( resMat( i, j ), mat( i, j ) * 2.14 + mat1( i, j ) * 3.14, 1e-6 );    
}  

TEST_F( ElementMatrixTest, subtractTest )
{
   // test C = A - B
   FiniteElementMethod::ElementMatrix resMat;
   subtract( mat, mat1, resMat ); 
   
   for ( int i = 1; i <= 8; ++i ) 
      for ( int j = 1; j <= 8; ++j )
         EXPECT_NEAR( resMat( i, j ), 0.0, 1e-6 );    
}

TEST_F( ElementMatrixTest, scaleTest )
{
   // test C = C * 3.14
   int k = 0;
   for ( int i = 1; i <= 8; ++i ) 
      for ( int j = 1; j <= 8; ++j )   
         mat2( i, j ) = static_cast<double>( ++k );
      
   scale ( mat2, 3.14 ); 
   
   for ( int i = 1; i <= 8; ++i ) 
      for ( int j = 1; j <= 8; ++j )
         EXPECT_NEAR( mat2( i, j ), 3.14 * mat( i, j ), 1e-6 );    
}

TEST_F( ElementMatrixTest, OuterProductTest )
{
   // test C = vector1 * vector2
   FiniteElementMethod::ElementMatrix resMat;
   OuterProduct( vec1, vec2, resMat ); 
   
   for ( int i = 1; i <= 8; ++i ) 
      for ( int j = 1; j <= 8; ++j )
         EXPECT_NEAR( resMat( i, j ), i * ( 9 - j ), 1e-6 );    
      
   // test C = grad vector1 * grad vector2
   OuterProduct( grdvec1, grdvec2, resMat );
   
   double IP;
   for ( int i = 1; i <= 8; ++i ) 
      for ( int j = 1; j <= 8; ++j ) {
         IP = grdvec1( i, 1 ) * grdvec2 ( j, 1 ) + grdvec1 ( i, 2 ) * grdvec2 ( j, 2 ) + grdvec1 ( i, 3 ) * grdvec2 ( j, 3 );
         EXPECT_NEAR( resMat( i, j ), IP, 1e-6 );    
      }       
}

TEST_F( ElementMatrixTest, addOuterProductTest )
{
   FiniteElementMethod::ElementMatrix resMat;
   FiniteElementMethod::ElementMatrix resMat1;
   
   // test C = vector1 * vector2
   OuterProduct( vec1, vec2, resMat ); 
   OuterProduct( vec1, vec2, resMat1 ); 
   // test C += vector1 * vector2   
   addOuterProduct ( vec1, vec2, resMat1 );
   
   for ( int i = 1; i <= 8; ++i ) 
      for ( int j = 1; j <= 8; ++j )
         EXPECT_NEAR( resMat1( i, j ), resMat( i, j ) + i * ( 9 - j ), 1e-6 );    

   // test C = vector1 * vector2
   OuterProduct( vec1, vec2, resMat1 ); 
   // test C += 3.14 * vector1 * vector2   
   addOuterProduct( 3.14, vec1, vec2, resMat1 );
   
   for ( int i = 1; i <= 8; ++i ) 
      for ( int j = 1; j <= 8; ++j )
         EXPECT_NEAR( resMat1( i, j ), resMat( i, j ) + 3.14 * i * ( 9 - j ), 1e-6 );    
      
   // test C += grad vector1 * grad vector2
   addOuterProduct( grdvec1, grdvec2, resMat );
   OuterProduct( vec1, vec2, resMat1 );
   
   double IP;
   for ( int i = 1; i <= 8; ++i ) 
      for ( int j = 1; j <= 8; ++j ) {
         IP = grdvec1( i, 1 ) * grdvec2 ( j, 1 ) + grdvec1 ( i, 2 ) * grdvec2 ( j, 2 ) + grdvec1 ( i, 3 ) * grdvec2 ( j, 3 );
         EXPECT_NEAR( resMat( i, j ), resMat1( i, j ) + IP , 1e-6 );    
      }  
      
   // test C += 3.14 * grad vector1 * grad vector2
   OuterProduct( vec1, vec2, resMat );
   addOuterProduct( 3.14, grdvec1, grdvec2, resMat );
   
   for ( int i = 1; i <= 8; ++i ) 
      for ( int j = 1; j <= 8; ++j ) {
         IP = grdvec1( i, 1 ) * grdvec2 ( j, 1 ) + grdvec1 ( i, 2 ) * grdvec2 ( j, 2 ) + grdvec1 ( i, 3 ) * grdvec2 ( j, 3 );
         EXPECT_NEAR( resMat( i, j ), resMat1( i, j ) + 3.14 * IP , 1e-6 );    
      }       
}