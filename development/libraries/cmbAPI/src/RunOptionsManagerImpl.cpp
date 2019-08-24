//                                                                      
// Copyright (C) 2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by CGI.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file RunOptionsManagerImpl.C
/// @brief This file keeps API implementation for manipulating runOptionsIoTbl in Cauldron model

#include "RunOptionsManagerImpl.h"

#include <stdexcept>
#include <string>

namespace mbapi
{
	const char * RunOptionsManagerImpl::s_runOptionsTableName = "RunOptionsIoTbl";
	const char * RunOptionsManagerImpl::s_vreAlgorithmFieldName = "VreAlgorithm";
	const char * RunOptionsManagerImpl::s_velAlgorithmFieldName = "VelAlgorithm";
	const char * RunOptionsManagerImpl::s_optimisationLevelFieldName = "Optimisation_Level";
	const char * RunOptionsManagerImpl::s_TemperatureGradientFieldName = "Temperature_Gradient";
	const char * RunOptionsManagerImpl::s_OTPresDiffFieldName = "OptimalTotalPresDiff";
	const char * RunOptionsManagerImpl::s_OTTempDiffFieldName = "OptimalTotalTempDiff";
	const char * RunOptionsManagerImpl::s_OSRTempDiffFieldName = "OptimalSourceRockTempDiff";
	const char * RunOptionsManagerImpl::s_PTCouplingModeFieldName = "PTCouplingMode";
	const char * RunOptionsManagerImpl::s_coupledModeFieldName = "CoupledMode";
	const char * RunOptionsManagerImpl::s_chemCompactionAlgorithmFieldName = "ChemicalCompactionAlgorithm";
	const char* RunOptionsManagerImpl::s_workflowTypeFieldName = "WorkflowType";


	// Constructor
	RunOptionsManagerImpl::RunOptionsManagerImpl()
	{
		m_db = NULL;
	}

	// Copy operator
	RunOptionsManagerImpl & RunOptionsManagerImpl::operator = (const RunOptionsManagerImpl & /*otherRunOptionsMgr*/)
	{
		throw std::runtime_error("Not implemented yet");
		return *this;
	}



	// Set project database. Reset all
	void RunOptionsManagerImpl::setDatabase(database::ProjectFileHandlerPtr pfh)
	{
		m_db = pfh;
		m_runOptionsIoTbl = m_db->getTable(s_runOptionsTableName);
	}

	ErrorHandler::ReturnCode  RunOptionsManagerImpl::getTemperatureGradient(double & temperatureGradient) {
		if (errorCode() != NoError) resetError();
		try
		{
			if (!m_runOptionsIoTbl) { throw Exception(NonexistingID) << s_runOptionsTableName << " table could not be found in project"; }
			database::Record * rec = m_runOptionsIoTbl->getRecord(static_cast<int>(0));
			if (!rec) { throw Exception(NonexistingID) << "No data found in RunOptionsIo table: "; }
			temperatureGradient = rec->getValue<double>(s_TemperatureGradientFieldName);
		}

		catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
		return NoError;
	}

	ErrorHandler::ReturnCode  RunOptionsManagerImpl::setTemperatureGradient(double & temperatureGradient) {
		if (errorCode() != NoError) resetError();
		try
		{
			if (!m_runOptionsIoTbl) { throw Exception(NonexistingID) << s_runOptionsTableName << " table could not be found in project"; }
			database::Record * rec = m_runOptionsIoTbl->getRecord(static_cast<int>(0));
			if (!rec) { throw Exception(NonexistingID) << "No data found in RunOptionsIo table: "; }
			rec->setValue<double>(s_TemperatureGradientFieldName, temperatureGradient);
		}
		catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
		return NoError;
	}

