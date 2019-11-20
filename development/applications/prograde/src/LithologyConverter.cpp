//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "LithologyConverter.h"

//utilities
#include "LogHandler.h"

//cmbAPI
#include "StratigraphyManager.h"

using namespace mbapi;
#include<iterator>

std::string Prograde::LithologyConverter::upgradeLithologyName(const std::string & legacyLithotypeName)
{
	std::string newLithoTypeNameAssigned;
	
		if (legacyLithotypeName.compare("Diorite/Granodiorite") == 0)
			newLithoTypeNameAssigned = "Diorite/Granodiorite (SI)";
		else if (legacyLithotypeName.compare("Gabbro/Dry basalt") == 0)
			newLithoTypeNameAssigned = "Dry Gabbro/Basalt (SI)";
		else if (legacyLithotypeName.compare("Gabbro/Wet basalt") == 0)
			newLithoTypeNameAssigned = "Granite/Rhyolite (SI)";
		else if (legacyLithotypeName.compare("Granite/Rhyolite") == 0)
			newLithoTypeNameAssigned = "Wet Gabbro/Basalt (SI)";
		else if ((legacyLithotypeName.compare("HEAT Chalk") == 0) || (legacyLithotypeName.compare("Std. Chalk") == 0))
			newLithoTypeNameAssigned = "Chalk, white";
		else if (legacyLithotypeName.compare("HEAT Dolostone") == 0)
			newLithoTypeNameAssigned = "Grainstone, dolomitic, typical";
		else if (legacyLithotypeName.compare("HEAT Limestone") == 0)
			newLithoTypeNameAssigned = "Grainstone, calcitic, typical";
		else if ((legacyLithotypeName.compare("HEAT Sandstone") == 0) || (legacyLithotypeName.compare("Std. Sandstone") == 0) || (legacyLithotypeName.compare("SM. Sandstone") == 0))
			newLithoTypeNameAssigned = "Sandstone, typical";
		else if (legacyLithotypeName.compare("SM.Mudst.40%Clay") == 0)
			newLithoTypeNameAssigned = "Mudstone, 40% clay";
		else if (legacyLithotypeName.compare("SM.Mudst.50%Clay") == 0)
			newLithoTypeNameAssigned = "Mudstone, 50% clay";
		else if ((legacyLithotypeName.compare("SM.Mudst.60%Clay") == 0) || (legacyLithotypeName.compare("Std. Shale") == 0) || (legacyLithotypeName.compare("HEAT Shale") == 0))
			newLithoTypeNameAssigned = "Mudstone, 60% clay";
		else if (legacyLithotypeName.compare("Standard Ice") == 0)
			newLithoTypeNameAssigned = "Ice";
		else if (legacyLithotypeName.compare("Std. Anhydrite") == 0)
			newLithoTypeNameAssigned = "Anhydrite";
		else if (legacyLithotypeName.compare("Std. Basalt") == 0)
			newLithoTypeNameAssigned = "Basalt, typical";
		else if (legacyLithotypeName.compare("Std. Coal") == 0)
			newLithoTypeNameAssigned = "Coal";
		else if (legacyLithotypeName.compare("Std. Marl") == 0)
			newLithoTypeNameAssigned = "Marl";
		else if (legacyLithotypeName.compare("Std. Salt") == 0)
			newLithoTypeNameAssigned = "Halite";
		else if (legacyLithotypeName.compare("Std. Siltstone") == 0)
			newLithoTypeNameAssigned = "Siltstone, 20% clay";
		else if (legacyLithotypeName.compare("Std.Dolo.Grainstone") == 0)
			newLithoTypeNameAssigned = "Grainstone, dolomitic, typical";
		else if (legacyLithotypeName.compare("Std.Dolo.Mudstone") == 0)
			newLithoTypeNameAssigned = "Lime-Mudstone, dolomitic";
		else if (legacyLithotypeName.compare("Std.Grainstone") == 0)
			newLithoTypeNameAssigned = "Grainstone, calcitic, typical";
		else if (legacyLithotypeName.compare("Std.Lime Mudstone") == 0)
			newLithoTypeNameAssigned = "Lime-Mudstone";
		else if (legacyLithotypeName.compare("Sylvite") == 0)
			newLithoTypeNameAssigned = "Potash-Sylvite";
		else if ((legacyLithotypeName.compare("Astheno. Mantle") == 0))//(legacyLithotypeName.compare("Litho. Mantle") == 0) ||
			newLithoTypeNameAssigned = "Mantle";
	else
			newLithoTypeNameAssigned = legacyLithotypeName;

	return newLithoTypeNameAssigned;
}
//This part needs further modifications as the requirement got changed which will be taken in the next sprint......hence commented for the time being 	
#if 0
std::vector<double> Prograde::LithologyConverter::computeSingleExpModelParameters(const std::string definedBy, const int lithologyFlag, mbapi::LithologyManager::PorosityModel &porModel, std::vector<double> & originalPorModelParam, std::vector<double> & newPorModelParam)
{
	std::string baseLithologyType;
	int pos = definedBy.find("|");
	baseLithologyType = definedBy.substr(pos + 1);
	pos = baseLithologyType.find("|");
	baseLithologyType = baseLithologyType.substr(pos + 1);

	double newSurfacePorosity;
	double newMinMechanicalPorosity;
	double newExponentialCompCoeff;

	if (lithologyFlag == 1 && porModel == mbapi::LithologyManager::PorosityModel::PorSoilMechanics)
	{

		if ((baseLithologyType.compare("HEAT Sandstone") == 0) || (baseLithologyType.compare("SM. Sandstone") == 0) || (baseLithologyType.compare("Std. Sandstone") == 0))
		{
			newSurfacePorosity = 45.0;
			newExponentialCompCoeff = 3.25;
			newMinMechanicalPorosity = 5.0;

			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Deprecated 'SoilMechanics' model is detected for this lithotype which is based on legacy "<< baseLithologyType<< ". Porosity model is upgraded to 'Exponential' model with default values of BPA2 'Sandstone,typical'. ";

		}
		else if (baseLithologyType.compare("Std. Siltstone") == 0)
		{
			newSurfacePorosity = 36.0;
			newExponentialCompCoeff = 5.5;
			newMinMechanicalPorosity = 4.0;

			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Deprecated 'SoilMechanics' model is detected for this lithotype which is based on legacy " << baseLithologyType << ". Porosity model is upgraded to 'Exponential' model with default values of BPA2 'Siltstone, clean'. ";
		}
		else {
			double param = originalPorModelParam[0];
			double param1 = originalPorModelParam[1];

			newSurfacePorosity = (0.0173*originalPorModelParam[0] * originalPorModelParam[0]) - (0.6878*originalPorModelParam[0]) + 27.73;
			newMinMechanicalPorosity = (0.00123*originalPorModelParam[0] * originalPorModelParam[0]) - (0.04913*originalPorModelParam[0]) + 4.34;
			newExponentialCompCoeff = (-1.508*originalPorModelParam[1] * originalPorModelParam[1]) + (2.526*originalPorModelParam[1]) + 5.05;

			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Deprecated 'SoilMechanics' model is detected for this lithotype which is based on legacy " << baseLithologyType << ". Porosity model is upgraded to 'Exponential' model with model parameters calculated from the imperical formula. ";
		}

		newPorModelParam.push_back(newSurfacePorosity);
		newPorModelParam.push_back(newExponentialCompCoeff);
		newPorModelParam.push_back(newMinMechanicalPorosity);

		porModel = mbapi::LithologyManager::PorosityModel::PorExponential;

	}
	else if (lithologyFlag == 0 && porModel == mbapi::LithologyManager::PorosityModel::PorSoilMechanics) {

		newPorModelParam = originalPorModelParam;

		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Deprecated 'SoilMechanics' model is detected for this lithotype. No upgradation of the porosity model is done as it is a standard lithology of BPA1. ";

	}
	else {

		newPorModelParam = originalPorModelParam;
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Latest porosity model is detected for this lithotype. No upgradation of the porosity model is needed. ";

	}

	return newPorModelParam;

}
#endif
	

	



