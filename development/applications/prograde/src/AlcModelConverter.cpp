//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "AlcModelConverter.h"

//utilities
#include "LogHandler.h"

//cmbAPI
#include "BottomBoundaryManager.h"

using namespace mbapi;

BottomBoundaryManager::CrustPropertyModel Prograde::AlcModelConverter::upgradeAlcCrustPropModel(const BottomBoundaryManager::CrustPropertyModel CrPropModel) const {
	BottomBoundaryManager::CrustPropertyModel myCrPropModel = CrPropModel;
	switch (CrPropModel) {
	case BottomBoundaryManager::CrustPropertyModel::LegacyCrust:
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> Legacy crust model detected; Crust property model upgraded to Standard conductivity model";
		myCrPropModel = BottomBoundaryManager::CrustPropertyModel::StandardCondModel;;
		break;
	}
	case BottomBoundaryManager::CrustPropertyModel::LowCondModel:
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> Low conductivity crust model detected; Crust property model upgraded to Standard conductivity model";
		myCrPropModel = BottomBoundaryManager::CrustPropertyModel::StandardCondModel;
		break;
	}
	case BottomBoundaryManager::CrustPropertyModel::StandardCondModel:
	{
		break;
	}
	default:
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> Unknown crust property model detected; Crust property model upgraded to Standard conductivity model";
		myCrPropModel = BottomBoundaryManager::CrustPropertyModel::StandardCondModel;
		break;
	}
	return myCrPropModel;
}

BottomBoundaryManager::MantlePropertyModel Prograde::AlcModelConverter::upgradeAlcMantlePropModel(const BottomBoundaryManager::MantlePropertyModel MnPropModel) const {
	BottomBoundaryManager::MantlePropertyModel myMnPropModel = MnPropModel;
	switch (MnPropModel) {
	case BottomBoundaryManager::MantlePropertyModel::LegacyMantle:
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> Legacy mantle model detected; Mantle property model upgraded to High Conductivity Mantle model";
		myMnPropModel = BottomBoundaryManager::MantlePropertyModel::HighCondMnModel;
		break;
	}
	case BottomBoundaryManager::MantlePropertyModel::LowCondMnModel:
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> Low conductivity mantle model detected; Mantle property model upgraded to High Conductivity Mantle model";
		myMnPropModel = BottomBoundaryManager::MantlePropertyModel::HighCondMnModel;
		break;
	}
	case BottomBoundaryManager::MantlePropertyModel::StandardCondMnModel:
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> Standard conductivity mantle model detected; Mantle property model upgraded to High Conductivity Mantle model";
		myMnPropModel = BottomBoundaryManager::MantlePropertyModel::HighCondMnModel;
		break;
	}
	case BottomBoundaryManager::MantlePropertyModel::HighCondMnModel:
	{
		break;
	}
	default:
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> Unknown mantle property model detected; Mantle property model upgraded to High Conductivity Mantle model";
		myMnPropModel = BottomBoundaryManager::MantlePropertyModel::HighCondMnModel;
		break;
	}
	return myMnPropModel;

}

std::string Prograde::AlcModelConverter::updateBottomBoundaryModel(std::string & BottomBoundaryModel_original)
{
	std::string BottomBoundaryModel_updated = BottomBoundaryModel_original;
	if (!BottomBoundaryModel_updated.compare("Advanced Lithosphere Calculator")) {
		BottomBoundaryModel_updated = "Improved Lithosphere Calculator Linear Element Mode";
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> BottomBoundaryModel changed from 'Advanced Lithosphere Calculator' to 'Improved Lithosphere Calculator Linear Element Mode'";
	}
	return BottomBoundaryModel_updated;
}