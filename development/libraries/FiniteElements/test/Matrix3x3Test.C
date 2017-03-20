//
// Unit tests for FiniteElementMethod::Matrix3x3 
//

#include "../src/FiniteElementTypes.h"

#include <gtest/gtest.h>

class Matrix3x3Test: public ::testing::Test
{
public:
   Matrix3x3Test();
   ~Matrix3x3Test() {;}

   FiniteElementMethod::Matrix3x3 mat;
   FiniteElementMethod::Matrix3x3 mat2;

   FiniteElementMethod::ThreeVector vec1;
};

Matrix3x3Test::Matrix3x3Test()
{
   // init test matrix
   mat( 1, 1 ) = 1;
   mat( 1, 2 ) = 4;
   mat( 1, 3 ) = 5;
   mat( 2, 1 ) = 6;
   mat( 2, 2 ) = 2;
   mat( 2, 3 ) = 7;
   mat( 3, 1 ) = 8;
   mat( 3, 2 ) = 9;
   mat( 3, 3 ) = 3;

   mat2( 1, 1 ) = 1;
   mat2( 1, 2 ) = 2;
   mat2( 1, 3 ) = 3;
   mat2( 2, 1 ) = 4;
   mat2( 2, 2 ) = 5;
   mat2( 2, 3 ) = 6;
   mat2( 3, 1 ) = 7;
   mat2( 3, 2 ) = 8;
   mat2( 3, 3 ) = 9;

   vec1( 1 ) = 16;
   vec1( 2 ) = 10;
   vec1( 3 ) = 70;
}


TEST_F( Matrix3x3Test, zeroTest )
{
   // test A = 0 
   FiniteElementMethod::Matrix3x3 smat = mat;
   smat.zero();

   for ( int i = 1; i <= 3; ++i )
      for ( int j = 1; j <= 3; ++j )
         EXPECT_DOUBLE_EQ( 0.0, smat( i, j ) );
}

TEST_F( Matrix3x3Test, scaleTest )
{
   // test A = A * 3.14
   FiniteElementMethod::Matrix3x3 smat = mat;

   FiniteElementMethod::scale( smat, 3.14 );

   EXPECT_NEAR( smat(1,1),  3.1400, 1e-6 );
   EXPECT_NEAR( smat(1,2), 12.5600, 1e-6 );
   EXPECT_NEAR( smat(1,3), 15.7000, 1e-6 );

   EXPECT_NEAR( smat(2,1), 18.8400, 1e-6 );
   EXPECT_NEAR( smat(2,2),  6.2800, 1e-6 );
   EXPECT_NEAR( smat(2,3), 21.9800, 1e-6 );

   EXPECT_NEAR( smat(3,1), 25.1200, 1e-6 );
   EXPECT_NEAR( smat(3,2), 28.2600, 1e-6 );
   EXPECT_NEAR( smat(3,3),  9.4200, 1e-6 );

   smat = mat;
   smat *= 3.14;

   EXPECT_NEAR( smat(1,1),  3.1400, 1e-6 );
   EXPECT_NEAR( smat(1,2), 12.5600, 1e-6 );
   EXPECT_NEAR( smat(1,3), 15.7000, 1e-6 );

   EXPECT_NEAR( smat(2,1), 18.8400, 1e-6 );
   EXPECT_NEAR( smat(2,2),  6.2800, 1e-6 );
   EXPECT_NEAR( smat(2,3), 21.9800, 1e-6 );

   EXPECT_NEAR( smat(3,1), 25.1200, 1e-6 );
   EXPECT_NEAR( smat(3,2), 28.2600, 1e-6 );
   EXPECT_NEAR( smat(3,3),  9.4200, 1e-6 );
}


TEST_F( Matrix3x3Test, matrixVectorProductTest )
{
   //test c = A * v
   FiniteElementMethod::ThreeVector resVec;

   FiniteElementMethod::matrixVectorProduct ( mat, vec1, resVec );
   EXPECT_NEAR( resVec(1), 406, 1e-6 );
   EXPECT_NEAR( resVec(2), 606, 1e-6 );
   EXPECT_NEAR( resVec(3), 428, 1e-6 );

   const FiniteElementMethod::ThreeVector retVec = FiniteElementMethod::matrixVectorProduct ( mat, vec1 );

   EXPECT_NEAR( retVec(1), 406, 1e-6 );
   EXPECT_NEAR( retVec(2), 606, 1e-6 );
   EXPECT_NEAR( retVec(3), 428, 1e-6 );
}

 
TEST_F( Matrix3x3Test, matrixTransposeVectorProductTest )
{
   //test c = A' * v
   FiniteElementMethod::ThreeVector resVec;

   FiniteElementMethod::matrixTransposeVectorProduct( mat, vec1, resVec );

   EXPECT_NEAR( resVec(1), 636, 1e-6 );
   EXPECT_NEAR( resVec(2), 714, 1e-6 );
   EXPECT_NEAR( resVec(3), 360, 1e-6 );
}

