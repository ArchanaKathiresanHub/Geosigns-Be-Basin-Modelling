//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_BRINE_MODEL_CONVERTER_H
#define PROGRADE_BRINE_MODEL_CONVERTER_H

//cmbAPI
#include "FluidManager.h"

namespace Prograde
{
	/// @class BrineModelConverter Converts brine density and brine siesmic velocity models
	class BrineModelConverter {

	public:

		BrineModelConverter() = default;
		BrineModelConverter(const BrineModelConverter &) = delete;
		BrineModelConverter& operator=(const BrineModelConverter &) = delete;
		~BrineModelConverter() = default;

		/// @brief Upgrades the corresponding brine models
		/// @details The density models are upgraded using upgradeDensityModel and seismic velocity models are upgraded using upgradeSeismicVelocityModel
		mbapi::FluidManager::FluidDensityModel upgradeDensityModel(const mbapi::FluidManager::FluidDensityModel, const std::string &) const;

		mbapi::FluidManager::CalculationModel upgradeSeismicVelocityModel(const mbapi::FluidManager::CalculationModel, const std::string &) const;

	};
}

#endif