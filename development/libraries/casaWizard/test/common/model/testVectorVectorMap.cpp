#include "model/VectorVectorMap.h"

#include <gtest/gtest.h>

#include "ConstantsNumerical.h"

std::vector<std::vector<double>> createConstantMap(const double constantMapValue, const int dimI, const int dimJ)
{
   std::vector<std::vector<double>> mapData;
   for (int i = 0; i < dimI; i++)
   {
      std::vector<double> mapRow;
      for (int j = 0; j < dimJ; j++)
      {
         mapRow.push_back(constantMapValue);
      }
      mapData.push_back(mapRow);
   }

   return mapData;
}

TEST(VectorVectorMapTest, TestInitialisation)
{
   std::vector<std::vector<double>> mapData = createConstantMap(7.5, 5, 10);

   casaWizard::VectorVectorMap lithoMap(mapData);
   ASSERT_EQ(mapData, lithoMap.getData());
}

TEST(VectorVectorMapTest, TestInitialisationWithEmptyMap)
{
   std::vector<std::vector<double>> mapData;

   casaWizard::VectorVectorMap lithoMap(mapData);
   EXPECT_TRUE(lithoMap.getData().empty());
}

TEST(VectorVectorMapTest, TestInitialisationWithInvalidMapdata)
{
   std::vector<std::vector<double>> mapData = {{1,2,3,4}, {1,2}}; // different lengths

   casaWizard::VectorVectorMap lithoMap(mapData);
   EXPECT_TRUE(lithoMap.getData().empty());
}

TEST(VectorVectorMapTest, TestAdditionOperation)
{
   std::vector<std::vector<double>> mapData = createConstantMap(7.5, 5, 10);
   mapData[0][0] = Utilities::Numerical::CauldronNoDataValue;
   std::vector<std::vector<double>> mapData2 = createConstantMap(16.4, 5, 10);
   mapData2[1][1] = Utilities::Numerical::CauldronNoDataValue;

   casaWizard::VectorVectorMap lithoMap1(mapData);
   casaWizard::VectorVectorMap lithoMap2(mapData2);

   casaWizard::VectorVectorMap resultMap = lithoMap1 + lithoMap2;

   for (int i = 0; i < lithoMap1.getData().size(); i++)
   {
      for (int j = 0; j < lithoMap1.getData()[0].size(); j++)
      {
         if ((i == 0 && j == 0) || (i == 1 && j == 1)) // check the undefined regions
         {
            EXPECT_EQ(resultMap.getData()[i][j], Utilities::Numerical::CauldronNoDataValue);
         }
         else
         {
            EXPECT_EQ(lithoMap1.getData()[i][j] + lithoMap2.getData()[i][j], resultMap.getData()[i][j]);
         }
      }
   }
}

TEST(VectorVectorMapTest, TestMultiplicationScalarOperation)
{
   std::vector<std::vector<double>> mapData = createConstantMap(7.5, 5, 10);
   mapData[0][0] = Utilities::Numerical::CauldronNoDataValue;

   casaWizard::VectorVectorMap lithoMap1(mapData);
   casaWizard::VectorVectorMap resultMap = lithoMap1 * -1.0;

   for (int i = 0; i < lithoMap1.getData().size(); i++)
   {
      for (int j = 0; j < lithoMap1.getData()[0].size(); j++)
      {
         if (i == 0 && j == 0)
         {
            EXPECT_EQ(lithoMap1.getData()[i][j], Utilities::Numerical::CauldronNoDataValue);
         }
         else
         {
            EXPECT_EQ(lithoMap1.getData()[i][j] * -1.0, resultMap.getData()[i][j]);
         }
      }
   }
}

TEST(VectorVectorMapTest, TestAdditionScalarOperation)
{
   std::vector<std::vector<double>> mapData = createConstantMap(7.5, 5, 10);

   casaWizard::VectorVectorMap lithoMap1(mapData);
   casaWizard::VectorVectorMap resultMap = lithoMap1 + 1.0;

   for (int i = 0; i < lithoMap1.getData().size(); i++)
   {
      std::vector<double> mapRow;
      for (int j = 0; j < lithoMap1.getData()[0].size(); j++)
      {
         EXPECT_EQ(lithoMap1.getData()[i][j] + 1.0, resultMap.getData()[i][j]);
      }
   }
}

TEST(VectorVectorMapTest, TestAdditionDifferentMapShapes)
{
   std::vector<std::vector<double>> mapData = createConstantMap(7.5, 5, 10);
   std::vector<std::vector<double>> mapData2 = createConstantMap(16.4, 10, 5);

   casaWizard::VectorVectorMap lithoMap1(mapData);
   casaWizard::VectorVectorMap lithoMap2(mapData2);

   casaWizard::VectorVectorMap resultMap = lithoMap1 + lithoMap2;

   EXPECT_EQ(resultMap.getData().empty(), true);
}

