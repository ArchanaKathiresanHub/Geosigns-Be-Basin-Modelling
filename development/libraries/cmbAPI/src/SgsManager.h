//                                                                      
// Copyright (C) 2019 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by CGI.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file SgsManager.h
/// @brief This file keeps API declaration for manipulating shale gas related table in Cauldron model

#ifndef CMB_SGS_MANAGER_API
#define CMB_SGS_MANAGER_API

#include <vector>
#include "ErrorHandler.h"

/// @page SgsManagerPage Shale Gas Manager
/// @link mbapi::SgsManager Shale Gas Manager @endlink provides set of interfaces to create/delete/edit
/// list various shale gas related tables in the data model. Also it has set of interfaces to get/set property of any shale gas
/// related tables from the list

namespace mbapi {
	/// @class SgsManager SgsManager.h "SgsManager.h"
	/// @brief Class SgsManager keeps a list of shale gas related table in Cauldron model and allows to add/delete/edit shale gas 
	/// related tables
	class SgsManager : public ErrorHandler
	{
	public:

		/// @{
		/// Set of interfaces for interacting with a shale gas related table

		typedef size_t sgsID;  ///< unique ID for shale gas related table
		
		/// @brief Get list of row ID in LangmuirAdsorptionCapacityIsothermSetIoTbl table
		/// @return array with IDs of different langmuir isotherm defined in the model
		virtual std::vector<sgsID> sgsIDs() const = 0;

		/// @brief Get langmuir name for given ID
		/// @param[in] id row ID in LangmuirAdsorptionCapacityIsothermSetIoTbl table
		/// @param[out] LangmuirName langmuir name in LangmuirAdsorptionCapacityIsothermSetIoTbl table
		virtual std::string getLangmuirName(sgsID id) = 0;

		/// @brief Set langmuir name for given ID
		/// @param[in] id row ID in LangmuirAdsorptionCapacityIsothermSetIoTbl table
		/// @param[in] newLangmuirName langmuir name in LangmuirAdsorptionCapacityIsothermSetIoTbl table		
		virtual ReturnCode setLangmuirName(sgsID id, std::string newLangmuirName) = 0;

		/// @brief Set langmuir temperature for given ID
		/// @param[in] id row ID in LangmuirAdsorptionCapacityIsothermSetIoTbl table
		/// @param[in] newLangmuirTemperature langmuir temperature in LangmuirAdsorptionCapacityIsothermSetIoTbl table
		virtual ReturnCode setLangmuirTemperature(sgsID id, double newLangmuirTemperature) = 0;

		/// @brief Set langmuir pressure for given ID
		/// @param[in] id row ID in LangmuirAdsorptionCapacityIsothermSetIoTbl table
		/// @param[in] newLangmuirPressure langmuir pressure in LangmuirAdsorptionCapacityIsothermSetIoTbl table
		virtual ReturnCode setLangmuirPressure(sgsID id, double newLangmuirPressure) = 0;

		/// @brief Set langmuir volume for given ID
		/// @param[in] id row ID in LangmuirAdsorptionCapacityIsothermSetIoTbl table
		/// @param[in] newLangmuirVolume langmuir volume in LangmuirAdsorptionCapacityIsothermSetIoTbl table
		virtual ReturnCode setLangmuirVolume(sgsID id, double newLangmuirVolume) = 0;

		/// @brief Get coefficient A for given ID
		/// @param[in] id row ID in IrreducibleWaterSaturationIoTbl table
		/// @param[out] CoefficientA coefficient A in IrreducibleWaterSaturationIoTbl table
		virtual double getCoefficientA(sgsID id)=0;

		/// @brief Get coefficient B for given ID
		/// @param[in] id row ID in IrreducibleWaterSaturationIoTbl table
		/// @param[out] CoefficientB coefficient B in IrreducibleWaterSaturationIoTbl table
		virtual double getCoefficientB(sgsID id)=0;

		/// @brief Set coefficient A for given ID
		/// @param[in] id row ID in IrreducibleWaterSaturationIoTbl table
		/// @param[in] newCoefficientA coefficient A in IrreducibleWaterSaturationIoTbl table
		virtual ReturnCode  setCoefficientA(sgsID id, double newCoefficientA)=0;

		/// @brief Set coefficient B for given ID
		/// @param[in] id row ID in IrreducibleWaterSaturationIoTbl table
		/// @param[in] newCoefficientB coefficient B in IrreducibleWaterSaturationIoTbl table
		virtual ReturnCode  setCoefficientB(sgsID id, double newCoefficientB)=0;

		/// @}		

	protected:
		/// @name Constructors/destructor
		/// @{
		/// @brief Constructor which creates an empty model
		SgsManager() { ; }

		/// @brief Destructor, no any actual work is needed here, all is done in the implementation part
		virtual ~SgsManager() { ; }

	private:
		/// @{
		/// Copy constructor and operator are disabled, use the copyTo method instead
		SgsManager(const SgsManager & otherSourceRockManager);
		SgsManager & operator = (const SgsManager & otherSourceRockManager);
		/// @}
	};
}

#endif // CMB_SGS_MANAGER_API
