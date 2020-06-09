//                                                                      
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by CGI India Pvt. Ltd.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "../src/FiniteElementTypes.h"

#include <gtest/gtest.h>


TEST(FreeFunctionTest, matrixVectorProductTest) {
   int k = 0;
   FiniteElementMethod::Matrix3x3 m3;
   for (int i = 1; i <= 3; i++)
   {
      for (int j = 1; j <= 3; j++)
      {
         m3(i, j) = ++k;
      }

   }
   FiniteElementMethod::ThreeVector T3;

   for (int i = 1; i <= 3; i++)
   {
      T3(i) = i;
   }


   auto result = FiniteElementMethod::matrixVectorProduct(m3, T3);
   // result(1) = 1x1 + 2x2 + 3x3 = 14
   // result(2) = 4x1 + 5x2 + 6x3 = 32
   // result(3) = 7x1 + 8x2 + 9x3 = 50
      EXPECT_DOUBLE_EQ(result(1), 14);
      EXPECT_DOUBLE_EQ(result(2), 32);
      EXPECT_DOUBLE_EQ(result(3), 50);

      result.zero();
   // test the other overloaded matrixVectorProduct()
      FiniteElementMethod::matrixVectorProduct(m3, T3, result);
      EXPECT_DOUBLE_EQ(result(1), 14);
      EXPECT_DOUBLE_EQ(result(2), 32);
      EXPECT_DOUBLE_EQ(result(3), 50);
}

TEST(FreeFunctionTest, matrixTransposeVectorProductTest)
{
   int k = 0;
   FiniteElementMethod::Matrix3x3 m3;
   for (int i = 1; i <= 3; i++)
   {
      for (int j = 1; j <= 3; j++)
      {
         m3(i, j) = ++k;
      }

   }
   FiniteElementMethod::ThreeVector T3;

   for (int i = 1; i <= 3; i++)
   {
      T3(i) = i;
   }

   FiniteElementMethod::ThreeVector result;

   FiniteElementMethod::matrixTransposeVectorProduct(m3, T3, result);
   // result(1) = 1x1 + 4x2 + 7x3 = 30
   // result(2) = 2x1 + 5x2 + 8x3 = 36
   // result(3) = 3x1 + 6x2 + 9x3 = 42
   EXPECT_DOUBLE_EQ(result(1), 30);
   EXPECT_DOUBLE_EQ(result(2), 36);
   EXPECT_DOUBLE_EQ(result(3), 42);

}

TEST(FreeFunctionTest, matrixMatrixTransposeProductTest)
{
   int k = 1;
   FiniteElementMethod::Matrix3x3 m3;
   FiniteElementMethod::Matrix3x3 m4;
   for (int i = 1; i <= 3; i++)
   {
      for (int j = 1; j <= 3; j++)
      {
         m3(i, j) = k;
         m4(i, j) = k;
         k += 1;
      }

   }

   FiniteElementMethod::Matrix3x3 result;

   FiniteElementMethod::matrixMatrixTransposeProduct(m3, m4, result);
   //result(1,1) = 1x1 + 2x2 + 3x3 = 14; etc...
   EXPECT_DOUBLE_EQ(result(1,1), 14); EXPECT_DOUBLE_EQ(result(1, 2), 32); EXPECT_DOUBLE_EQ(result(1, 3), 50);
   EXPECT_DOUBLE_EQ(result(2, 1), 32); EXPECT_DOUBLE_EQ(result(2, 2), 77); EXPECT_DOUBLE_EQ(result(2, 3), 122);
   EXPECT_DOUBLE_EQ(result(3, 1), 50); EXPECT_DOUBLE_EQ(result(3, 2), 122); EXPECT_DOUBLE_EQ(result(3, 3), 194);

}