	ErrorHandler::ReturnCode  RunOptionsManagerImpl::getVelAlgorithm(std::string & velAlgorithm) {
		if (errorCode() != NoError) resetError();
		try
		{
			if (!m_runOptionsIoTbl) { throw Exception(NonexistingID) << s_runOptionsTableName << " table could not be found in project"; }
			database::Record * rec = m_runOptionsIoTbl->getRecord(static_cast<int>(0));
			if (!rec) { throw Exception(NonexistingID) << "No data found in RunOptionsIo table: "; }
			velAlgorithm = rec->getValue<std::string>(s_velAlgorithmFieldName);
		}

		catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
		return NoError;
	}
	ErrorHandler::ReturnCode  RunOptionsManagerImpl::setVelAlgorithm(const std::string & velAlgorithm) {
		if (errorCode() != NoError) resetError();
		try
		{
			if (!m_runOptionsIoTbl) { throw Exception(NonexistingID) << s_runOptionsTableName << " table could not be found in project"; }
			database::Record * rec = m_runOptionsIoTbl->getRecord(static_cast<int>(0));
			if (!rec) { throw Exception(NonexistingID) << "No data found in RunOptionsIo table: "; }
			rec->setValue<std::string>(s_velAlgorithmFieldName, velAlgorithm);
		}
		catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
		return NoError;
	}

	ErrorHandler::ReturnCode  RunOptionsManagerImpl::getVreAlgorithm(std::string & vreAlgorithm) {
		if (errorCode() != NoError) resetError();
		try
		{
			if (!m_runOptionsIoTbl) { throw Exception(NonexistingID) << s_runOptionsTableName << " table could not be found in project"; }
			database::Record * rec = m_runOptionsIoTbl->getRecord(static_cast<int>(0));
			if (!rec) { throw Exception(NonexistingID) << "No data found in RunOptionsIo table: "; }
			vreAlgorithm = rec->getValue<std::string>(s_vreAlgorithmFieldName);
		}

		catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
		return NoError;
	}
	ErrorHandler::ReturnCode  RunOptionsManagerImpl::setVreAlgorithm(const std::string & vreAlgorithm) {
		if (errorCode() != NoError) resetError();
		try
		{
			if (!m_runOptionsIoTbl) { throw Exception(NonexistingID) << s_runOptionsTableName << " table could not be found in project"; }
			database::Record * rec = m_runOptionsIoTbl->getRecord(static_cast<int>(0));
			if (!rec) { throw Exception(NonexistingID) << "No data found in RunOptionsIo table: "; }
			rec->setValue<std::string>(s_vreAlgorithmFieldName, vreAlgorithm);
		}
		catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
		return NoError;
	}

	ErrorHandler::ReturnCode  RunOptionsManagerImpl::getPTCouplingMode(std::string & PTCouplingMode) {
		if (errorCode() != NoError) resetError();
		try
		{
			if (!m_runOptionsIoTbl) { throw Exception(NonexistingID) << s_runOptionsTableName << " table could not be found in project"; }
			database::Record * rec = m_runOptionsIoTbl->getRecord(static_cast<int>(0));
			if (!rec) { throw Exception(NonexistingID) << "No data found in RunOptionsIo table: "; }
			PTCouplingMode = rec->getValue<std::string>(s_PTCouplingModeFieldName);
		}

		catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
		return NoError;
	}

	ErrorHandler::ReturnCode  RunOptionsManagerImpl::setPTCouplingMode(const std::string & PTCouplingMode) {
		if (errorCode() != NoError) resetError();
		try
		{
			if (!m_runOptionsIoTbl) { throw Exception(NonexistingID) << s_runOptionsTableName << " table could not be found in project"; }
			database::Record * rec = m_runOptionsIoTbl->getRecord(static_cast<int>(0));
			if (!rec) { throw Exception(NonexistingID) << "No data found in RunOptionsIo table: "; }
			rec->setValue<std::string>(s_PTCouplingModeFieldName, PTCouplingMode);
		}
		catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
		return NoError;
	}


	ErrorHandler::ReturnCode  RunOptionsManagerImpl::getCoupledMode(int & coupledMode) {
		if (errorCode() != NoError) resetError();
		try
		{
			if (!m_runOptionsIoTbl) { throw Exception(NonexistingID) << s_runOptionsTableName << " table could not be found in project"; }
			database::Record * rec = m_runOptionsIoTbl->getRecord(static_cast<int>(0));
			if (!rec) { throw Exception(NonexistingID) << "No data found in RunOptionsIo table: "; }
			coupledMode = rec->getValue<int>(s_coupledModeFieldName);
		}

		catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
		return NoError;
	}

