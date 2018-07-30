//                                                                      
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by CGI India Pvt. Ltd.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

//
// Unit tests for FiniteElementMethod::ElementGeometryMatrix copy methods
//

#include "../src/FiniteElementTypes.h"

#include <gtest/gtest.h>
#include <memory>
#define errLim 1e-10
class CopyTest : public ::testing::Test
{
public:
   CopyTest();
   ~CopyTest() { ; }
   void fillWithDefaultelemGM(int row, int col) {
      int k = 0;
      for (int i = 1; i <= row; i++)
      {
         for (int j = 1; j <= col; j++)
         {
            elemGM(i, j) = static_cast<double>(++k);
         }

      }
   }

   void fillWithDefaultelemM(int row, int col) {
      int k = 0;
      for (int i = 1; i <= row; i++)
      {
         for (int j = 1; j <= col; j++)
         {
            elemM(i, j) = static_cast<double>(++k);
         }

      }
   }

   FiniteElementMethod::ElementGeometryMatrix elemGM;// 3x8
   FiniteElementMethod::ElementMatrix elemM;// 8x8
};

CopyTest::CopyTest()
{
   fillWithDefaultelemGM(3, 8);
   fillWithDefaultelemM(8, 8);
}

TEST_F(CopyTest, ElementGeometryMatrixtestCopy_ctor)
{
   // test copy constructor
   auto CelemGM(elemGM);
   for (int i = 1; i <= 3; i++)
   {
      for (int j = 1; j <= 8; j++)
      {
         EXPECT_NEAR(CelemGM(i, j), elemGM(i,j), errLim);
      }

   }
   // changing the copy
   for (int i = 1; i <= 3; i++)
   {
      for (int j = 1; j <= 8; j++)
      {
         CelemGM(i, j) = -99.12;
      }

   }
   for (int i = 1; i <= 3; i++)
   {
      for (int j = 1; j <= 8; j++)
      {
         EXPECT_NEAR(CelemGM(i, j), -99.12, errLim);
      }

   }
  
   // the original should not get affected
   int k_ = 0;
   for (int i = 1; i <= 3; i++)
   {
      for (int j = 1; j <= 8; j++)
      {
         EXPECT_NEAR(elemGM(i, j), ++k_, errLim);
      }

   }
}

TEST_F(CopyTest, ElementGeometryMatrixtestCopy_assignment)
{
   // test assignment operator
   FiniteElementMethod::ElementGeometryMatrix CelemGM;
   CelemGM = (elemGM);
   // changing the copy
   for (int i = 1; i <= 3; i++)
   {
      for (int j = 1; j <= 8; j++)
      {
         EXPECT_NEAR(CelemGM(i, j), elemGM(i, j), errLim);
      }

   }
   for (int i = 1; i <= 3; i++)
   {
      for (int j = 1; j <= 8; j++)
      {
         CelemGM(i, j) = 877.009;
      }

   }
   for (int i = 1; i <= 3; i++)
   {
      for (int j = 1; j <= 8; j++)
      {
         EXPECT_NEAR(CelemGM(i, j), 877.009, errLim);
      }

   }

   // the original should not get affected
   int k_ = 0;
   for (int i = 1; i <= 3; i++)
   {
      for (int j = 1; j <= 8; j++)
      {
         EXPECT_NEAR(elemGM(i, j), ++k_, errLim);
      }

   }
}

TEST_F(CopyTest, ElementMatrixtestCopy_ctor)
{
   // test copy constructor
   FiniteElementMethod::ElementMatrix CelemM(elemM);
   for (int i = 1; i <= 8; i++)
   {
      for (int j = 1; j <= 8; j++)
      {
         EXPECT_NEAR(CelemM(i, j), elemM(i, j), errLim);
      }

   }
   // changing the copy
   for (int i = 1; i <= 8; i++)
   {
      for (int j = 1; j <= 8; j++)
      {
         CelemM(i, j) = -99.12;
      }

   }
   for (int i = 1; i <= 8; i++)
   {
      for (int j = 1; j <= 8; j++)
      {
         EXPECT_NEAR(CelemM(i, j), -99.12, errLim);
      }

   }

   // the original should not get affected
   int k_ = 0;
   for (int i = 1; i <= 8; i++)
   {
      for (int j = 1; j <= 8; j++)
      {
         EXPECT_NEAR(elemM(i, j), ++k_, errLim);
      }

   }
}

TEST_F(CopyTest, ElementMatrixtestCopy_assignment)
{
   // test assignment operator
   FiniteElementMethod::ElementMatrix CelemM;
   CelemM = (elemM);
   for (int i = 1; i <= 8; i++)
   {
      for (int j = 1; j <= 8; j++)
      {
         EXPECT_NEAR(CelemM(i, j), elemM(i, j), errLim);
      }

   }
   // changing the copy
   for (int i = 1; i <= 8; i++)
   {
      for (int j = 1; j <= 8; j++)
      {
         CelemM(i, j) = 877.009;
      }

   }
   for (int i = 1; i <= 8; i++)
   {
      for (int j = 1; j <= 8; j++)
      {
         EXPECT_NEAR(CelemM(i, j), 877.009, errLim);
      }

   }

   // the original should not get affected
   int k_ = 0;
   for (int i = 1; i <= 8; i++)
   {
      for (int j = 1; j <= 8; j++)
      {
         EXPECT_NEAR(elemM(i, j), ++k_,errLim);
      }

   }
}