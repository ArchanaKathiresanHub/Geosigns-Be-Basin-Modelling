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
	class SourceRockConverter {

	public:

		SourceRockConverter() = default;
		SourceRockConverter(const SourceRockConverter &) = delete;
		SourceRockConverter& operator=(const SourceRockConverter &) = delete;
		~SourceRockConverter() = default;

		/// @brief Upgrades shale gas related fields in SourceRockLithoIoTbl		
		double upgradeHcVre05(const std::string &, const std::string &, double, double);
		double upgradeScVre05(const std::string &, const std::string &, double);
		double upgradeEa(const std::string &, const std::string &, double, double);
		void upgradeSourceRockName(const std::string &, const std::string &, double, std::string &, std::string &);
		void upgradeDiffusionEnergy(const std::string &, double, double &, double &, double &, double &);
		void upgradeVESlimit(const std::string &, double &);
		void upgradeVREthreshold(const std::string &, double &);
	};
}

#endif

