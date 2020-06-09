#include <gtest/gtest.h>

#include "LithologyConverter.h"
#include "LithologyUpgradeManager.h"


//cmbAPI
#include "LithologyManager.h"
static const double eps = 0.00001;

using namespace mbapi;
using namespace std;

//Tests to validate whether lithology names are updated correctly or not
TEST(LithologyConverter, upgradeLithologyName)
{
   Prograde::LithologyConverter modelConverter;
   
   EXPECT_EQ("UserDefinedLitho", modelConverter.upgradeLithologyName("UserDefinedLitho"));
   EXPECT_EQ("Grainstone, dolomitic, typical", modelConverter.upgradeLithologyName("HEAT Dolostone"));

}

//Tests to validate permeability model parameters for system defined lithology
TEST(LithologyConverter, upgradePermModelForSysDefLitho)
{
   Prograde::LithologyConverter modelConverter;

   std::string lithoTypeName = "Crust";
   std::vector<double> mpPor; 
   std::vector<double> mpPerm;
   int numPts = 2;
   mpPor.resize(numPts);
   mpPerm.resize(numPts);

   // for standard lithotype name crust 
   modelConverter.upgradePermModelForSysDefLitho(lithoTypeName, mpPor, mpPerm, numPts);
   EXPECT_NEAR(5, mpPor[0], eps);
   EXPECT_NEAR(60, mpPor[1], eps);
   EXPECT_NEAR(-4, mpPerm[0], eps);
   EXPECT_NEAR(-4, mpPerm[1], eps);

   // for standard lithotype name Grainstone, calcitic, typical 
   lithoTypeName = "Grainstone, calcitic, typical";
   modelConverter.upgradePermModelForSysDefLitho(lithoTypeName, mpPor, mpPerm, numPts);
   EXPECT_NEAR(5, mpPor[0], eps);
   EXPECT_NEAR(60, mpPor[1], eps);
   EXPECT_NEAR(1.6, mpPerm[0], eps);
   EXPECT_NEAR(6.4, mpPerm[1], eps);

   // for standard lithotype name Lime-Mudstone 
   lithoTypeName = "Lime-Mudstone";
   modelConverter.upgradePermModelForSysDefLitho(lithoTypeName, mpPor, mpPerm, numPts);
   EXPECT_NEAR(5, mpPor[0], eps);
   EXPECT_NEAR(60, mpPor[1], eps);
   EXPECT_NEAR(-6.19, mpPerm[0], eps);
   EXPECT_NEAR(-1.28, mpPerm[1], eps);

}

//Tests to validate permeability model parameters for user defined lithology
TEST(LithologyConverter, upgradePermModelForUsrDefLitho)
{
   Prograde::LithologyConverter modelConverter;

   std::string permModelName = "Sands";
   std::vector<double> mpPor;
   std::vector<double> mpPerm;
   int numPts = 2;
   mpPor.resize(numPts);
   mpPerm.resize(numPts);

   // for permeability model name Sands 
   modelConverter.upgradePermModelForUsrDefLitho(permModelName, mpPor, mpPerm, numPts);
   EXPECT_NEAR(5, mpPor[0], eps);
   EXPECT_NEAR(60, mpPor[1], eps);
   EXPECT_NEAR(0.2, mpPerm[0], eps);
   EXPECT_NEAR(4.9, mpPerm[1], eps);

   // for permeability model name Shales 
   permModelName = "Shales";
   modelConverter.upgradePermModelForUsrDefLitho(permModelName, mpPor, mpPerm, numPts);
   EXPECT_NEAR(5, mpPor[0], eps);
   EXPECT_NEAR(60, mpPor[1], eps);
   EXPECT_NEAR(-6.4, mpPerm[0], eps);
   EXPECT_NEAR(-2.16, mpPerm[1], eps);

   // for permeability model name None 
   permModelName = "None";
   modelConverter.upgradePermModelForUsrDefLitho(permModelName, mpPor, mpPerm, numPts);
   EXPECT_NEAR(5, mpPor[0], eps);
   EXPECT_NEAR(60, mpPor[1], eps);
   EXPECT_NEAR(-6, mpPerm[0], eps);
   EXPECT_NEAR(-6, mpPerm[1], eps);

}

