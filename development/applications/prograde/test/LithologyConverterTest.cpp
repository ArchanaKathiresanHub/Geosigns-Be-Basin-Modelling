#include <gtest/gtest.h>

#include "LithologyConverter.h"


//cmbAPI
#include "LithologyManager.h"
static const double eps = 0.00001;

using namespace mbapi;

//Tests to validate whether lithology names are updated correctly or not
TEST(LithologyConverter, upgradeLithologyName)
{
   Prograde::LithologyConverter modelConverter;
   
   EXPECT_EQ("UserDefinedLitho", modelConverter.upgradeLithologyName("UserDefinedLitho"));
   EXPECT_EQ("Grainstone, dolomitic, typical", modelConverter.upgradeLithologyName("HEAT Dolostone"));

}
//These test cases needs further modifications as the strategy for POROSITY model upgradation has changed which will be taken in the next sprint......hence commented for the time being
#if 0
//test to check whether the porosity model parameters are upgraded correctly from Soil Mechanics to Exponential model, if the parent lithology contains the string “sand” 
TEST(LithologyConverter, upgradePorosityModelForSandLitho)
{
	Prograde::LithologyConverter modelConverter;
	
	std::string baseLithoType = "BPA REF INFO|Std. Sandstone|Std. Sandstone";
	int lithologyFlag = 1;
	mbapi::LithologyManager::PorosityModel porModel = mbapi::LithologyManager::PorosityModel::PorSoilMechanics;//porosity model found in the project file

	std::vector<double> OriginalPorosityModelParam = { 62, 2 };
	std::vector<double> updatedPorModelParam;
	std::vector<double> actualPorModelParameter;

	actualPorModelParameter.push_back(45);
	actualPorModelParameter.push_back(3.25);
	actualPorModelParameter.push_back(5);

	modelConverter.computeSingleExpModelParameters(baseLithoType, lithologyFlag, porModel, OriginalPorosityModelParam, updatedPorModelParam);

	EXPECT_EQ(mbapi::LithologyManager::PorosityModel::PorExponential, porModel);//Updated the porosity model from SM to Exponential
	EXPECT_EQ(actualPorModelParameter.size(), updatedPorModelParam.size());
	EXPECT_NEAR(actualPorModelParameter[0], updatedPorModelParam[0], eps);
	EXPECT_NEAR(actualPorModelParameter[1], updatedPorModelParam[1], eps);
	EXPECT_NEAR(actualPorModelParameter[2], updatedPorModelParam[2], eps);

	OriginalPorosityModelParam.clear();
	updatedPorModelParam.clear();
	actualPorModelParameter.clear();
}

//test to check whether the porosity model parameters are upgraded correctly from Soil Mechanics to Exponential model, if the parent lithology contains the string “silt” 
TEST(LithologyConverter, upgradePorosityModelForSiltLitho)
{
	Prograde::LithologyConverter modelConverter;

	std::string baseLithoType = "BPA REF INFO|Std. Siltstone|Std. Siltstone";
	int lithologyFlag = 1;
	mbapi::LithologyManager::PorosityModel porModel = mbapi::LithologyManager::PorosityModel::PorSoilMechanics;

	std::vector<double> OriginalPorosityModelParam = { 62, 2 };
	std::vector<double> updatedPorModelParam;
	std::vector<double> actualPorModelParameter;

	actualPorModelParameter.push_back(36);
	actualPorModelParameter.push_back(5.5);
	actualPorModelParameter.push_back(4);

	modelConverter.computeSingleExpModelParameters(baseLithoType, lithologyFlag, porModel, OriginalPorosityModelParam, updatedPorModelParam);

	EXPECT_EQ(mbapi::LithologyManager::PorosityModel::PorExponential, porModel);//Updated the porosity model from SM to Exponential
	EXPECT_EQ(actualPorModelParameter.size(), updatedPorModelParam.size());
	EXPECT_NEAR(actualPorModelParameter[0], updatedPorModelParam[0], eps);
	EXPECT_NEAR(actualPorModelParameter[1], updatedPorModelParam[1], eps);
	EXPECT_NEAR(actualPorModelParameter[2], updatedPorModelParam[2], eps);
	OriginalPorosityModelParam.clear();
	updatedPorModelParam.clear();
	actualPorModelParameter.clear();

}

