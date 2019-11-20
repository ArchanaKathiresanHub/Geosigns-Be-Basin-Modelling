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

#include "FaultCutManagerImpl.h"
#include <stdexcept>
#include <string>


namespace mbapi
{
	const char* FaultCutManagerImpl::s_pressureFaultcutTableName = "PressureFaultcutIoTbl";
	
	const char* FaultCutManagerImpl::s_faultLithologyFieldName = "FaultLithology";
	

	// Constructor
	FaultCutManagerImpl::FaultCutManagerImpl()
	{
		m_db = nullptr;
		m_pressureFaultcutIoTbl = nullptr;
	}

	// Copy operator
	FaultCutManagerImpl& FaultCutManagerImpl::operator = (const FaultCutManagerImpl&)
	{
		throw std::runtime_error("Not implemented yet");
		return *this;
	}

	// Set project database. Reset all
	void FaultCutManagerImpl::setDatabase(database::ProjectFileHandlerPtr pfh)
	{
		m_db = pfh;
		m_pressureFaultcutIoTbl = m_db->getTable(s_pressureFaultcutTableName);
	}

	
	std::vector<FaultCutManager::faultCutLithologyID> FaultCutManagerImpl::faultCutLithIDs() const
	{
		std::vector<faultCutLithologyID> fltIDs; 
		if (m_pressureFaultcutIoTbl)
		{
			// fill IDs array with increasing indexes
			fltIDs.resize(m_pressureFaultcutIoTbl->size(), 0);
			for (size_t i = 0; i < fltIDs.size(); ++i) fltIDs[i] = static_cast<faultCutLithologyID>(i);
		}

		return fltIDs;
	}

	// Get fault lothology name
	ErrorHandler::ReturnCode FaultCutManagerImpl::getFaultLithoName(const faultCutLithologyID id, std::string & lithName)
	{
		if (errorCode() != NoError) resetError();

		try
		{
			if (!m_pressureFaultcutIoTbl) { throw Exception(NonexistingID) << s_pressureFaultcutTableName << " table could not be found in project"; }
			database::Record * rec = m_pressureFaultcutIoTbl->getRecord(static_cast<int>(id));
			if (!rec) { throw Exception(NonexistingID) << "No fluid type with such ID: " << id; }
			lithName = rec->getValue<std::string>(s_faultLithologyFieldName);
		}
		catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
		return NoError;
	}

	// set fault lothology name
	ErrorHandler::ReturnCode FaultCutManagerImpl::setFaultLithoName(const faultCutLithologyID id, const std::string faultLithoName)
	{
		if (errorCode() != NoError) resetError();

		try
		{
			if (!m_pressureFaultcutIoTbl) { throw Exception(NonexistingID) << s_pressureFaultcutTableName << " table could not be found in project"; }
			database::Record * rec = m_pressureFaultcutIoTbl->getRecord(static_cast<int>(id));
			if (!rec) { throw Exception(NonexistingID) << "No fluid type with such ID: " << id; }
			rec->setValue<std::string>(s_faultLithologyFieldName, faultLithoName);
		}
		catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
		return NoError;
	}

	
}

