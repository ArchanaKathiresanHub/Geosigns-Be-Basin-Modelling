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
		throw std::invalid_argument(getName() + " cannot retreive the project handle from Cauldron data model");
	}
	m_ph = ph;
}


//------------------------------------------------------------//

void Prograde::TopBoundaryUpgradeManager::upgrade() {
	upgradeSurfaceDepthIoTable();
	upgradeSurfaceTempIoTable();
}


void Prograde::TopBoundaryUpgradeManager::upgradeSurfaceDepthIoTable() {
	auto surfIDs = m_model.topBoundaryManager().getSurfaceDepthIDs();

	std::vector<double> original_surfaceAgeList;
	std::vector<double> updated_surfaceAgeList;

	for (auto surfID : surfIDs) {

		double surfAge;
		m_model.topBoundaryManager().getSurfaceDepthAge(surfID, surfAge);
		original_surfaceAgeList.push_back(surfAge);
		updated_surfaceAgeList.push_back(surfAge);
	}

	// do the age transformations and set them to the records
	bool isClipped = clipAndBufferSurfaceAges(updated_surfaceAgeList);

	if (!isClipped){
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "SurfaceDepth Layer age Values not needed to be clipped";
		return;
	}
	else{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "SurfaceDepth Layer age values are clipped to a maximum of 999, anything above 999 is placed between 998 and 999";
		for (size_t i = 0; i < surfIDs.size(); ++i) {
			if (updated_surfaceAgeList[i] != original_surfaceAgeList[i]) {
				m_model.topBoundaryManager().setSurfaceDepthAge(surfIDs[i], updated_surfaceAgeList[i]);
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "SurfaceDepth Layer age is updated from " << original_surfaceAgeList[i] << " to " << updated_surfaceAgeList[i];
			}	
		}
	}
}

void Prograde::TopBoundaryUpgradeManager::upgradeSurfaceTempIoTable() {
	auto surfIDs = m_model.topBoundaryManager().getSurfaceTempIDs();

	std::vector<double> original_surfaceAgeList;
	std::vector<double> updated_surfaceAgeList;

	for (auto surfID : surfIDs) {

		double surfAge;
		m_model.topBoundaryManager().getSurfaceTempAge(surfID, surfAge);
		original_surfaceAgeList.push_back(surfAge);
		updated_surfaceAgeList.push_back(surfAge);
	}

	// do the age transformations and set them to the records
	bool isClipped = clipAndBufferSurfaceAges(updated_surfaceAgeList);

	if (!isClipped)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "SurfaceTemperature Layer age Values not needed to be clipped";
	}
	else
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "SurfaceTemperature Layer age values are clipped to a maximum of 999, anything above 999 is placed between 998 and 999";
		for (size_t i = 0; i < surfIDs.size(); ++i) {
			if (updated_surfaceAgeList[i] != original_surfaceAgeList[i])
			{
				m_model.topBoundaryManager().setSurfaceTempAge(surfIDs[i], updated_surfaceAgeList[i]);
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "SurfaceTemperature Layer age is updated from " << original_surfaceAgeList[i] << " to " << updated_surfaceAgeList[i];
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
