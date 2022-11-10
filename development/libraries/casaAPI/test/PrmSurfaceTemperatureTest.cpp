#include "../src/PrmSurfaceTemperature.h"

#include "../src/ScenarioAnalysis.h"
#include <gtest/gtest.h>

TEST(PrmSurfaceTemperatureTest, testAgeIsSnapshot)
{
   casa::ScenarioAnalysis scenario;
   scenario.defineBaseCase( "Ottoland.project3d" );
   mbapi::Model& baseModel = scenario.baseCase();

   casa::PrmSurfaceTemperature( baseModel, {"55"});
   ASSERT_EQ(ErrorHandler::NoError, baseModel.errorCode());
}

TEST(PrmSurfaceTemperatureTest, testAgeIsNotSnapshot)
{
   casa::ScenarioAnalysis scenario;
   scenario.defineBaseCase( "Ottoland.project3d" );
   mbapi::Model& baseModel = scenario.baseCase();

   casa::PrmSurfaceTemperature( baseModel, {"30"});
   ASSERT_EQ(ErrorHandler::OutOfRangeValue, baseModel.errorCode());
}

TEST(PrmSurfaceTemperatureTest, testBaseValueAtRow)
{
   casa::ScenarioAnalysis scenario;
   scenario.defineBaseCase( "Ottoland.project3d" );
   mbapi::Model& baseModel = scenario.baseCase();

   casa::PrmSurfaceTemperature parameter( baseModel, {"97"});

   ASSERT_EQ(ErrorHandler::NoError, baseModel.errorCode());

   ASSERT_DOUBLE_EQ(20, parameter.asDoubleArray()[0]);
}

TEST(PrmSurfaceTemperatureTest, testBaseValueBetweenRows)
{
   casa::ScenarioAnalysis scenario;
   scenario.defineBaseCase( "Ottoland.project3d" );
   mbapi::Model& baseModel = scenario.baseCase();

   casa::PrmSurfaceTemperature parameter( baseModel, {"55"});

   ASSERT_EQ(ErrorHandler::NoError, baseModel.errorCode());

   ASSERT_DOUBLE_EQ(15.670103092783506, parameter.asDoubleArray()[0]);
}

TEST(PrmSurfaceTemperatureTest, testBaseValueBeforeOldest)
{
   casa::ScenarioAnalysis scenario;
   scenario.defineBaseCase( "Ottoland.project3d" );
   mbapi::Model& baseModel = scenario.baseCase();

   casa::PrmSurfaceTemperature parameter( baseModel, {"220"});

   ASSERT_EQ(ErrorHandler::NoError, baseModel.errorCode());
   ASSERT_DOUBLE_EQ(20, parameter.asDoubleArray()[0]);

   const double temp = baseModel.tableValueAsDouble("SurfaceTempIoTbl", 2, "Temperature");
   const double age = baseModel.tableValueAsDouble("SurfaceTempIoTbl", 2, "Age");

   ASSERT_DOUBLE_EQ(20, temp);
   ASSERT_DOUBLE_EQ(220, age);
}
