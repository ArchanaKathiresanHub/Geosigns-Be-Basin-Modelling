// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_SGS_MODEL_CONVERTER_H
#define PROGRADE_SGS_MODEL_CONVERTER_H

//cmbAPI
#include "SourceRockManager.h"

namespace Prograde
{
	/// @class SgsConverter Converts BPA Shale gas related input parameters into BPA2 standard
	class SgsConverter {

	public:

		SgsConverter() = default;
		SgsConverter(const SgsConverter &) = delete;
		SgsConverter& operator=(const SgsConverter &) = delete;
		~SgsConverter() = default;

		/// @brief Upgrades shale gas related fields in SourceRockLithoIoTbl		
		int upgradeAdsorptionTOCDependent(int);
		std::string upgradeAdsorptionCapacityFunctionName(int, const std::string &);		
		/// @brief Upgrades irreducible water saturation related fields in IrreducibleWaterSaturationIoTbl
		void upgradeIrreducibleWaterSaturationCoefficients(const double, const double, double &, double & );
	};
}

#endif