	ErrorHandler::ReturnCode  RunOptionsManagerImpl::setCoupledMode(const int & coupledMode) {
		if (errorCode() != NoError) resetError();
		try
		{
			if (!m_runOptionsIoTbl) { throw Exception(NonexistingID) << s_runOptionsTableName << " table could not be found in project"; }
			database::Record * rec = m_runOptionsIoTbl->getRecord(static_cast<int>(0));
			if (!rec) { throw Exception(NonexistingID) << "No data found in RunOptionsIo table: "; }
			rec->setValue<int>(s_coupledModeFieldName, coupledMode);
		}
		catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
		return NoError;
	}

	ErrorHandler::ReturnCode  RunOptionsManagerImpl::getChemicalCompactionAlgorithm(std::string & chemCompactionAlgorithm) {
		if (errorCode() != NoError) resetError();
		try
		{
			if (!m_runOptionsIoTbl) { throw Exception(NonexistingID) << s_runOptionsTableName << " table could not be found in project"; }
			database::Record * rec = m_runOptionsIoTbl->getRecord(static_cast<int>(0));
			if (!rec) { throw Exception(NonexistingID) << "No data found in RunOptionsIo table: "; }
			chemCompactionAlgorithm = rec->getValue<std::string>(s_chemCompactionAlgorithmFieldName);
		}

		catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
		return NoError;
	}

	ErrorHandler::ReturnCode  RunOptionsManagerImpl::setChemicalCompactionAlgorithm(const std::string & chemCompactionAlgorithm) {
		if (errorCode() != NoError) resetError();
		try
		{
			if (!m_runOptionsIoTbl) { throw Exception(NonexistingID) << s_runOptionsTableName << " table could not be found in project"; }
			database::Record * rec = m_runOptionsIoTbl->getRecord(static_cast<int>(0));
			if (!rec) { throw Exception(NonexistingID) << "No data found in RunOptionsIo table: "; }
			rec->setValue<std::string>(s_chemCompactionAlgorithmFieldName, chemCompactionAlgorithm);
		}
		catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
		return NoError;
	}


	ErrorHandler::ReturnCode  RunOptionsManagerImpl::getOptimisationLevel(std::string & optimizationLevel) {
		if (errorCode() != NoError) resetError();
		try
		{
			if (!m_runOptionsIoTbl) { throw Exception(NonexistingID) << s_runOptionsTableName << " table could not be found in project"; }
			database::Record * rec = m_runOptionsIoTbl->getRecord(static_cast<int>(0));
			if (!rec) { throw Exception(NonexistingID) << "No data found in RunOptionsIo table: "; }
			optimizationLevel = rec->getValue<std::string>(s_optimisationLevelFieldName);
		}

		catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
		return NoError;
	}


	ErrorHandler::ReturnCode  RunOptionsManagerImpl::setOptimisationLevel(const std::string & optimizationLevel) {
		if (errorCode() != NoError) resetError();
		try
		{
			if (!m_runOptionsIoTbl) { throw Exception(NonexistingID) << s_runOptionsTableName << " table could not be found in project"; }
			database::Record * rec = m_runOptionsIoTbl->getRecord(static_cast<int>(0));
			if (!rec) { throw Exception(NonexistingID) << "No data found in RunOptionsIo table: "; }
			rec->setValue<std::string>(s_optimisationLevelFieldName, optimizationLevel);
		}
		catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
		return NoError;
	}

	ErrorHandler::ReturnCode  RunOptionsManagerImpl::getOptimalTotalTempDiff(double & OTTempDiff) {
		if (errorCode() != NoError) resetError();
		try
		{
			if (!m_runOptionsIoTbl) { throw Exception(NonexistingID) << s_runOptionsTableName << " table could not be found in project"; }
			database::Record * rec = m_runOptionsIoTbl->getRecord(static_cast<int>(0));
			if (!rec) { throw Exception(NonexistingID) << "No data found in RunOptionsIo table: "; }
			OTTempDiff = rec->getValue<double>(s_OTTempDiffFieldName);
		}

		catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
		return NoError;
	}

