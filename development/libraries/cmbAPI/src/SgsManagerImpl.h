//                                                                      
// Copyright (C) 2019 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by CGI.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file SgsManagerImpl.h
/// @brief This file keeps API implementation for manipulating shale gas related table in Cauldron model

#ifndef CMB_SGS_MANAGER_IMPL_API
#define CMB_SGS_MANAGER_IMPL_API

#include "ProjectFileHandler.h"

#include "SgsManager.h"

namespace mbapi {

	// Class SgsManager keeps a list of shale gas related table in Cauldron model and allows to add/delete/edit shale gas related table
	class SgsManagerImpl : public SgsManager
	{
	public:
		// Constructors/destructor
		// brief Constructor which creates an SgsManager
		SgsManagerImpl();

		// Destructor
		virtual ~SgsManagerImpl() { ; }

		// Copy operator
		SgsManagerImpl & operator = (const SgsManagerImpl & otherSgsMgr);

		
		// Set project database. Reset all
		void setDatabase(database::ProjectFileHandlerPtr pfh);

		// Get list of rows in LangmuirAdsorptionCapacityIsothermSetIoTbl table		
		virtual std::vector<sgsID> sgsIDs() const;

		/// @brief Get langmuir name for given ID
		/// @param[in] id row ID in LangmuirAdsorptionCapacityIsothermSetIoTbl table
		/// @param[out] LangmuirName langmuir name in LangmuirAdsorptionCapacityIsothermSetIoTbl table
		virtual std::string getLangmuirName(sgsID id);

		/// @brief Set langmuir name for given ID
		/// @param[in] id row ID in LangmuirAdsorptionCapacityIsothermSetIoTbl table
		/// @param[in] newLangmuirName langmuir name in LangmuirAdsorptionCapacityIsothermSetIoTbl table
		virtual ReturnCode  setLangmuirName(sgsID id, std::string newLangmuirName);

		/// @brief Set langmuir temperature for given ID
		/// @param[in] id row ID in LangmuirAdsorptionCapacityIsothermSetIoTbl table
		/// @param[in] newLangmuirTemperature langmuir temperature in LangmuirAdsorptionCapacityIsothermSetIoTbl table
		virtual ReturnCode  setLangmuirTemperature(sgsID id, double newLangmuirTemperature);

		/// @brief Set langmuir pressure for given ID
		/// @param[in] id row ID in LangmuirAdsorptionCapacityIsothermSetIoTbl table
		/// @param[in] newLangmuirPressure langmuir pressure in LangmuirAdsorptionCapacityIsothermSetIoTbl table
		virtual ReturnCode  setLangmuirPressure(sgsID id, double newLangmuirPressure);

		/// @brief Set langmuir volume for given ID
		/// @param[in] id row ID in LangmuirAdsorptionCapacityIsothermSetIoTbl table
		/// @param[in] newLangmuirVolume langmuir volume in LangmuirAdsorptionCapacityIsothermSetIoTbl table
		virtual ReturnCode  setLangmuirVolume(sgsID id, double newLangmuirVolume);

		/// @brief Get coefficient A for given ID
		/// @param[in] id row ID in IrreducibleWaterSaturationIoTbl table
		/// @param[out] CoefficientA coefficient A in IrreducibleWaterSaturationIoTbl table
		virtual double getCoefficientA(sgsID id);

		/// @brief Get coefficient B for given ID
		/// @param[in] id row ID in IrreducibleWaterSaturationIoTbl table
		/// @param[out] CoefficientB coefficient B in IrreducibleWaterSaturationIoTbl table
		virtual double getCoefficientB(sgsID id);

		/// @brief Set coefficient A for given ID
		/// @param[in] id row ID in IrreducibleWaterSaturationIoTbl table
		/// @param[in] newCoefficientA coefficient A in IrreducibleWaterSaturationIoTbl table
		virtual ReturnCode  setCoefficientA(sgsID id, double newCoefficientA);

		/// @brief Set coefficient B for given ID
		/// @param[in] id row ID in IrreducibleWaterSaturationIoTbl table
		/// @param[in] newCoefficientB coefficient B in IrreducibleWaterSaturationIoTbl table
		virtual ReturnCode  setCoefficientB(sgsID id, double newCoefficientB);

	private:
		static const char * s_LangmuirAdsorptionCapacityIsothermSetTableName;   // table name for langmuir adsorption Capacity isotherm table in project file
		static const char * s_LangmuirNameFieldName;							// name of the field which keeps langmuir name
		static const char * s_LangmuirTemperatureFieldName;						// name of the field which keeps langmuir temperature name
		static const char * s_LangmuirPressureFieldName;						// name of the field which keeps langmuir pressure name
		static const char * s_LangmuirVolumeFieldName;							// name of the field which keeps langmuir volume name
		static const char * s_IrreducibleWaterSaturationTableName;				// table name for irreducible water saturation table in project file
		static const char * s_CoefficientAFieldName;							// name of the field which keeps coefficient A name
		static const char * s_CoefficientBFieldName;							// name of the field which keeps coefficient B name
		// Copy constructor is disabled, use the copy operator instead
		SgsManagerImpl(const SgsManager &);

		database::ProjectFileHandlerPtr m_db; // cauldron project database
	};
}

#endif // CMB_SGS_MANAGER_IMPL_API