//Tests to validate the upgradation enforced by Prograde for the lithology descriptions 
TEST(LithologyConverter, upgradeLithologyDescription)
{
	Prograde::LithologyConverter modelConverter;

	std::string legacyDescription = "User defined lithology based on Std. Sandstone";
	std::string parentLithology = "Std. Sandstone";

	// upgrading descriptions for userDefined lithotype
	int userDefinedFlag = 1;
	std::string upgradedDescription = modelConverter.upgradeLithologyDescription(legacyDescription, userDefinedFlag, parentLithology);
	EXPECT_EQ("User defined lithology based on Std. Sandstone( Based on BPA Std. Sandstone)", upgradedDescription);

	// upgrading descriptions for standardlLithotype
	userDefinedFlag = 0;
	legacyDescription = "Std. Sandstone";
	upgradedDescription = modelConverter.upgradeLithologyDescription(legacyDescription, userDefinedFlag, parentLithology);
	EXPECT_EQ("Std. Sandstone", upgradedDescription);

}
//Tests to validate the upgradation of audit informations for a particular lithology 
TEST(LithologyConverter, upgradeLithologyAuditInfo)
{
	Prograde::LithologyConverter modelConverter;

	std::string legacyDefDate = "December 30 2019 15:22";
	std::string legacyUserInfo = "";
	std::string legacyLastModifiedDate = "December 30 2019 16:22";

	// checking for userDefined lithotype
	int userDefinedFlag = 1;
	modelConverter.upgradeLithologyAuditInfo(legacyDefDate, legacyUserInfo, legacyLastModifiedDate, userDefinedFlag);
	EXPECT_EQ("December 30 2019 15:22", legacyDefDate);
	EXPECT_EQ("Lorcan Kenan", legacyUserInfo);
	EXPECT_EQ("November 21 2019 12:00", legacyLastModifiedDate);

	// checking for standardlLithotype
	userDefinedFlag = 0;
	modelConverter.upgradeLithologyAuditInfo(legacyDefDate, legacyUserInfo, legacyLastModifiedDate, userDefinedFlag);
	EXPECT_EQ("November 21 2019 12:00", legacyDefDate);
	EXPECT_EQ("Lorcan Kenan", legacyUserInfo);
	EXPECT_EQ("November 21 2019 12:00", legacyLastModifiedDate);

}

//Tests to validate the upgradation of audit informations for a particular lithology 
TEST(LithologyConverter, findParentLithology)
{
	Prograde::LithologyConverter modelConverter;

	std::string legacyParentLithoDetails = "BPA REF INFO|Std. Basalt|Std. Basalt";//Parent lithology name is available
	std::string parentLithologyName = modelConverter.findParentLithology(legacyParentLithoDetails);
	EXPECT_EQ("Std. Basalt", parentLithologyName);

	legacyParentLithoDetails = "BPA REF INFO||CLAY"; //Parent lithology name is not available
	parentLithologyName = modelConverter.findParentLithology(legacyParentLithoDetails);
	EXPECT_EQ("", parentLithologyName);

	//Since parent lithology name is not found in the original inputs...it will be checked with the mapping which is based on description and/or lithology names
	std::string legacyDescription = "See \\n\\nBPA2_Reference_Lithology_Catalog_Signed_Off_latest_version_in_Sharepoint_dated_modified_December_2019.xlsx\\n\\nUse in most cases for rift filling basalt.";
	parentLithologyName = modelConverter.findMissingParentLithology("UserDefinedName", legacyDescription);
	EXPECT_EQ("Std. Basalt", parentLithologyName);

	legacyDescription = "Mix Lithology created from: Project Lithology Std. Basalt(90%); Project Lithology Std. Sandstone(10%); \\n\\nSlightly increased Surface Porosity\\nComp Coeff (Eff Stress) increased\\nMultipoint Perm Model Sm Sandstone\\nSeisVel=6500 instead of 5750\\n\\nHighest thermal conductivity";
	parentLithologyName = modelConverter.findMissingParentLithology("UserDefinedName", legacyDescription);
	EXPECT_EQ("Std. Basalt", parentLithologyName);
	parentLithologyName = modelConverter.findMissingParentLithology("SDR_basalt+volclastic_LK04022014 EH27102015 (high k)", legacyDescription);
	EXPECT_EQ("Basalt, SDR extrusive flows", parentLithologyName);

	legacyDescription = "Description not available in the mapping sheet";
	parentLithologyName = modelConverter.findMissingParentLithology("UserDefinedName", legacyDescription);
	EXPECT_EQ("", parentLithologyName);
	
}

