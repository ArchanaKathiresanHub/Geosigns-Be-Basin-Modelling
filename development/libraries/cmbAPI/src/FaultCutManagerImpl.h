
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
/// @brief This file keeps API declaration which deal with fault cut related inputs

#ifndef CMB_FAULT_CUT_MANAGER_IMPL_API
#define CMB_FAULT_CUT_MANAGER_IMPL_API

#include <memory>

#include "ProjectFileHandler.h"

#include "FaultCutManager.h"

namespace mbapi {

	// Class FaultCutManager keeps fault cute inputs in Cauldron model and allows to add / delete / edit those inputs

	class FaultCutManagerImpl : public FaultCutManager
	{
	public:
		// Constructors/destructor
		// brief Constructor which creates an FaultCutManager
		FaultCutManagerImpl();

		// Destructor
		virtual ~FaultCutManagerImpl() { ; }

		// Copy operator
		FaultCutManagerImpl& operator = (const FaultCutManagerImpl& projectMgr);

		// Set project database. Reset all
		void setDatabase(database::ProjectFileHandlerPtr pfh);

		/// @brief Get list of record ids defined in PressureFaultcutIoTbl
		/// @return array with IDs of each record 
		virtual std::vector<faultCutLithologyID> faultCutLithIDs() const;

		/// @brief Get fault lithology name for
		/// @param[in] id fault cut record ID
		/// @param[out] faultLithoName lithotype name of fault specified in the table 
		/// @return NoError on success or NonexistingID on error
		virtual ReturnCode getFaultLithoName(const faultCutLithologyID id, std::string & faultLithoName);

		/// @brief Setfault lithology name for
		/// @param[in] id fault cut record ID
		/// @param[in] faultLithoName lithotype name of fault to be set in the table
		/// @return NoError on success or NonexistingID on error
		virtual ReturnCode setFaultLithoName(const faultCutLithologyID id, const std::string faultLithoName);

		

	private:
		// Copy constructor is disabled, use the copy operator instead
		FaultCutManagerImpl(const FaultCutManager&);

		static const char* s_pressureFaultcutTableName;

		database::ProjectFileHandlerPtr m_db;  // cauldron project database
		database::Table* m_pressureFaultcutIoTbl;       // ProjectIo table

											   //static const char* s_xcoordFieldName;  // column name for x-coordinate of origin in ProjectIoTbl
											   //static const char* s_ycoordFieldName;  // column name for y-coordinate of origin in ProjectIoTbl
		static const char* s_faultLithologyFieldName;  // column name for number of nodes in x direction
		

	};
}

#endif // CMB_PROJECT_DATA_MANAGER_IMPL_API


