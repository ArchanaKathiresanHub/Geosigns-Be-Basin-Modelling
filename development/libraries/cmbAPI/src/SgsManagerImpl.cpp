//                                                                      
// Copyright (C) 2019 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by CGI.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file SgsManagerImpl.C
/// @brief This file keeps API implementation for manipulating shale gas related table in Cauldron model

// CMB API
#include "cmbAPI.h"
#include "SgsManagerImpl.h"

// TableIO lib
#include "database.h"

// STL
#include <stdexcept>
#include <string>

namespace mbapi
{
	const char * SgsManagerImpl::s_LangmuirAdsorptionCapacityIsothermSetTableName = "LangmuirAdsorptionCapacityIsothermSetIoTbl";
	const char * SgsManagerImpl::s_LangmuirNameFieldName = "LangmuirName";
	const char * SgsManagerImpl::s_LangmuirTemperatureFieldName = "LangmuirTemperature";
	const char * SgsManagerImpl::s_LangmuirPressureFieldName = "LangmuirPressure";
	const char * SgsManagerImpl::s_LangmuirVolumeFieldName = "LangmuirVolume";
	const char * SgsManagerImpl::s_IrreducibleWaterSaturationTableName = "IrreducibleWaterSaturationIoTbl";
	const char * SgsManagerImpl::s_CoefficientAFieldName = "CoefficientA";
	const char * SgsManagerImpl::s_CoefficientBFieldName = "CoefficientB";

	// Constructor
	SgsManagerImpl::SgsManagerImpl()
	{
		//throw std::runtime_error( "Not implemented yet" );
		m_db = NULL;
	}

	// Copy operator
	SgsManagerImpl & SgsManagerImpl::operator = (const SgsManagerImpl & /*otherSgsMgr*/)
	{
		throw ErrorHandler::Exception(ErrorHandler::NotImplementedAPI) << "SgsManagerImpl::operator = () not implemented yet";
		return *this;
	}

	// Set project database. Reset all
	void SgsManagerImpl::setDatabase(database::ProjectFileHandlerPtr pfh)
	{
		m_db = pfh;
	}

	// Get list of rows in LangmuirAdsorptionCapacityIsothermSetIoTbl table
	// return array with IDs of different rows in LangmuirAdsorptionCapacityIsothermSetIoTbl table defined in the model
	std::vector<SgsManager::sgsID> SgsManagerImpl::sgsIDs() const
	{
		std::vector<sgsID> sgsIDs;
		if (!m_db) return sgsIDs;

		// get pointer to the table
		database::Table * table = m_db->getTable(s_LangmuirAdsorptionCapacityIsothermSetTableName);

		// if table does not exist - return empty array
		if (!table) return sgsIDs;

		// fill IDs array with increasing indexes
		sgsIDs.resize(table->size(), 0);

		for (size_t i = 0; i < sgsIDs.size(); ++i) sgsIDs[i] = static_cast<sgsID>(i);

		return sgsIDs;
	}

	// Get langmuir name for given ID
	std::string SgsManagerImpl::getLangmuirName(sgsID id)
	{
		if (errorCode() != NoError) resetError();
		std::string langName;
		try
		{
			// get pointer to the table
			database::Table * table = m_db->getTable(s_LangmuirAdsorptionCapacityIsothermSetTableName);

			// if table does not exist - report error
			if (!table)
			{
				throw Exception(NonexistingID) << s_LangmuirAdsorptionCapacityIsothermSetTableName << " table could not be found in project";
			}

			database::Record * rec = table->getRecord(static_cast<int>(id));
			if (!rec)
			{
				throw Exception(NonexistingID) << "No Langmuir name with such ID: " << id;
			}
			langName = rec->getValue<std::string>(s_LangmuirNameFieldName);
		}
		catch (const Exception & e) { reportError(e.errorCode(), e.what()); }

		return langName;
	}

	// Set langmuir name for given ID
	ErrorHandler::ReturnCode SgsManagerImpl::setLangmuirName(sgsID id, std::string newLangmuirName)
	{
		if (errorCode() != NoError) resetError();
		std::string langName;

		try
		{
			// get pointer to the table
			database::Table * table = m_db->getTable(s_LangmuirAdsorptionCapacityIsothermSetTableName);

			// if table does not exist - report error
			if (!table)
			{
				throw Exception(NonexistingID) << s_LangmuirAdsorptionCapacityIsothermSetTableName << " table could not be found in project";
			}

			database::Record * rec = table->getRecord(static_cast<int>(id));
			if (!rec)
			{
				throw Exception(NonexistingID) << "No Langmuir name with such ID: " << id;
			}
			rec->setValue(s_LangmuirNameFieldName, newLangmuirName);
		}
		catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

		return NoError;
	}

	// Set langmuir temperature for given ID
	ErrorHandler::ReturnCode SgsManagerImpl::setLangmuirTemperature(sgsID id, double newLangmuirTemperature)
	{
		if (errorCode() != NoError) resetError();
		std::string langName;

		try
		{
			// get pointer to the table
			database::Table * table = m_db->getTable(s_LangmuirAdsorptionCapacityIsothermSetTableName);

			// if table does not exist - report error
			if (!table)
			{
				throw Exception(NonexistingID) << s_LangmuirAdsorptionCapacityIsothermSetTableName << " table could not be found in project";
			}

			database::Record * rec = table->getRecord(static_cast<int>(id));
			if (!rec)
			{
				throw Exception(NonexistingID) << "No Langmuir name with such ID: " << id;
			}
			rec->setValue(s_LangmuirTemperatureFieldName, newLangmuirTemperature);
		}
		catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

		return NoError;
	}

