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


		/// @brief Upgrades the deprecated standard LITHOLOGYs
		/// @details The fluid user defined flags are upgraded using upgradeUserDefined and fluid descriptions are upgraded using upgradeDescription
		std::string upgradeLithologyName(const std::string &);

		///@details The function computes the necessary model parameters (which are CompactionCoefficient, MinimumMecahnicalPorosity and the SurfacePorosity) used to define the single exponential model porosity model 
		//std::vector<double> computeSingleExpModelParameters(const std::string, const int, mbapi::LithologyManager::PorosityModel &, std::vector<double> &, std::vector<double> &);

	};
}

#endif