//test to validate the upgradation of Soil Mechanics model to Exponential model for custom lithologies whose parent lithology contains Sandstone sub-string 
TEST(LithologyConverter, upgradePorosityModelForSandLitho)
{
	Prograde::LithologyConverter modelConverter;

	std::string baseLithoType = "Std. Sandstone";
	int lithologyFlag = 1;
	mbapi::LithologyManager::PorosityModel porModel = mbapi::LithologyManager::PorosityModel::PorSoilMechanics;//deprecated porosity model 

	std::vector<double> OriginalPorosityModelParam = { 62, 2 };
	std::vector<double> updatedPorModelParam;
	std::vector<double> actualPorModelParameter = { 45,3.25,5 };

	modelConverter.computeSingleExpModelParameters(baseLithoType, lithologyFlag, porModel, OriginalPorosityModelParam, updatedPorModelParam);

	EXPECT_EQ(mbapi::LithologyManager::PorosityModel::PorExponential, porModel);//Updated the porosity model from SM to Exponential
	EXPECT_EQ(actualPorModelParameter.size(), updatedPorModelParam.size());
	EXPECT_EQ(actualPorModelParameter[0], updatedPorModelParam[0]);
	EXPECT_EQ(actualPorModelParameter[1], updatedPorModelParam[1]);
	EXPECT_EQ(actualPorModelParameter[2], updatedPorModelParam[2]);
}