TEST(FreeFunctionTest, matrixTransposeMatrixProductTest)
{
   int k = 1;
   FiniteElementMethod::Matrix3x3 m3;
   FiniteElementMethod::Matrix3x3 m4;
   for (int i = 1; i <= 3; i++)
   {
      for (int j = 1; j <= 3; j++)
      {
         m3(i, j) = k;
         m4(i, j) = k;
         k += 1;
      }

   }

   FiniteElementMethod::Matrix3x3 result;

   FiniteElementMethod::matrixTransposeMatrixProduct(m3, m4, result);
   //result(1,1) = 1x1 + 4x4 + 7x7 = 66; etc...
   EXPECT_DOUBLE_EQ(result(1, 1), 66); EXPECT_DOUBLE_EQ(result(1, 2), 78); EXPECT_DOUBLE_EQ(result(1, 3), 90);
   EXPECT_DOUBLE_EQ(result(2, 1), 78); EXPECT_DOUBLE_EQ(result(2, 2), 93); EXPECT_DOUBLE_EQ(result(2, 3), 108);
   EXPECT_DOUBLE_EQ(result(3, 1), 90); EXPECT_DOUBLE_EQ(result(3, 2), 108); EXPECT_DOUBLE_EQ(result(3, 3), 126);

}

TEST(FreeFunctionTest, InvertTest) {
   FiniteElementMethod::Matrix3x3 m3;
   m3(1, 1) = 1; m3(1, 2) = 2; m3(1, 3) = 3;
   m3(2, 1) = 0; m3(2, 2) = 1; m3(2, 3) = 4;
   m3(3, 1) = 5; m3(3, 2) = 6; m3(3, 3) = 0;
   // test determinant 
  EXPECT_NEAR( FiniteElementMethod::determinant(m3),1,1e-6);

  FiniteElementMethod::Matrix3x3 invResult;
  // test invert overload 1
  FiniteElementMethod::invert(m3,invResult);
  EXPECT_NEAR(invResult(1, 1), -24, 1e-6); EXPECT_NEAR(invResult(1, 2), 18, 1e-6); EXPECT_NEAR(invResult(1, 3), 5, 1e-6);
  EXPECT_NEAR(invResult(2, 1), 20, 1e-6); EXPECT_NEAR(invResult(2, 2), -15, 1e-6); EXPECT_NEAR(invResult(2, 3), -4, 1e-6);
  EXPECT_NEAR(invResult(3, 1), -5, 1e-6); EXPECT_NEAR(invResult(3, 2), 4, 1e-6); EXPECT_NEAR(invResult(3, 3), 1, 1e-6);

  // test invert overload 2
  double determ = 0;
  invResult.zero();
  
  FiniteElementMethod::invert(m3, invResult,determ);
  EXPECT_NEAR(invResult(1, 1), -24, 1e-6); EXPECT_NEAR(invResult(1, 2), 18, 1e-6); EXPECT_NEAR(invResult(1, 3), 5, 1e-6);
  EXPECT_NEAR(invResult(2, 1), 20, 1e-6); EXPECT_NEAR(invResult(2, 2), -15, 1e-6); EXPECT_NEAR(invResult(2, 3), -4, 1e-6);
  EXPECT_NEAR(invResult(3, 1), -5, 1e-6); EXPECT_NEAR(invResult(3, 2), 4, 1e-6); EXPECT_NEAR(invResult(3, 3), 1, 1e-6);

  EXPECT_NEAR(determ, 1, 1e-6);

  //=============================Another Matrix===============================================================

  m3(1, 1) = 3; m3(1, 2) = 0; m3(1, 3) = 2;
  m3(2, 1) = 2; m3(2, 2) = 0; m3(2, 3) = -2;
  m3(3, 1) = 0; m3(3, 2) = 1; m3(3, 3) = 1;
  // test determinant 
  EXPECT_NEAR(FiniteElementMethod::determinant(m3), 10, 1e-6);

  invResult.zero();
  // test invert overload 1
  FiniteElementMethod::invert(m3, invResult);
  EXPECT_NEAR(invResult(1, 1), 0.2, 1e-6); EXPECT_NEAR(invResult(1, 2), 0.2, 1e-6); EXPECT_NEAR(invResult(1, 3), 0, 1e-6);
  EXPECT_NEAR(invResult(2, 1), -0.2, 1e-6); EXPECT_NEAR(invResult(2, 2), 0.3, 1e-6); EXPECT_NEAR(invResult(2, 3), 1., 1e-6);
  EXPECT_NEAR(invResult(3, 1), 0.2, 1e-6); EXPECT_NEAR(invResult(3, 2), -0.3, 1e-6); EXPECT_NEAR(invResult(3, 3), 0, 1e-6);

  // test invert overload 2
  determ = 0;
  invResult.zero();

  FiniteElementMethod::invert(m3, invResult, determ);
  EXPECT_NEAR(invResult(1, 1), 0.2, 1e-6); EXPECT_NEAR(invResult(1, 2), 0.2, 1e-6); EXPECT_NEAR(invResult(1, 3), 0, 1e-6);
  EXPECT_NEAR(invResult(2, 1), -0.2, 1e-6); EXPECT_NEAR(invResult(2, 2), 0.3, 1e-6); EXPECT_NEAR(invResult(2, 3), 1., 1e-6);
  EXPECT_NEAR(invResult(3, 1), 0.2, 1e-6); EXPECT_NEAR(invResult(3, 2), -0.3, 1e-6); EXPECT_NEAR(invResult(3, 3), 0, 1e-6);

  EXPECT_NEAR(determ, 10, 1e-6);
}

