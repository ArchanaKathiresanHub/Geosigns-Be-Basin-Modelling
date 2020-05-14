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
#include "SourceRockTypeNameMappings.h"

#include <map>
#include <utility>
#include <string>
#include "Utilities.h"


namespace Prograde
{
	// @brief a definition of maps that relates bpa source rock names to BPa2 source rock names & base source rock names
	typedef std::map<std::string, std::pair<std::string, std::string>> pairMap;
	// @brief a definition of maps that relates bpa source rock default properties to BPA2 source rock default properties
	typedef std::map<std::string, std::vector<double>> DefaultValueMap;
	/// @class SourceRockConverter Converts BPA Source rocks related input parameters into BPA2 standard
	class SourceRockConverter {

	public:

		SourceRockConverter()= default;
		SourceRockConverter(const SourceRockConverter &) = delete;
		SourceRockConverter& operator=(const SourceRockConverter &) = delete;
		~SourceRockConverter() = default;

		/// @brief Upgrades related fields in SourceRockLithoIoTbl		
		double upgradeHcVre05(const std::string &, const std::string &, double, double, double &, double &) const;
		double upgradeHiVre05(const std::string&, const std::string&, double, double, double&, double&) const;
		double upgradeScVre05(const std::string &, const std::string &, double) const;
		double upgradeEa(const std::string &, const std::string &, double, double) const;
		void upgradeSourceRockName(const std::string& legacySourceRockType, const std::string& legacyBaseSourceRockType, double legacyScVRe05, std::string& bpa2SourceRockType, std::string& bpa2BaseSourceRockType, bool& litFlag);
		void upgradeDiffusionEnergy(const std::string &, double, double &, double &, double &, double &);
		void upgradeVESlimit(const std::string &, double &);
		void upgradeVREthreshold(const std::string &, double &);

		static const pairMap SrNameMaps;
		static const DefaultValueMap SrDefValueMap;
		/// @brief
		std::string GetBPA2BaseRockName(const std::string& bpaBaseSourceRockName,const std::string* bpaSourceRockName=nullptr) const;
		std::string GetBPA2RockName(const std::string& bpaBaseSourceRockName,const std::string* bpaSourceRockName=nullptr) const;
		bool isBpaSrSulfurous(const std::string& bpaSourceRockName) const;
		bool isSrFromLiterature(const std::string& bpaBaseSourceRockName, const std::string* bpaSourceRockName = nullptr) const;

		bool isSrFromLiterature2ndImplementation(const std::string* bpaSourceRockName) const;

		int getIndexOfSrMap(const std::string& bpaBaseSourceRockName, const std::string* bpaSourceRockName = nullptr);
		double GetmeasuredHI(const std::string& bpaBaseSourceRockName, const std::string* bpaSourceRockName = nullptr) const;


	private:
		std::string AddTailing_sToSulfurousBpaBaseSrName(const std::string& bpaBaseSourceRockName) const;
		
		double GetmeasuredEa(const std::string& bpaBaseSourceRockName, const std::string* bpaSourceRockName = nullptr) const;
		double GetmeasuredHcVre05(const std::string& bpaBaseSourceRockName, const std::string* bpaSourceRockName = nullptr) const;
		double GetmeasuredScVre05s(const std::string& bpaBaseSourceRockName) const;
		
		double GetbpaBaseSourceRockAsphalteneDE(const std::string& bpaBaseSourceRockName);
		double GetbpaBaseSourceRockResinDE(const std::string& bpaBaseSourceRockName);
		double GetbpaBaseSourceRockC15AroDE(const std::string& bpaBaseSourceRockName);
		double GetbpaBaseSourceRockC15SatDE(const std::string& bpaBaseSourceRockName);
	};
}

#endif