//test to validate the upgradation of Soil Mechanics model to Exponential model for custom lithologies whose parent lithology contains Siltstone sub-string
TEST(LithologyConverter, upgradePorosityModelForSiltLitho)
{
	Prograde::LithologyConverter modelConverter;

	std::string baseLithoType = "Std. Siltstone";
	int lithologyFlag = 1;
	mbapi::LithologyManager::PorosityModel porModel = mbapi::LithologyManager::PorosityModel::PorSoilMechanics;//Deprecated porosity model

	std::vector<double> OriginalPorosityModelParam = { 62, 2 };
	std::vector<double> updatedPorModelParam;
	std::vector<double> actualPorModelParameter = { 35,5.5,2 };

	modelConverter.computeSingleExpModelParameters(baseLithoType, lithologyFlag, porModel, OriginalPorosityModelParam, updatedPorModelParam);

	EXPECT_EQ(mbapi::LithologyManager::PorosityModel::PorExponential, porModel);//Updated the porosity model from SM to Exponential
	EXPECT_EQ(actualPorModelParameter.size(), updatedPorModelParam.size());
	EXPECT_EQ(actualPorModelParameter[0], updatedPorModelParam[0]);
	EXPECT_EQ(actualPorModelParameter[1], updatedPorModelParam[1]);
	EXPECT_EQ(actualPorModelParameter[2], updatedPorModelParam[2]);
	OriginalPorosityModelParam.clear();
	updatedPorModelParam.clear();
	actualPorModelParameter.clear();

}
//test to validate the upgradation of Soil Mechanics model to Exponential model for custom lithologies whose parent lithologie contains sub-strings “shale” or “mudst
TEST(LithologyConverter, upgradePorosityModelForMudstoneLitho)
{
	Prograde::LithologyConverter modelConverter;

	std::string baseLithoType = "SM.Mudst.50%Clay";
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

}
//test to validate the upgradation of Soil Mechanics model to Exponential model for custom lithologies whose parent lithologie are carbonate
TEST(LithologyConverter, upgradePorosityModelForCarbonateLitho)
{
	Prograde::LithologyConverter modelConverter;

	std::string baseLithoType = "Std.Dolo.Mudstone";
	int lithologyFlag = 1;
	mbapi::LithologyManager::PorosityModel porModel = mbapi::LithologyManager::PorosityModel::PorSoilMechanics;

	std::vector<double> OriginalPorosityModelParam = { 62, 1.9 };
	std::vector<double> updatedPorModelParam;
	std::vector<double> actualPorModelParameter = { 35,6,3 };

	modelConverter.computeSingleExpModelParameters(baseLithoType, lithologyFlag, porModel, OriginalPorosityModelParam, updatedPorModelParam);

	EXPECT_EQ(mbapi::LithologyManager::PorosityModel::PorExponential, porModel);//Updated the porosity model from SM to Exponential
	EXPECT_EQ(actualPorModelParameter.size(), updatedPorModelParam.size());
	EXPECT_EQ(actualPorModelParameter[0], updatedPorModelParam[0]);
	EXPECT_EQ(actualPorModelParameter[1], updatedPorModelParam[1]);
	EXPECT_EQ(actualPorModelParameter[2], updatedPorModelParam[2]);

}
//test to validate the upgradation of porosity model for custom lithologies which are based on the other parent lithologies for whom mapping is not available
TEST(LithologyConverter, upgradePorosityModelForUnknownLitho)
{
	Prograde::LithologyConverter modelConverter;

	std::string baseLithoType = "Std. Anhydrite";
	int lithologyFlag = 1;
	mbapi::LithologyManager::PorosityModel porModel = mbapi::LithologyManager::PorosityModel::PorSoilMechanics;

	std::vector<double> OriginalPorosityModelParam = { 62, 1.9 };
	std::vector<double> updatedPorModelParam;
	std::vector<double> actualPorModelParameter = { 62,1.9 };

	modelConverter.computeSingleExpModelParameters(baseLithoType, lithologyFlag, porModel, OriginalPorosityModelParam, updatedPorModelParam);

	EXPECT_EQ(mbapi::LithologyManager::PorosityModel::PorSoilMechanics, porModel);//Updated the porosity model from SM to Exponential
	EXPECT_EQ(actualPorModelParameter.size(), updatedPorModelParam.size());
	EXPECT_EQ(actualPorModelParameter[0], updatedPorModelParam[0]);
	EXPECT_EQ(actualPorModelParameter[1], updatedPorModelParam[1]);
}

//test to validate the upgradation of Soil Mechanics model to Exponential model for standard lithologies
TEST(LithologyConverter, upgradePorosityModelForStandardLithoWithSoilMechanicsModel)
{
	Prograde::LithologyConverter modelConverter;

	std::string baseLithoType = "Std. Sandstone";
	int lithologyFlag = 0;
	mbapi::LithologyManager::PorosityModel porModel = mbapi::LithologyManager::PorosityModel::PorSoilMechanics;

	std::vector<double> OriginalPorosityModelParam = { 25, 0.2546 };
	std::vector<double> updatedPorModelParam;
	std::vector<double> actualPorModelParameter = { 25,0.2546 };

	modelConverter.computeSingleExpModelParameters(baseLithoType, lithologyFlag, porModel, OriginalPorosityModelParam, updatedPorModelParam);

	EXPECT_EQ(mbapi::LithologyManager::PorosityModel::PorSoilMechanics, porModel);//Updated the porosity model from SM to Exponential
	EXPECT_EQ(actualPorModelParameter.size(), updatedPorModelParam.size());
	EXPECT_EQ(actualPorModelParameter[0], updatedPorModelParam[0]);
	EXPECT_EQ(actualPorModelParameter[1], updatedPorModelParam[1]);
}

