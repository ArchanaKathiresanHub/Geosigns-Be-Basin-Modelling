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

namespace {
   static const int NumberOfComponents = 2;
   // this serves as simple multicomponent class along the lines of the PVTCalulator class
   class MultiComponent {
   public:
      static const int BlockSize = NumberOfComponents;
      double m_var[NumberOfComponents];
      //Return the value for the component.
      inline double  operator ()(const int id) const {
        return m_var[id ];
      }
      // Return the value for the component.
      inline double& operator ()(const int id) {
         return m_var[id];
      }
      enum IndexType
      {
         ZERO_COMPONENT = 0,
         FIRST_COMPONENT = 1,
      };
   };
}

class MultiComponentVectorTest : public ::testing::Test
{
public:
   
   MultiComponentVectorTest() {
      fillWithDefault();
   };
   ~MultiComponentVectorTest() { };
   // this servers as a test of the fill operator ()
   void fillWithDefault() 
   {
      int k = 0;
      for (int i = 1; i <= 8; ++i) {
         for (int cmp = 0; cmp < NumberOfComponents; ++cmp)
         {
            Mvec(i)(cmp) = k++;
         }
      }
   }

   FiniteElementMethod::MultiComponentVector<MultiComponent> Mvec;
};

TEST_F(MultiComponentVectorTest, WholeClass) {
   // testing access operator ()
   int KK = 0;
   for (int j = 1; j <= 8; j++)
   {
      for (int i = 0; i < NumberOfComponents; i++)
      {
         EXPECT_DOUBLE_EQ(Mvec(j)(i), KK);
         KK++;
      }
   }

   // creating the result using the logic of the dot() function
   KK = 0;
   double res[2] = { 0.,0. };
   FiniteElementMethod::ElementVector elemVec;
   for (int i = 1; i <= 8; i++)
   {
      elemVec(i) = i - 1;
      for (int j = 0; j < NumberOfComponents; j++)
      {
         res[j] += (i - 1)*(KK++);
      }
   }
   // test the dot() function
   auto result = Mvec.dot(elemVec);
   EXPECT_DOUBLE_EQ(result(0), res[0]);
   EXPECT_DOUBLE_EQ(result(1), res[1]);
}