	ErrorHandler::ReturnCode  RunOptionsManagerImpl::setOptimalTotalTempDiff(const double & OTTempDiff) {
		if (errorCode() != NoError) resetError();
		try
		{
			if (!m_runOptionsIoTbl) { throw Exception(NonexistingID) << s_runOptionsTableName << " table could not be found in project"; }
			database::Record * rec = m_runOptionsIoTbl->getRecord(static_cast<int>(0));
			if (!rec) { throw Exception(NonexistingID) << "No data found in RunOptionsIo table: "; }
			rec->setValue<double>(s_OTTempDiffFieldName, OTTempDiff);
		}
		catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
		return NoError;
	}

	ErrorHandler::ReturnCode  RunOptionsManagerImpl::getOptimalSourceRockTempDiff(double & OSRTempDiff) {
		if (errorCode() != NoError) resetError();
		try
		{
			if (!m_runOptionsIoTbl) { throw Exception(NonexistingID) << s_runOptionsTableName << " table could not be found in project"; }
			database::Record * rec = m_runOptionsIoTbl->getRecord(static_cast<int>(0));
			if (!rec) { throw Exception(NonexistingID) << "No data found in RunOptionsIo table: "; }
			OSRTempDiff = rec->getValue<double>(s_OSRTempDiffFieldName);
		}

		catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
		return NoError;
	}

	ErrorHandler::ReturnCode  RunOptionsManagerImpl::setOptimalSourceRockTempDiff(const double & OSRTempDiff) {
		if (errorCode() != NoError) resetError();
		try
		{
			if (!m_runOptionsIoTbl) { throw Exception(NonexistingID) << s_runOptionsTableName << " table could not be found in project"; }
			database::Record * rec = m_runOptionsIoTbl->getRecord(static_cast<int>(0));
			if (!rec) { throw Exception(NonexistingID) << "No data found in RunOptionsIo table: "; }
			rec->setValue<double>(s_OSRTempDiffFieldName, OSRTempDiff);
		}
		catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
		return NoError;
	}

	ErrorHandler::ReturnCode  RunOptionsManagerImpl::getOptimalTotalPresDiff(double & OTPresDiff) {
		if (errorCode() != NoError) resetError();
		try
		{
			if (!m_runOptionsIoTbl) { throw Exception(NonexistingID) << s_runOptionsTableName << " table could not be found in project"; }
			database::Record * rec = m_runOptionsIoTbl->getRecord(static_cast<int>(0));
			if (!rec) { throw Exception(NonexistingID) << "No data found in RunOptionsIo table: "; }
			OTPresDiff = rec->getValue<double>(s_OTPresDiffFieldName);
		}

		catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
		return NoError;
	}


	ErrorHandler::ReturnCode  RunOptionsManagerImpl::setOptimalTotalPresDiff(const double & OTPresDiff) {
		if (errorCode() != NoError) resetError();
		try
		{
			if (!m_runOptionsIoTbl) { throw Exception(NonexistingID) << s_runOptionsTableName << " table could not be found in project"; }
			database::Record * rec = m_runOptionsIoTbl->getRecord(static_cast<int>(0));
			if (!rec) { throw Exception(NonexistingID) << "No data found in RunOptionsIo table: "; }
			rec->setValue<double>(s_OTPresDiffFieldName, OTPresDiff);
		}
		catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
		return NoError;
	}

	ErrorHandler::ReturnCode RunOptionsManagerImpl::getWorkflowType(std::string & workflow) {
		if (errorCode() != NoError) resetError();
		try
		{
			if (!m_runOptionsIoTbl) { throw Exception(NonexistingID) << s_runOptionsTableName << " table could not be found in project"; }
			database::Record * rec = m_runOptionsIoTbl->getRecord(static_cast<int>(0));
			if (!rec) { throw Exception(NonexistingID) << "No data found in RunOptionsIo table: "; }
			workflow = rec->getValue<std::string>(s_workflowTypeFieldName);
		}

		catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
		return NoError;
	}
}