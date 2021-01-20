#include "model/lithofractionmap.h"

#include <gtest/gtest.h>


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

TEST(LithofractionMapTest, TestInitialisation)
{
  std::vector<std::vector<double>> mapData = createConstantMap(7.5, 5, 10);

  casaWizard::LithofractionMap lithoMap(mapData);
  ASSERT_EQ(mapData, lithoMap.getData());
}

TEST(LithofractionMapTest, TestAdditionOperation)
{
  std::vector<std::vector<double>> mapData = createConstantMap(7.5, 5, 10);
  std::vector<std::vector<double>> mapData2 = createConstantMap(16.4, 5, 10);

  casaWizard::LithofractionMap lithoMap1(mapData);
  casaWizard::LithofractionMap lithoMap2(mapData2);

  casaWizard::LithofractionMap resultMap = lithoMap1 + lithoMap2;

  for (int i = 0; i < lithoMap1.getData().size(); i++)
  {
    std::vector<double> mapRow;
    for (int j = 0; j < lithoMap1.getData()[0].size(); j++)
    {
      EXPECT_EQ(lithoMap1.getData()[i][j] + lithoMap2.getData()[i][j], resultMap.getData()[i][j]);
    }
  }
}

TEST(LithofractionMapTest, TestMultiplicationScalarOperation)
{
  std::vector<std::vector<double>> mapData = createConstantMap(7.5, 5, 10);

  casaWizard::LithofractionMap lithoMap1(mapData);
  casaWizard::LithofractionMap resultMap = lithoMap1 * -1.0;

  for (int i = 0; i < lithoMap1.getData().size(); i++)
  {
    std::vector<double> mapRow;
    for (int j = 0; j < lithoMap1.getData()[0].size(); j++)
    {
      EXPECT_EQ(lithoMap1.getData()[i][j] * -1.0, resultMap.getData()[i][j]);
    }
  }
}

TEST(LithofractionMapTest, TestAdditionScalarOperation)
{
  std::vector<std::vector<double>> mapData = createConstantMap(7.5, 5, 10);

  casaWizard::LithofractionMap lithoMap1(mapData);
  casaWizard::LithofractionMap resultMap = lithoMap1 + 1.0;

  for (int i = 0; i < lithoMap1.getData().size(); i++)
  {
    std::vector<double> mapRow;
    for (int j = 0; j < lithoMap1.getData()[0].size(); j++)
    {
      EXPECT_EQ(lithoMap1.getData()[i][j] + 1.0, resultMap.getData()[i][j]);
    }
  }
}

TEST(LithofractionMapTest, TestAdditionDifferentMapShapes)
{
  std::vector<std::vector<double>> mapData = createConstantMap(7.5, 5, 10);
  std::vector<std::vector<double>> mapData2 = createConstantMap(16.4, 10, 5);

  casaWizard::LithofractionMap lithoMap1(mapData);
  casaWizard::LithofractionMap lithoMap2(mapData2);

  casaWizard::LithofractionMap resultMap = lithoMap1 + lithoMap2;

  EXPECT_EQ(resultMap.getData().empty(), true);
}
