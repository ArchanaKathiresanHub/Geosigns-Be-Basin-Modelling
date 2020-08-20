//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "TopBoundaryUpgradeManager.h"

//utilities
#include "LogHandler.h"

//cmbAPI
#include "cmbAPI.h"
#include "FluidManager.h"

//DataAccess
#include "ProjectHandle.h"
#include "RunParameters.h"

#include <algorithm>

using namespace mbapi;


//------------------------------------------------------------//
Prograde::TopBoundaryUpgradeManager::TopBoundaryUpgradeManager(Model& model) :
	IUpgradeManager("topBoundary upgrade manager"), m_model(model)
{
	const auto ph = m_model.projectHandle();
	if (ph == nullptr) {
		throw std::invalid_argument(getName() + " cannot retrieve the project handle from Cauldron data model");
	}
	m_ph = ph;
}


//------------------------------------------------------------//

void Prograde::TopBoundaryUpgradeManager::upgrade() {
	upgradeSurfaceDepthIoTable();
	upgradeSurfaceTempIoTable();
}


void Prograde::TopBoundaryUpgradeManager::upgradeSurfaceDepthIoTable() {

	// Updating the Depth and DepthGrid values of SurfaceDepthIoTbl table at age = 0
	// Currently blank but cauldron reads the values of the data at age = 0 Ma from StratIoTbl
	database::Table* surfaceDepthIo_tbl = m_ph->getTable("SurfaceDepthIoTbl");
	database::Record* recSurface = surfaceDepthIo_tbl->getRecord(0);
	// we know the first row of SurfaceDepthIoTbl and StratIoTBl is always at age=0;
	std::string gridMapSD = recSurface->getValue<std::string>("DepthGrid");
	double depthSD = recSurface->getValue<double>("Depth");
	if (gridMapSD == "" && depthSD == -9999)
	{
		// get the record from stratIoTbl
		database::Table* stratIo_tbl = m_ph->getTable("StratIoTbl");
		database::Record* recStrat = stratIo_tbl->getRecord(0);
		std::string gridMapStrat = recStrat->getValue<std::string>("DepthGrid");
		double depthStrat = recStrat->getValue<double>("Depth");
		// set the record to SurfaceDepthIoTbl
		recSurface->setValue<std::string>("DepthGrid", gridMapStrat);
		recSurface->setValue<double>("Depth", depthStrat);
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Updating SurfaceDepthIoTbl";
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> SurfaceDepthIoTbl is updated for the record at age = 0Ma by using the data from the StratIoTbl at age = 0Ma";
	}

	auto surfIDs = m_model.topBoundaryManager().getSurfaceDepthIDs();
	mbapi::TopBoundaryManager&  topBoundaryLocal = m_model.topBoundaryManager();

	std::vector<double> original_surfaceAgeList;
	std::vector<double> updated_surfaceAgeList;

	for (auto surfID : surfIDs) {

		double surfAge;
		auto err = m_model.topBoundaryManager().getSurfaceDepthAge(surfID, surfAge);
		if (ErrorHandler::NoError != err)
			throw ErrorHandler::Exception(topBoundaryLocal.errorCode()) << topBoundaryLocal.errorMessage();
		original_surfaceAgeList.push_back(surfAge);
		updated_surfaceAgeList.push_back(surfAge);
	}

	// do the age transformations and set them to the records
	bool isClipped = clipAndBufferSurfaceAges(updated_surfaceAgeList);
	if (isClipped)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> SurfaceDepth Layer age values are clipped to a maximum of 999, anything above 999 is placed between 998 and 999";
		for (size_t i = 0; i < surfIDs.size(); ++i) {
			if (updated_surfaceAgeList[i] != original_surfaceAgeList[i]) {
				auto err = m_model.topBoundaryManager().setSurfaceDepthAge(surfIDs[i], updated_surfaceAgeList[i]);
				if (ErrorHandler::NoError != err)
					throw ErrorHandler::Exception(topBoundaryLocal.errorCode()) << topBoundaryLocal.errorMessage();
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> SurfaceDepth Layer age is updated from " << original_surfaceAgeList[i] << " to " << updated_surfaceAgeList[i];
			}
		}
	}
	else return;
}

void Prograde::TopBoundaryUpgradeManager::upgradeSurfaceTempIoTable() {
	auto surfIDs = m_model.topBoundaryManager().getSurfaceTempIDs();
	mbapi::TopBoundaryManager&  topBoundaryLocal = m_model.topBoundaryManager();

	std::vector<double> original_surfaceAgeList;
	std::vector<double> updated_surfaceAgeList;

	for (auto surfID : surfIDs) {

		double surfAge;
		auto err = m_model.topBoundaryManager().getSurfaceTempAge(surfID, surfAge);
		if (ErrorHandler::NoError != err)
			throw ErrorHandler::Exception(topBoundaryLocal.errorCode()) << topBoundaryLocal.errorMessage();
		original_surfaceAgeList.push_back(surfAge);
		updated_surfaceAgeList.push_back(surfAge);
	}

	// do the age transformations and set them to the records
	bool isClipped = clipAndBufferSurfaceAges(updated_surfaceAgeList);
	if(isClipped)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> SurfaceTemperature Layer age values are clipped to a maximum of 999, anything above 999 is placed between 998 and 999";
		for (size_t i = 0; i < surfIDs.size(); ++i) {
			if (updated_surfaceAgeList[i] != original_surfaceAgeList[i])
			{
				auto err = m_model.topBoundaryManager().setSurfaceTempAge(surfIDs[i], updated_surfaceAgeList[i]);
				if (ErrorHandler::NoError != err)
					throw ErrorHandler::Exception(topBoundaryLocal.errorCode()) << topBoundaryLocal.errorMessage();
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> SurfaceTemperature Layer age is updated from " << original_surfaceAgeList[i] << " to " << updated_surfaceAgeList[i];
			}
		}
	}
}


bool Prograde::TopBoundaryUpgradeManager::clipAndBufferSurfaceAges(std::vector<double>& surfaceAgeList)
{
	std::vector<pair<size_t, double>> bufferedAges;
	double bufferStartAge = 998.0;
	double maxAgeLimit = 999.0;

	size_t maxAgeOverflowCount = 0;

	for (size_t i = 0; i < surfaceAgeList.size(); ++i)
	{
		if (surfaceAgeList[i] > 998.0)
		{
			bufferedAges.push_back(std::make_pair(i, surfaceAgeList[i]));
		}
		if (surfaceAgeList[i] > 999.0) maxAgeOverflowCount++;
	}

	if (bufferedAges.size() <= 0 || maxAgeOverflowCount <= 0)
		return false;

	double bufferInterval = (maxAgeLimit - bufferStartAge) / bufferedAges.size();

	// here the pairs are sorted according to the age
	std::sort(bufferedAges.begin(), bufferedAges.end(), [](const std::pair<size_t, double>& lhs, const std::pair<size_t, double>& rhs)
	{
		return lhs.second < rhs.second;
	});

	// modify the ages according to the interval
	// here we make them equispaced since the difference is getting clipped and we loose the original information about the ages
	double currentBuffer = bufferInterval;
	for (size_t i = 0; i < bufferedAges.size(); ++i)
	{
		surfaceAgeList[bufferedAges[i].first] = bufferStartAge + currentBuffer;
		currentBuffer += bufferInterval;
	}

	return true;
}
