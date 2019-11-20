//                                                                      
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by CGI.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file FaultCutManager.h
/// @brief This file keeps API declaration which deal with project level inputs such as modelling mode, project AOI, name, description etc.

#ifndef CMB_FAULT_CUT_MANAGER_API
#define CMB_FAULT_CUT_MANAGER_API

#include "ErrorHandler.h"

/// @page FaultCutManagerPage fault cut Manager
/// @link mbapi::FaultCutManager FaultCu manager @endlink provides set of interfaces to interfaces to load/modify/save fault cut inputs in project file 

namespace mbapi {
	/// @class FaultCutManager FaultCutManager.h "FaultCutManager.h" 
	/// @brief Class FaultCutManager keeps fault caut inputs in Cauldron model and allows to add/delete/edit those inputs 
	class FaultCutManager : public ErrorHandler
	{
	public:

		typedef size_t faultCutLithologyID;         ///< unique ID for fault cut lithology

													
		virtual std::vector<faultCutLithologyID> faultCutLithIDs() const = 0;

		/// @brief Get fault lithology name for
		/// @param[in] id fault cut record ID
		/// @param[out] faultLithoName lithotype name of fault specified in the table 
		/// @return NoError on success or NonexistingID on error
		virtual ReturnCode getFaultLithoName(const faultCutLithologyID id, std::string & faultLithoName) = 0;

		/// @brief Setfault lithology name for
		/// @param[in] id fault cut record ID
		/// @param[in] faultLithoName lithotype name of fault to be set in the table
		/// @return NoError on success or NonexistingID on error
		virtual ReturnCode setFaultLithoName(const faultCutLithologyID id, const std::string faultLithoName) = 0;

		/// @}
	protected:
		/// @{
		/// Constructors/destructor

		/// @brief Constructor which creates an empty model
		FaultCutManager() = default;

		/// @brief Destructor, no any actual work is needed here, all is done in the implementation part
		virtual ~FaultCutManager() = default;

		/// @}

	private:
		/// @{
		/// Copy constructor and copy operator are disabled
		FaultCutManager(const FaultCutManager& otherFaultCutManager);
		FaultCutManager& operator = (const FaultCutManager& otherFaultCutManager);
		/// @}
	};
}

#endif // CMB_FRACTURE_PRESSURE_MANAGER_API





