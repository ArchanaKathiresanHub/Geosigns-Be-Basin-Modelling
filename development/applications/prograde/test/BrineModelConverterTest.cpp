

#include <gtest/gtest.h>

#include "BrineModelConverter.h"


//cmbAPI
#include "FluidManager.h"


using namespace mbapi;

TEST(BrineModelConverter, upgrade)
{
   Prograde::BrineModelConverter modelConverter;
   std::string stdMarineWater("Std. Marine Water");
   std::string fluidName("test");
   EXPECT_EQ(1, modelConverter.upgradeUserDefined(stdMarineWater, 0));
   EXPECT_EQ(1, modelConverter.upgradeUserDefined(stdMarineWater, 1));
   EXPECT_EQ(0, modelConverter.upgradeUserDefined(fluidName, 0));
   EXPECT_EQ(1, modelConverter.upgradeUserDefined(fluidName, 1));
   EXPECT_EQ("KSEPL's Deprecated Marine Water", modelConverter.upgradeDescription(stdMarineWater, "KSEPL's Standard Marine Water"));
   EXPECT_EQ("KSEPL's Deprecated Marine Water", modelConverter.upgradeDescription(stdMarineWater, "KSEPL's Deprecated Marine Water"));
   EXPECT_EQ("test description", modelConverter.upgradeDescription(fluidName, "test description"));
   EXPECT_EQ(FluidManager::Calculated, modelConverter.upgradeDensityModel(FluidManager::Calculated, fluidName) );
   EXPECT_EQ(FluidManager::Calculated, modelConverter.upgradeDensityModel(FluidManager::Constant, fluidName) );
   EXPECT_EQ(FluidManager::CalculatedModel, modelConverter.upgradeSeismicVelocityModel(FluidManager::CalculatedModel, fluidName) );
   EXPECT_EQ(FluidManager::CalculatedModel, modelConverter.upgradeSeismicVelocityModel(FluidManager::ConstantModel, fluidName) );
}
