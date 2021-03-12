#include "Genex0dSimulator.h"

#include "Genex0dSimulatorFactory.h"
#include "ObjectFactory.h"
#include "LangmuirAdsorptionIsothermSample.h"
#include "Interface.h"

#include <gtest/gtest.h>

class TestGenex0dSimulator : public ::testing::Test
{
protected:
  void SetUp() final
  {
    objectFactory = new Genex0d::Genex0dSimulatorFactory;
    simulator = Genex0d::Genex0dSimulator::CreateFrom("TestGenex0dSimulator.project3d", objectFactory);
  }

  void TearDown() final
  {
    delete simulator;
    delete objectFactory;
  }

  DataAccess::Interface::ObjectFactory * objectFactory;
  Genex0d::Genex0dSimulator* simulator;
};

TEST_F( TestGenex0dSimulator, TestSetLangmuirData )
{
  simulator->setLangmuirData("20 6.8 3.67 50 7.48 2.35 90 4.64 0.99 115 3.868571429 0.03286", "Default Langmuir Isotherm");
  DataAccess::Interface::LangmuirAdsorptionIsothermSampleList* list = simulator->getLangmuirAdsorptionIsothermSampleList("Default Langmuir Isotherm");

  ASSERT_EQ(list->size(), 4);

  EXPECT_DOUBLE_EQ(list->at(0)->getLangmuirTemperature(), 20.0);
  EXPECT_DOUBLE_EQ(list->at(1)->getLangmuirTemperature(), 50.0);
  EXPECT_DOUBLE_EQ(list->at(2)->getLangmuirTemperature(), 90.0);
  EXPECT_DOUBLE_EQ(list->at(3)->getLangmuirTemperature(), 115.0);

  EXPECT_DOUBLE_EQ(list->at(0)->getLangmuirPressure(), 6.8);
  EXPECT_DOUBLE_EQ(list->at(1)->getLangmuirPressure(), 7.48);
  EXPECT_DOUBLE_EQ(list->at(2)->getLangmuirPressure(), 4.64);
  EXPECT_DOUBLE_EQ(list->at(3)->getLangmuirPressure(), 3.868571429);

  EXPECT_DOUBLE_EQ(list->at(0)->getLangmuirVolume(), 3.67);
  EXPECT_DOUBLE_EQ(list->at(1)->getLangmuirVolume(), 2.35);
  EXPECT_DOUBLE_EQ(list->at(2)->getLangmuirVolume(), 0.99);
  EXPECT_DOUBLE_EQ(list->at(3)->getLangmuirVolume(), 0.03286);

  EXPECT_EQ(list->at(0)->getLangmuirName(), "Default Langmuir Isotherm");
}

TEST_F( TestGenex0dSimulator, TestSetLangmuirDataWithMissingData )
{
  simulator->setLangmuirData("20 6.8 3.67 50 7.48 2.35 90 4.64 0.99 115 3.868571429", "User Defined Isotherm");
  DataAccess::Interface::LangmuirAdsorptionIsothermSampleList* list = simulator->getLangmuirAdsorptionIsothermSampleList("User Defined Isotherm");

  EXPECT_EQ(list->size(), 3);
}




