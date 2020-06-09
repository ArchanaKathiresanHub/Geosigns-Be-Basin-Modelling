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

class ElementGeometryMatrixTest : public ::testing::Test
{
public:
   ElementGeometryMatrixTest();
   ~ElementGeometryMatrixTest() {;}
   void fillWithDefault(int row, int col) {
      int k = 0;
      for (size_t i = 1; i <= row; i++)
      {
         for (size_t j = 1; j <= col; j++)
         {
            elemGM(i, j) = static_cast<double>(++k);
         }

      }
   }
   FiniteElementMethod::ElementGeometryMatrix elemGM;// 3x8
};

ElementGeometryMatrixTest::ElementGeometryMatrixTest()
{
   fillWithDefault(3,8);
}

TEST_F(ElementGeometryMatrixTest, methodsTest)
{
   // test access operator
   EXPECT_DOUBLE_EQ(elemGM(1,1), 1);
   EXPECT_DOUBLE_EQ(elemGM(3, 8), 24);

   //change an entry 
   elemGM(2, 4) = 67.7;
   EXPECT_DOUBLE_EQ(elemGM(2, 4), 67.7);

   // test numberOfColumns
   EXPECT_DOUBLE_EQ(elemGM.numberOfColumns(), 8);

   // test numberOfRows
   EXPECT_DOUBLE_EQ(elemGM.numberOfRows(), 3);

   // test .zero()
   elemGM.zero();
   for (size_t i = 1; i <= 3; i++)
   {
      for (size_t j = 1; j <= 8; j++)
      {
        EXPECT_DOUBLE_EQ( elemGM(i, j),0);
      }

   }
}

TEST_F(ElementGeometryMatrixTest, matrixMatrixProductTest) {
   fillWithDefault(3, 8);
   FiniteElementMethod::GradElementVector Geml; //8x3
   int k = 0;
   for (size_t i = 1; i <= 8; i++)
   {
      for (size_t j = 1; j <= 3; j++)
      {
         Geml(i, j) = ++k;
      }

   }
   
   FiniteElementMethod::Matrix3x3 jacob;
   FiniteElementMethod::matrixMatrixProduct(elemGM, Geml, jacob);
   
   //jacob(1,1) = 1x1 + 2x4 + 3x7 +4x10 + 5x13 + 6x16 +7x19 + 8x22 = 540; etc...
   EXPECT_NEAR(jacob(1, 1), 540, 1e-6); EXPECT_NEAR(jacob(1, 2), 576, 1e-6); EXPECT_NEAR(jacob(1, 3), 612, 1e-6);
   EXPECT_NEAR(jacob(2, 1), 1276, 1e-6); EXPECT_NEAR(jacob(2, 2), 1376, 1e-6); EXPECT_NEAR(jacob(2, 3), 1476, 1e-6);
   EXPECT_NEAR(jacob(3, 1), 2012, 1e-6); EXPECT_NEAR(jacob(3, 2), 2176, 1e-6); EXPECT_NEAR(jacob(3, 3), 2340, 1e-6);

   // check determinant which is zero
   auto d = FiniteElementMethod::determinant(jacob);
   EXPECT_DOUBLE_EQ(d, 0);

   // test .getMidPoint
   auto mid = elemGM.getMidPoint();
   EXPECT_NEAR(mid, 20.5,1e-9);
}