TEST_F( Matrix3x3Test, matrixMatrixProductTest )
{
   // test C = A * B
   FiniteElementMethod::Matrix3x3 resMat;

   FiniteElementMethod::matrixMatrixProduct( mat, mat2, resMat );

   EXPECT_NEAR( resMat(1,1), 52, 1e-6 );
   EXPECT_NEAR( resMat(1,2), 62, 1e-6 );
   EXPECT_NEAR( resMat(1,3), 72, 1e-6 );

   EXPECT_NEAR( resMat(2,1), 63, 1e-6 );
   EXPECT_NEAR( resMat(2,2), 78, 1e-6 );
   EXPECT_NEAR( resMat(2,3), 93, 1e-6 );

   EXPECT_NEAR( resMat(3,1), 65, 1e-6 );
   EXPECT_NEAR( resMat(3,2), 85, 1e-6 );
   EXPECT_NEAR( resMat(3,3),105, 1e-6 );
}

TEST_F( Matrix3x3Test, addTest )
{
   // test C = A + B
   FiniteElementMethod::Matrix3x3 resMat;

   FiniteElementMethod::add( mat, mat2, resMat );

   EXPECT_NEAR( resMat(1,1),  2, 1e-6 );
   EXPECT_NEAR( resMat(1,2),  6, 1e-6 );
   EXPECT_NEAR( resMat(1,3),  8, 1e-6 );

   EXPECT_NEAR( resMat(2,1), 10, 1e-6 );
   EXPECT_NEAR( resMat(2,2),  7, 1e-6 );
   EXPECT_NEAR( resMat(2,3), 13, 1e-6 );

   EXPECT_NEAR( resMat(3,1), 15, 1e-6 );
   EXPECT_NEAR( resMat(3,2), 17, 1e-6 );
   EXPECT_NEAR( resMat(3,3), 12, 1e-6 );

   resMat = mat;
   resMat += mat2;

   EXPECT_NEAR( resMat(1,1),  2, 1e-6 );
   EXPECT_NEAR( resMat(1,2),  6, 1e-6 );
   EXPECT_NEAR( resMat(1,3),  8, 1e-6 );

   EXPECT_NEAR( resMat(2,1), 10, 1e-6 );
   EXPECT_NEAR( resMat(2,2),  7, 1e-6 );
   EXPECT_NEAR( resMat(2,3), 13, 1e-6 );

   EXPECT_NEAR( resMat(3,1), 15, 1e-6 );
   EXPECT_NEAR( resMat(3,2), 17, 1e-6 );
   EXPECT_NEAR( resMat(3,3), 12, 1e-6 );
}

TEST_F( Matrix3x3Test, matrixMatrixTransposeProductTest )
{
   // test C = A * B'
   FiniteElementMethod::Matrix3x3 resMat;

   FiniteElementMethod::matrixMatrixTransposeProduct( mat, mat2, resMat );

   EXPECT_NEAR( resMat(1,1), 24, 1e-6 );
   EXPECT_NEAR( resMat(1,2), 54, 1e-6 );
   EXPECT_NEAR( resMat(1,3), 84, 1e-6 );

   EXPECT_NEAR( resMat(2,1), 31, 1e-6 );
   EXPECT_NEAR( resMat(2,2), 76, 1e-6 );
   EXPECT_NEAR( resMat(2,3),121, 1e-6 );

   EXPECT_NEAR( resMat(3,1), 35, 1e-6 );
   EXPECT_NEAR( resMat(3,2), 95, 1e-6 );
   EXPECT_NEAR( resMat(3,3),155, 1e-6 );
}

TEST_F( Matrix3x3Test, matrixTransposeMatrixProductTest )
{
   // test C = A' * B  
   FiniteElementMethod::Matrix3x3 resMat;

   FiniteElementMethod::matrixTransposeMatrixProduct( mat, mat2, resMat );

   EXPECT_NEAR( resMat(1,1), 81, 1e-6 );
   EXPECT_NEAR( resMat(1,2), 96, 1e-6 );
   EXPECT_NEAR( resMat(1,3),111, 1e-6 );

   EXPECT_NEAR( resMat(2,1), 75, 1e-6 );
   EXPECT_NEAR( resMat(2,2), 90, 1e-6 );
   EXPECT_NEAR( resMat(2,3),105, 1e-6 );

   EXPECT_NEAR( resMat(3,1), 54, 1e-6 );
   EXPECT_NEAR( resMat(3,2), 69, 1e-6 );
   EXPECT_NEAR( resMat(3,3), 84, 1e-6 );
}

TEST_F( Matrix3x3Test, InversionTest )
{
   // test inverse
   FiniteElementMethod::Matrix3x3 invMat;

   FiniteElementMethod::invert( mat, invMat );

   EXPECT_NEAR( invMat(1,1),-0.200000, 1e-6 );
   EXPECT_NEAR( invMat(1,2), 0.115789, 1e-6 );
   EXPECT_NEAR( invMat(1,3), 0.063158, 1e-6 );

   EXPECT_NEAR( invMat(2,1), 0.133333, 1e-6 );
   EXPECT_NEAR( invMat(2,2),-0.129825, 1e-6 );
   EXPECT_NEAR( invMat(2,3), 0.080702, 1e-6 );

   EXPECT_NEAR( invMat(3,1), 0.133333, 1e-6 );
   EXPECT_NEAR( invMat(3,2), 0.080702, 1e-6 );
   EXPECT_NEAR( invMat(3,3),-0.077193, 1e-6 );
}


TEST_F( Matrix3x3Test, DeterminantTest )
{
   // test determinant
   double det = FiniteElementMethod::determinant( mat );

   EXPECT_NEAR( det, 285.0, 1e-6 );
}


