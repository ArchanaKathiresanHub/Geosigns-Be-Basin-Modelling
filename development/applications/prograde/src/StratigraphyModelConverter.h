//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_STRATIGRAPHY_MODEL_CONVERTER_H
#define PROGRADE_STRATIGRAPHY_MODEL_CONVERTER_H

//cmbAPI
#include "StratigraphyManager.h"

namespace Prograde
{
	/// @class StratigraphyModelConverter
	class StratigraphyModelConverter {

	public:

		StratigraphyModelConverter() = default;
		StratigraphyModelConverter(const StratigraphyModelConverter &) = delete;
		StratigraphyModelConverter& operator=(const StratigraphyModelConverter &) = delete;
		~StratigraphyModelConverter() = default;


		/// @brief Upgrades the names of stratigraphy components according to BPA2 standards
		/// @details In BPA2, only alphanumeric characters are allowed for naming the stratigraphy surfaces, layers and fluidnames so, the special characters are removed
		std::string upgradeName(const std::string &);
		
		///@brief Upgrades the (min,max) limits of depth and thickness values in StratIoTbl according to BPA2 standards
		///@details In BPA2, the depth and thickness values are set to (-6380000, +6380000) contrary to (-infinity, +infinity) in BPA legacy
		double upgradeDepthThickness(const double &);

		///@brief Upgrades the LayeringIndex in StratIoTbl depending upon the selected MixModel as per BPA2 standards
		///@details In BPA2, the LayeringIndex is set to -9999 for Homogeneous MixModel contrary to the value 1 in BPA legacy
		double upgradeLayeringIndex(const std::string &, const double &);

		///@brief Checks the ChemicalCompaction value set to 0 for Hydrostatic PTCouplingMode in RunOptionsIoTbl
		///@details ChemicalCompaction can not be activated if PTCouplingMode is set as Homogeneous
		int	checkChemicalCompaction(const std::string &, const int &);

		///@brief Upgrades the ChemicalCompaction in StratIoTbl according to BPA2 standards
		///@details In BPA2, ChemicalCompaction can not be set individually in each layer, it can be set globally for each layer in RunOptionsIoTbl, so values of ChemicalCompaction in each layer is set as same as in RunOptionsIoTbl
		int	upgradeChemicalCompaction(const int &, const int &);

	};

}

#endif

