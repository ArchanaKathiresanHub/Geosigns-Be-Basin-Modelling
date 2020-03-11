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
		/// @details The standard lithologies of BPA are upgraded using to the new lithologies of BPA2 as per the mapping provided. No upgrade is needed for userDefined lithologies
		std::string upgradeLithologyName(const std::string &);

		/// @brief Upgraded the descriptions of the userDefined lithologies of BPA1 by appending "Based on legacy BPA <Old Parent lithology>" to the original description
		//@details The maximum length of the lithology description allowed in BPA2 is limited to 1000 characters. If the description exceeds this limit then we are resetting it to 1000 characters only
		std::string upgradeLithologyDescription(std::string &, const int &, const std::string &);

		/// @details Upgraded the DefinitionDate/LastChangedBy/LastChangedDate
		void upgradeLithologyAuditInfo(std::string &, std::string &, std::string &, const int &);

		/// @details Find the parent lithology name from the DefinedBy field of LithotypeIoTbl
		std::string findParentLithology(std::string definedBy, std::string, const int);

		///@details The function computes the necessary model parameters (which are CompactionCoefficient, MinimumMecahnicalPorosity and the SurfacePorosity) used to define the single exponential model porosity model 
		void computeSingleExpModelParameters(const std::string, const int, mbapi::LithologyManager::PorosityModel &, std::vector<double> &, std::vector<double> &);

      /// @brief Upgrades the permeability model parameters for standard LITHOLOGYs
      void upgradePermModelForSysDefLitho(const std::string &, std::vector<double> &, std::vector<double> &, int &);

      /// @brief Upgrades the permeability model parameters for user defined LITHOLOGYs
      void upgradePermModelForUsrDefLitho(const std::string &, std::vector<double> &, std::vector<double> &, int &);

      /// @brief Check and update Density property value in proposed range
      void upgradeLitPropDensity(double &);

      /// @brief Check and update Heat Production property value in proposed range
      void upgradeLitPropHeatProduction(double &);

      /// @brief Check and update Thermal Conductivity property value in proposed range
      void upgradeLitPropThrConductivity(double &);

      /// @brief Check and update Thermal Conductivity Anistropy property value in proposed range
      void upgradeLitPropThrCondAnistropy(double &);

      /// @brief Check and update Permeability Anistropy property value in proposed range
      void upgradeLitPropPermAnistropy(double &);

      /// @brief Check and update Seismic Velocity property value in proposed range
      void upgradeLitPropSeisVelocity(double &);

      /// @brief Check and update Seismic Velocity Exponent property value in proposed range
      void upgradeLitPropSeisVeloExponent(double &);

      /// @brief Check and update Entry Pressure Coefficient 1 property value in proposed range
      void upgradeLitPropEntryPresCoeff1(double &);

      /// @brief Check and update Entry Pressure Coefficient 2 property value in proposed range
      void upgradeLitPropEntryPresCoeff2(double &);

      /// @brief Check and update Hydraulic Fracturing property value in proposed range
      void upgradeLitPropHydFracturing(double &);

      /// @brief Check and update ReferenceSolidViscosity property value in proposed range
      void upgradeLitPropRefSoldViscosity(double &);

      /// @brief Check and update Intrusion Temperature property value in proposed range
      void upgradeLitPropIntrTemperature(double &);

	};
}

#endif

