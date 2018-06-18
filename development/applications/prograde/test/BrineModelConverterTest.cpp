

#include <gtest/gtest.h>

#include "BrineModelConverter.h"


//cmbAPI
#include "FluidManager.h"


using namespace mbapi;

TEST(BrineModelConverter, upgrade)
{
	Prograde::BrineModelConverter modelConverter;
	std::string fluidName("test");
	ASSERT_EQ(FluidManager::Calculated, modelConverter.upgradeDensityModel(FluidManager::Calculated, fluidName) );
	ASSERT_EQ(FluidManager::Calculated, modelConverter.upgradeDensityModel(FluidManager::Constant, fluidName) );
	ASSERT_EQ(FluidManager::CalculatedModel, modelConverter.upgradeSeismicVelocityModel(FluidManager::CalculatedModel, fluidName) );
	ASSERT_EQ(FluidManager::CalculatedModel, modelConverter.upgradeSeismicVelocityModel(FluidManager::ConstantModel, fluidName) );
}