TEST(FreeFunctionTest, InnerProductTest) {
   // Test (1) ElementVector
   FiniteElementMethod::ElementVector e1;
   FiniteElementMethod::ElementVector e2;
   for (int i = 1; i <= 8; i++)
   {
      e1(i) = i;
      e2(i) = i;
   }
   double res = FiniteElementMethod::innerProduct(e1, e2);
   EXPECT_DOUBLE_EQ(res, 204);
   // Test (2) for ElementVector
   //=====================================================
   for (int i = 1; i <= 8; i++)
   {
      e1(i) = 1;
      e2(i) = 2;
   }
   res = FiniteElementMethod::innerProduct(e1, e2);
   EXPECT_DOUBLE_EQ(res, 16);
   //======================================================

   // Test (1) ThreeVector
   FiniteElementMethod::ThreeVector t3;
   FiniteElementMethod::ThreeVector t4;
   for (int i = 1; i <= 3; i++)
   {
      t3(i) = i;
      t4(i) = i;
   }

   res = FiniteElementMethod::innerProduct(t3, t4);
   EXPECT_DOUBLE_EQ(res, 14);
   // Test (2) ThreeVector
   //=========================================================
   t3.zero(); t4.zero(); double C = 0;
   for (int i = 1; i <= 3; i++)
   {
      C += 1.2;
      t3(i) = C;
      t4(i) = 2;
   }

   res = FiniteElementMethod::innerProduct(t3, t4);
   EXPECT_NEAR(res, 14.4, 1e-9);
   //==========================================================
}

TEST(FreeFunctionTest, NormalizeTest) {
   double c = 0;
   FiniteElementMethod::ThreeVector t3;
   FiniteElementMethod::ThreeVector t4;
   for (int i = 1; i <= 3; i++)
   {
      t3(i) = i;
      c += i*i;
   }
   c = sqrt(c);
   // manual calculations
   for (int i = 1; i <= 3; i++)
      t4(i) = t3(i) / c;
   FiniteElementMethod::normalise(t3);
   EXPECT_DOUBLE_EQ(t3(1), t4(1)); EXPECT_DOUBLE_EQ(t3(2), t4(2)); EXPECT_DOUBLE_EQ(t3(3), t4(3));
}

TEST(FreeFunctionTest, AddElementMatrix) {
   FiniteElementMethod::ElementMatrix M1;
   FiniteElementMethod::ElementMatrix M2;
   int k = 1;
   for (int i = 1; i <= 8; i++)
   {
      for (int j = 1; j <= 8; j++)
      {
         M1(i, j) = k;
         M2(i, j) = -k;
         ++k;
      }
   }
   FiniteElementMethod::ElementMatrix result;
   FiniteElementMethod::add(M1, M2, result);
   for (int i = 1; i <= 8; i++)
   {
      for (int j = 1; j <= 8; j++)
      {
         EXPECT_DOUBLE_EQ(result(i, j), 0);
      }
   }

   double m = 1.3;
   for (int i = 1; i <= 8; i++)
   {
      for (int j = 1; j <= 8; j++)
      {
         M1(i, j) = m;
         M2(i, j) = m;
      }
   }

   FiniteElementMethod::add(M1, M2, result);
   for (int i = 1; i <= 8; i++)
   {
      for (int j = 1; j <= 8; j++)
      {
         EXPECT_NEAR(result(i, j), 2.6,1e-9);
      }
   }
}