	// Set langmuir pressure for given ID
	ErrorHandler::ReturnCode SgsManagerImpl::setLangmuirPressure(sgsID id, double newLangmuirPressure)
	{
		if (errorCode() != NoError) resetError();
		std::string langName;

		try
		{
			// get pointer to the table
			database::Table * table = m_db->getTable(s_LangmuirAdsorptionCapacityIsothermSetTableName);

			// if table does not exist - report error
			if (!table)
			{
				throw Exception(NonexistingID) << s_LangmuirAdsorptionCapacityIsothermSetTableName << " table could not be found in project";
			}

			database::Record * rec = table->getRecord(static_cast<int>(id));
			if (!rec)
			{
				throw Exception(NonexistingID) << "No Langmuir name with such ID: " << id;
			}
			rec->setValue(s_LangmuirPressureFieldName, newLangmuirPressure);
		}
		catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

		return NoError;
	}

	// Set langmuir volume for given ID
	ErrorHandler::ReturnCode SgsManagerImpl::setLangmuirVolume(sgsID id, double newLangmuirVolume)
	{
		if (errorCode() != NoError) resetError();
		std::string langName;

		try
		{
			// get pointer to the table
			database::Table * table = m_db->getTable(s_LangmuirAdsorptionCapacityIsothermSetTableName);

			// if table does not exist - report error
			if (!table)
			{
				throw Exception(NonexistingID) << s_LangmuirAdsorptionCapacityIsothermSetTableName << " table could not be found in project";
			}

			database::Record * rec = table->getRecord(static_cast<int>(id));
			if (!rec)
			{
				throw Exception(NonexistingID) << "No Langmuir name with such ID: " << id;
			}
			rec->setValue(s_LangmuirVolumeFieldName, newLangmuirVolume);
		}
		catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

		return NoError;
	}

	// Get coefficient A for given ID
	double SgsManagerImpl::getCoefficientA(sgsID id)
	{
		if (errorCode() != NoError) resetError();
		double coeffA;
		try
		{
			// get pointer to the table
			database::Table * table = m_db->getTable(s_IrreducibleWaterSaturationTableName);

			// if table does not exist - report error
			if (!table)
			{
				throw Exception(NonexistingID) << s_IrreducibleWaterSaturationTableName << " table could not be found in project";
			}

			database::Record * rec = table->getRecord(static_cast<int>(id));
			if (!rec)
			{
				throw Exception(NonexistingID) << "No Coefficient A with such ID: " << id;
			}
			coeffA = rec->getValue<double>(s_CoefficientAFieldName);
		}
		catch (const Exception & e) { reportError(e.errorCode(), e.what()); }

		return coeffA;
	}

	// Get coefficient B for given ID
	double SgsManagerImpl::getCoefficientB(sgsID id)
	{
		if (errorCode() != NoError) resetError();
		double coeffB;
		try
		{
			// get pointer to the table
			database::Table * table = m_db->getTable(s_IrreducibleWaterSaturationTableName);

			// if table does not exist - report error
			if (!table)
			{
				throw Exception(NonexistingID) << s_IrreducibleWaterSaturationTableName << " table could not be found in project";
			}

			database::Record * rec = table->getRecord(static_cast<int>(id));
			if (!rec)
			{
				throw Exception(NonexistingID) << "No Coefficient B with such ID: " << id;
			}
			coeffB = rec->getValue<double>(s_CoefficientBFieldName);
		}
		catch (const Exception & e) { reportError(e.errorCode(), e.what()); }

		return coeffB;
	}

	// Set coefficient A for given ID
	ErrorHandler::ReturnCode SgsManagerImpl::setCoefficientA(sgsID id, double newCoefficientA)
	{
		if (errorCode() != NoError) resetError();
		
		try
		{
			// get pointer to the table
			database::Table * table = m_db->getTable(s_IrreducibleWaterSaturationTableName);

			// if table does not exist - report error
			if (!table)
			{
				throw Exception(NonexistingID) << s_IrreducibleWaterSaturationTableName << " table could not be found in project";
			}

			database::Record * rec = table->getRecord(static_cast<int>(id));
			if (!rec)
			{
				throw Exception(NonexistingID) << "No Coefficient A with such ID: " << id;
			}
			rec->setValue(s_CoefficientAFieldName, newCoefficientA);
		}
		catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

		return NoError;
	}

	// Set coefficient B for given ID
	ErrorHandler::ReturnCode SgsManagerImpl::setCoefficientB(sgsID id, double newCoefficientB)
	{
		if (errorCode() != NoError) resetError();

		try
		{
			// get pointer to the table
			database::Table * table = m_db->getTable(s_IrreducibleWaterSaturationTableName);

			// if table does not exist - report error
			if (!table)
			{
				throw Exception(NonexistingID) << s_IrreducibleWaterSaturationTableName << " table could not be found in project";
			}

			database::Record * rec = table->getRecord(static_cast<int>(id));
			if (!rec)
			{
				throw Exception(NonexistingID) << "No Coefficient B with such ID: " << id;
			}
			rec->setValue(s_CoefficientBFieldName, newCoefficientB);
		}
		catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

		return NoError;
	}
}