// test to check whether the porosity model parameters are upgraded correctly for standard lithology having the Exponential porosity model
TEST(LithologyConverter, upgradePorosityModelForStandardLithoWithExpModel)
{
	Prograde::LithologyConverter modelConverter;

	std::string baseLithoType = "Std. Sandstone";
	int lithologyFlag = 0;
	mbapi::LithologyManager::PorosityModel porModel = mbapi::LithologyManager::PorosityModel::PorExponential;

	std::vector<double> OriginalPorosityModelParam = { 31, 1.25, 1.5 };//SurfacePor=31, CompCoeffES=1.25, MinPor=1.5
	std::vector<double> updatedPorModelParam;
	std::vector<double> actualPorModelParameter = { 31,1.25,1.5 };

	modelConverter.computeSingleExpModelParameters(baseLithoType, lithologyFlag, porModel, OriginalPorosityModelParam, updatedPorModelParam);

	EXPECT_EQ(mbapi::LithologyManager::PorosityModel::PorExponential, porModel);//Updated the porosity model from SM to Exponential
	EXPECT_EQ(actualPorModelParameter.size(), updatedPorModelParam.size());
	EXPECT_EQ(actualPorModelParameter[0], updatedPorModelParam[0]);
	EXPECT_EQ(actualPorModelParameter[1], updatedPorModelParam[1]);
	EXPECT_EQ(actualPorModelParameter[2], updatedPorModelParam[2]);

	OriginalPorosityModelParam.clear();
	updatedPorModelParam.clear();
	actualPorModelParameter.clear();

	//For exponential model with legacy parameters values outside the allowed range of BPA2..
	//Not updated in Prograde as all the parameters for Standard lithologies will automatically updated from BPA2 reference catalog
	OriginalPorosityModelParam = { 100.1, 25.1, 0 };//SurfacePor=100.0, CompCoeffES=25.1, MinPor=0

	actualPorModelParameter = { 100.1,25.1,0 };
	
	modelConverter.computeSingleExpModelParameters(baseLithoType, lithologyFlag, porModel, OriginalPorosityModelParam, updatedPorModelParam);

	EXPECT_EQ(mbapi::LithologyManager::PorosityModel::PorExponential, porModel);//Updated the porosity model from SM to Exponential
	EXPECT_EQ(actualPorModelParameter.size(), updatedPorModelParam.size());
	EXPECT_EQ(actualPorModelParameter[0], updatedPorModelParam[0]);
	EXPECT_EQ(actualPorModelParameter[1], updatedPorModelParam[1]);
	EXPECT_EQ(actualPorModelParameter[2], updatedPorModelParam[2]);
}
// test to check whether the porosity model parameters are upgraded correctly for custom lithology having the Exponential porosity model
TEST(LithologyConverter, upgradePorosityModelForCustomLithoWithExpModel)
{
	Prograde::LithologyConverter modelConverter;

	std::string baseLithoType = "Std. Sandstone";
	int lithologyFlag = 1;
	mbapi::LithologyManager::PorosityModel porModel = mbapi::LithologyManager::PorosityModel::PorExponential;

	std::vector<double> OriginalPorosityModelParam = { 31, 1.25, 1.5 };
	std::vector<double> updatedPorModelParam;
	std::vector<double> actualPorModelParameter = { 31,1.25,1.5 };

	modelConverter.computeSingleExpModelParameters(baseLithoType, lithologyFlag, porModel, OriginalPorosityModelParam, updatedPorModelParam);

	EXPECT_EQ(mbapi::LithologyManager::PorosityModel::PorExponential, porModel);//Updated the porosity model from SM to Exponential
	EXPECT_EQ(actualPorModelParameter.size(), updatedPorModelParam.size());
	EXPECT_EQ(actualPorModelParameter[0], updatedPorModelParam[0]);
	EXPECT_EQ(actualPorModelParameter[1], updatedPorModelParam[1]);
	EXPECT_EQ(actualPorModelParameter[2], updatedPorModelParam[2]);

	OriginalPorosityModelParam.clear();
	updatedPorModelParam.clear();
	actualPorModelParameter.clear();

	OriginalPorosityModelParam = { 100.1, 25.1, 0 };
	actualPorModelParameter = { 100,25.0,0.1 };
	
	modelConverter.computeSingleExpModelParameters(baseLithoType, lithologyFlag, porModel, OriginalPorosityModelParam, updatedPorModelParam);

	EXPECT_EQ(mbapi::LithologyManager::PorosityModel::PorExponential, porModel);//Updated the porosity model from SM to Exponential
	EXPECT_EQ(actualPorModelParameter.size(), updatedPorModelParam.size());
	EXPECT_EQ(actualPorModelParameter[0], updatedPorModelParam[0]);
	EXPECT_EQ(actualPorModelParameter[1], updatedPorModelParam[1]);
	EXPECT_EQ(actualPorModelParameter[2], updatedPorModelParam[2]);
}

