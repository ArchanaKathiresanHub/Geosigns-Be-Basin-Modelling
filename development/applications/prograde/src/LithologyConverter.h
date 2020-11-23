//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_LITHOLOGY_MODEL_CONVERTER_H
#define PROGRADE_LITHOLOGY_MODEL_CONVERTER_H

//cmbAPI
#include "StratigraphyManager.h"
#include "LithologyManager.h"

namespace Prograde
{
	/// @class LithologyConverter Converts LITHOLOGY related input parameters such as name as per the BPA2 standard
	class LithologyConverter {

	public:

		LithologyConverter() = default;
		LithologyConverter(const LithologyConverter &) = delete;
		LithologyConverter& operator=(const LithologyConverter &) = delete;
		~LithologyConverter() = default;

		static std::map<std::string, std::string> createMapForLithoNames();
		static std::map<std::string, std::string> createMapForUnparentedLithoNames();

		/// @brief Upgrades the deprecated standard LITHOLOGYs
		/// @details The standard lithologies of BPA are upgraded using to the new lithologies of BPA2 as per the mapping provided. No upgrade is needed for userDefined lithologies
		std::string upgradeLithologyName(const std::string &);

		/// @brief Upgraded the descriptions of the userDefined lithologies of BPA1 by appending "Based on legacy BPA <Old Parent lithology>" to the original description
		//@details The maximum length of the lithology description allowed in BPA2 is limited to 1000 characters. If the description exceeds this limit then we are resetting it to 1000 characters only
		std::string upgradeLithologyDescription(std::string &, const int &, const std::string &);

		/// @details Upgraded the DefinitionDate/LastChangedBy/LastChangedDate
		void upgradeLithologyAuditInfo(std::string &, std::string &, std::string &, const int &);

		/// @details Find the parent lithology name from the DefinedBy field of LithotypeIoTbl
		std::string findParentLithology(std::string definedBy);

		/// @brief Find the parent lithology name either from the description or from the LithologyName itself as per the mapping. 
		//@details This will be needed if the parent lithology name is not present in the original DefinedBy field of LithotypeIoTbl
		std::string findMissingParentLithology(const std::string lithologyName, std::string lithologyDescription);

		bool isDefinedBeforeThanCutOffDate(const std::string dateOfCreation);

		///@details The function computes the necessary model parameters (which are CompactionCoefficient, MinimumMecahnicalPorosity and the SurfacePorosity) used to define the single exponential model porosity model 
		std::vector<double> computeSingleExpModelParameters(const std::string, const int, mbapi::LithologyManager::PorosityModel &, std::vector<double> &);

		//@brief  Preprocessing the inputs to check whether the legacy inputs are as per the BPA2 import validation code requirement or not....If not then modify the inputs
		/*@details Lithology can't be imported if the lithology name is not available but lithology percents are available. If this is the case, then reset the lithology percentages to NO-DATA-VALUE
		If lithology is available but lithology percentages have both scalar and map specified then keep the map info and reset the scalar value to NO-DATA-VALUE*/

		ErrorHandler::ReturnCode PreprocessLithofaciesInputOfStratIoTbl(std::vector<std::string> & lithologyNamesSingleLayer, std::vector<double> & lithoPercntSingleLayer, std::vector<std::string> & lithoPercntGridSingleLayer);

		/// @brief Upgrades the permeability model parameters for standard LITHOLOGYs
		void upgradePermModelForSysDefLitho(const std::string &, std::vector<double> &, std::vector<double> &, int &);

		/// @brief Upgrades the permeability model parameters for user defined LITHOLOGYs
		void upgradePermModelForUsrDefLitho(const std::string &, std::vector<double> &, std::vector<double> &, int &);

		static std::map<std::string, std::string> lithologyNameMaps;

		static std::map<std::string, std::string> mappingOfLithologyNameBasedOndescription;

	};
}

#endif