//test to check whether the porosity model parameters are upgraded correctly from Soil Mechanics to Exponential model, if the user defined lithology is based not on "sand" or "silt"
TEST(LithologyConverter, upgradePorosityModelForOtherLitho)
{
	Prograde::LithologyConverter modelConverter;

	std::string baseLithoType = "BPA REF INFO|SM.Mudst.50%Clay|SM.Mudst.50%Clay";
	int lithologyFlag = 1;
	mbapi::LithologyManager::PorosityModel porModel = mbapi::LithologyManager::PorosityModel::PorSoilMechanics;

	std::vector<double> OriginalPorosityModelParam = { 62, 2 };
	std::vector<double> updatedPorModelParam;
	std::vector<double> actualPorModelParameter;

	actualPorModelParameter.push_back((0.0173*OriginalPorosityModelParam[0] * OriginalPorosityModelParam[0]) - (0.6878*OriginalPorosityModelParam[0]) + 27.73);
	actualPorModelParameter.push_back((-1.508*OriginalPorosityModelParam[1] * OriginalPorosityModelParam[1]) + (2.526*OriginalPorosityModelParam[1]) + 5.05);
	actualPorModelParameter.push_back((0.00123*OriginalPorosityModelParam[0] * OriginalPorosityModelParam[0]) - (0.04913*OriginalPorosityModelParam[0]) + 4.34);

	modelConverter.computeSingleExpModelParameters(baseLithoType, lithologyFlag, porModel, OriginalPorosityModelParam, updatedPorModelParam);

	EXPECT_EQ(mbapi::LithologyManager::PorosityModel::PorExponential, porModel);//Updated the porosity model from SM to Exponential
	EXPECT_EQ(actualPorModelParameter.size(), updatedPorModelParam.size());
	ASSERT_NEAR(actualPorModelParameter[0], updatedPorModelParam[0], eps);
	ASSERT_NEAR(actualPorModelParameter[1], updatedPorModelParam[1], eps);
	ASSERT_NEAR(actualPorModelParameter[2], updatedPorModelParam[2], eps);

	OriginalPorosityModelParam.clear();
	updatedPorModelParam.clear();
	actualPorModelParameter.clear();

}
//test to check whether the porosity model parameters are upgraded correctly, if the lithotype is not a user defined lithology and having the SoilMechanics porosity model
TEST(LithologyConverter, upgradePorosityModelForStandardLithoWithSoilMechanicsModel)
{
	Prograde::LithologyConverter modelConverter;
	
	std::string baseLithoType = "BPA REF INFO|Std. Sandstone|Std. Sandstone";
	int lithologyFlag = 0;
	mbapi::LithologyManager::PorosityModel porModel = mbapi::LithologyManager::PorosityModel::PorSoilMechanics;

	std::vector<double> OriginalPorosityModelParam = { 25, 0.2546 };
	std::vector<double> updatedPorModelParam;
	std::vector<double> actualPorModelParameter;

	actualPorModelParameter.push_back(25);
	actualPorModelParameter.push_back(0.2546);

	modelConverter.computeSingleExpModelParameters(baseLithoType, lithologyFlag, porModel, OriginalPorosityModelParam, updatedPorModelParam);

	EXPECT_EQ(mbapi::LithologyManager::PorosityModel::PorSoilMechanics, porModel);//Updated the porosity model from SM to Exponential
	EXPECT_EQ(actualPorModelParameter.size(), updatedPorModelParam.size());
	EXPECT_NEAR(actualPorModelParameter[0], updatedPorModelParam[0], eps);
	EXPECT_NEAR(actualPorModelParameter[1], updatedPorModelParam[1], eps);

	OriginalPorosityModelParam.clear();
	updatedPorModelParam.clear();
	actualPorModelParameter.clear();
}

// test to check whether the porosity model parameters are upgraded correctly if the lithotype is not a user defined lithology and having the Exponential porosity model
TEST(LithologyConverter, upgradePorosityModelForStandardLithoWithExpModel)
{
	Prograde::LithologyConverter modelConverter;

	std::string baseLithoType = "BPA REF INFO|Std. Sandstone|Std. Sandstone";
	int lithologyFlag = 0;
	mbapi::LithologyManager::PorosityModel porModel = mbapi::LithologyManager::PorosityModel::PorExponential;

	std::vector<double> OriginalPorosityModelParam = { 31, 1.25, 1.5 };
	std::vector<double> updatedPorModelParam;
	std::vector<double> actualPorModelParameter;

	actualPorModelParameter.push_back(31);
	actualPorModelParameter.push_back(1.25);
	actualPorModelParameter.push_back(1.5);

	modelConverter.computeSingleExpModelParameters(baseLithoType, lithologyFlag, porModel, OriginalPorosityModelParam, updatedPorModelParam);

	EXPECT_EQ(mbapi::LithologyManager::PorosityModel::PorExponential, porModel);//Updated the porosity model from SM to Exponential
	EXPECT_EQ(actualPorModelParameter.size(), updatedPorModelParam.size());
	EXPECT_NEAR(actualPorModelParameter[0], updatedPorModelParam[0], eps);
	EXPECT_NEAR(actualPorModelParameter[1], updatedPorModelParam[1], eps);
	EXPECT_NEAR(actualPorModelParameter[2], updatedPorModelParam[2], eps);

	OriginalPorosityModelParam.clear();
	updatedPorModelParam.clear();
	actualPorModelParameter.clear(); 
}
#endif