//Tests to check and update lithotype property values are in proposed range in LithotypeIoTbl field
TEST(LithologyConverter, upgradeLithoTableFieldValues)
{
   Prograde::LithologyConverter modelConverter;
   double value;
   // Density
   // value with in the range 
   value = 2800;
   modelConverter.upgradeLitPropDensity(value);
   EXPECT_EQ(2800, value);
   // value less than lower limit; set it to lower limit  
   value = 450;
   modelConverter.upgradeLitPropDensity(value);
   EXPECT_EQ(500, value);
   // value higher than upper limit; set it to upper limit  
   value = 11000;
   modelConverter.upgradeLitPropDensity(value);
   EXPECT_EQ(10000, value);
   // Heat Production
   // value with in the range 
   value = 20;
   modelConverter.upgradeLitPropHeatProduction(value);
   EXPECT_EQ(20, value);
   // value less than lower limit; set it to lower limit  
   value = -1;
   modelConverter.upgradeLitPropHeatProduction(value);
   EXPECT_EQ(0, value);
   // value higher than upper limit; set it to upper limit  
   value = 30;
   modelConverter.upgradeLitPropHeatProduction(value);
   EXPECT_EQ(25, value);
   // Thermal Conductivity
   // value with in the range 
   value = 7;
   modelConverter.upgradeLitPropThrConductivity(value);
   EXPECT_EQ(7, value);
   // value less than lower limit; set it to lower limit  
   value = -1;
   modelConverter.upgradeLitPropThrConductivity(value);
   EXPECT_EQ(0, value);
   // value higher than upper limit; set it to upper limit  
   value = 15;
   modelConverter.upgradeLitPropThrConductivity(value);
   EXPECT_EQ(10, value);
   // Thermal Conductivity Anistropy
   // value with in the range 
   value = 7;
   modelConverter.upgradeLitPropThrCondAnistropy(value);
   EXPECT_EQ(7, value);
   // value less than lower limit; set it to lower limit  
   value = -1;
   modelConverter.upgradeLitPropThrCondAnistropy(value);
   EXPECT_EQ(0, value);
   // value higher than upper limit; set it to upper limit  
   value = 15;
   modelConverter.upgradeLitPropThrCondAnistropy(value);
   EXPECT_EQ(10, value);
   // Permeability Anistropy
   // value with in the range 
   value = 7;
   modelConverter.upgradeLitPropPermAnistropy(value);
   EXPECT_EQ(7, value);
   // value less than lower limit; set it to lower limit  
   value = -1;
   modelConverter.upgradeLitPropPermAnistropy(value);
   EXPECT_EQ(0, value);
   // value higher than upper limit; set it to upper limit  
   value = 115;
   modelConverter.upgradeLitPropPermAnistropy(value);
   EXPECT_EQ(100, value);
   // Seismic Velocity
   // value with in the range 
   value = 1200;
   modelConverter.upgradeLitPropSeisVelocity(value);
   EXPECT_EQ(1200, value);
   // value less than lower limit; set it to lower limit  
   value = 900;
   modelConverter.upgradeLitPropSeisVelocity(value);
   EXPECT_EQ(1000, value);
   // value higher than upper limit; set it to upper limit  
   value = 9500;
   modelConverter.upgradeLitPropSeisVelocity(value);
   EXPECT_EQ(9000, value);
   // Seismic Velocity Exponent
   // value with in the range 
   value = 0;
   modelConverter.upgradeLitPropSeisVeloExponent(value);
   EXPECT_EQ(0, value);
   // value less than lower limit; set it to lower limit  
   value = -2;
   modelConverter.upgradeLitPropSeisVeloExponent(value);
   EXPECT_EQ(-1, value);
   // value higher than upper limit; set it to upper limit  
   value = 2;
   modelConverter.upgradeLitPropSeisVeloExponent(value);
   EXPECT_EQ(1, value);
   // Entry Pressure Coefficient 1
   // value with in the range 
   value = 1;
   modelConverter.upgradeLitPropEntryPresCoeff1(value);
   EXPECT_EQ(1, value);
   // value less than lower limit; set it to lower limit  
   value = 0.01;
   modelConverter.upgradeLitPropEntryPresCoeff1(value);
   EXPECT_EQ(0.1, value);
   // value higher than upper limit; set it to upper limit  
   value = 3;
   modelConverter.upgradeLitPropEntryPresCoeff1(value);
   EXPECT_EQ(2, value);
   // Entry Pressure Coefficient 2
   // value with in the range 
   value = 1;
   modelConverter.upgradeLitPropEntryPresCoeff2(value);
   EXPECT_EQ(1, value);
   // value less than lower limit; set it to lower limit  
   value = -2.5;
   modelConverter.upgradeLitPropEntryPresCoeff2(value);
   EXPECT_EQ(-1.5, value);
   // value higher than upper limit; set it to upper limit  
   value = 3;
   modelConverter.upgradeLitPropEntryPresCoeff2(value);
   EXPECT_EQ(1.5, value);
   // Hydraulic Fracturing
   // value with in the range 
   value = 10;
   modelConverter.upgradeLitPropHydFracturing(value);
   EXPECT_EQ(10, value);
   // value less than lower limit; set it to lower limit  
   value = -2.5;
   modelConverter.upgradeLitPropHydFracturing(value);
   EXPECT_EQ(0, value);
   // value higher than upper limit; set it to upper limit  
   value = 110;
   modelConverter.upgradeLitPropHydFracturing(value);
   EXPECT_EQ(100, value);
   // ReferenceSolidViscosity
   // value with in the range 
   value = 1e17;
   modelConverter.upgradeLitPropRefSoldViscosity(value);
   EXPECT_EQ(1e17, value);
   // value less than lower limit; set it to lower limit  
   value = -2.5;
   modelConverter.upgradeLitPropRefSoldViscosity(value);
   EXPECT_EQ(0, value);
   // value higher than upper limit; set it to upper limit  
   value = 1e19;
   modelConverter.upgradeLitPropRefSoldViscosity(value);
   EXPECT_EQ(1e18, value);
   // Intrusion Temperature
   // value with in the range 
   value = 700;
   modelConverter.upgradeLitPropIntrTemperature(value);
   EXPECT_EQ(700, value);
   // value less than lower limit; set it to lower limit  
   value = 500;
   modelConverter.upgradeLitPropIntrTemperature(value);
   EXPECT_EQ(600, value);
   // value higher than upper limit; set it to upper limit  
   value = 1800;
   modelConverter.upgradeLitPropIntrTemperature(value);
   EXPECT_EQ(1650, value);
}
//Tests to check and update lithotype inputs of StratIoTbl
TEST(LithologyConverter, preprocessingLithoDataTest)
{
	//Test-1: Positive scenario with correct legacy inputs for lithology percenteges. This needs modification of the lithotype names only 
	std::string layerName{"Formation1"};
	std::vector<std::string> legacyLithoNames{"Std. Sandstone", "Std. Shale", "User defined litho"};
	std::vector<double> lithoPercnt{ 70, 20, 10 };
	std::vector<std::string> lithoPercntMap{"", ""};
	Prograde::LithologyConverter modelConverter;
	ErrorHandler::ReturnCode er = modelConverter.PreprocessLithofaciesInputOfStratIoTbl(legacyLithoNames, lithoPercnt, lithoPercntMap);
	EXPECT_EQ(ErrorHandler::ReturnCode::NoError, er);
	EXPECT_EQ(3, legacyLithoNames.size()); 
	EXPECT_EQ(3, lithoPercnt.size()); 
	EXPECT_EQ(2, lithoPercntMap.size());

	EXPECT_EQ("Sandstone, typical", legacyLithoNames[0]);
	EXPECT_EQ("Mudstone, 60% clay", legacyLithoNames[1]);
	EXPECT_EQ("User defined litho", legacyLithoNames[2]);

	EXPECT_EQ(70, lithoPercnt[0]);
	EXPECT_EQ(20, lithoPercnt[1]);
	EXPECT_EQ(10, lithoPercnt[2]);

	EXPECT_EQ("", lithoPercntMap[0]);
	EXPECT_EQ("", lithoPercntMap[1]);
	
	//Test-2: Legacy inputs where lithology percentages are available even when lithotype is not available. This needs modification for the lithology percentages also along with the modifications in the lithology names
	std::vector<std::string> legacyLithoNames1{ "Std. Sandstone", "", "" };
	er = modelConverter.PreprocessLithofaciesInputOfStratIoTbl(legacyLithoNames1, lithoPercnt, lithoPercntMap);
	EXPECT_EQ(ErrorHandler::ReturnCode::NoError, er);

	EXPECT_EQ("Sandstone, typical", legacyLithoNames1[0]);
	EXPECT_EQ("", legacyLithoNames1[1]);
	EXPECT_EQ("", legacyLithoNames1[2]);

	EXPECT_EQ(70, lithoPercnt[0]);
	EXPECT_EQ(-9999, lithoPercnt[1]);
	EXPECT_EQ(-9999, lithoPercnt[2]);

	//Test-3: Eronious inputs having specified more than 3 lithotypes for a particular layer which will be rejected from migration 
	std::vector<std::string> legacyLithoNames2{ "Std. Sandstone", "", "", "Std. Shale" };
	er = modelConverter.PreprocessLithofaciesInputOfStratIoTbl(legacyLithoNames2, lithoPercnt, lithoPercntMap);
	EXPECT_EQ(ErrorHandler::ReturnCode::ValidationError, er);

	//Test-4: Eronious inputs having all the 3 lithotypes NULL wrt a particular layer will be rejected from migration
	std::vector<std::string> legacyLithoNames3{ "", "", "" };
	er = modelConverter.PreprocessLithofaciesInputOfStratIoTbl(legacyLithoNames3, lithoPercnt, lithoPercntMap);
	EXPECT_EQ(ErrorHandler::ReturnCode::ValidationError, er);

	//Test-4: Eronious inputs having which will be rejected from migration
	std::vector<std::string> legacyLithoNames4{ "", "" };
	er = modelConverter.PreprocessLithofaciesInputOfStratIoTbl(legacyLithoNames4, lithoPercnt, lithoPercntMap);
	EXPECT_EQ(ErrorHandler::ReturnCode::ValidationError, er);

	//Test-5: Eronious legacy inputs where both maps and scalar values are available for each lithotypes
	std::vector<std::string> lithoPercntMap1{ "MAP1", "MAP2" };
	er = modelConverter.PreprocessLithofaciesInputOfStratIoTbl(legacyLithoNames, lithoPercnt, lithoPercntMap1);
	EXPECT_EQ(ErrorHandler::ReturnCode::NoError, er);
	EXPECT_EQ("Sandstone, typical", legacyLithoNames[0]);
	EXPECT_EQ("Mudstone, 60% clay", legacyLithoNames[1]);
	EXPECT_EQ("User defined litho", legacyLithoNames[2]);

	EXPECT_EQ(-9999, lithoPercnt[0]);
	EXPECT_EQ(-9999, lithoPercnt[1]);
	EXPECT_EQ(-9999, lithoPercnt[2]);

	EXPECT_EQ("MAP1", lithoPercntMap1[0]);
	EXPECT_EQ("MAP2", lithoPercntMap1[1]);
}