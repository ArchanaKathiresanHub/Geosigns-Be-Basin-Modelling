//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "RunOptionsUpgradeManager.h"
#include "RunOptionsModelConverter.h"

//utilities
#include "LogHandler.h"

//cmbAPI
#include "cmbAPI.h"
#include "RunOptionsManager.h"

//DataAccess
#include "ProjectHandle.h"
#include "RunParameters.h"

using namespace mbapi;

//------------------------------------------------------------//

Prograde::RunOptionsUpgradeManager::RunOptionsUpgradeManager(Model& model) :
   IUpgradeManager("Run Options upgrade manager"), m_model(model)
{
   const auto ph = m_model.projectHandle();
   if (ph == nullptr) {
      throw std::invalid_argument(getName() + " cannot retrieve the project handle from Cauldron data model");
   }
   m_ph = ph;
}

//------------------------------------------------------------//

void Prograde::RunOptionsUpgradeManager::upgrade() {


	Prograde::RunOptionsModelConverter runOptionsModelConverter;
	std::string workflowType;
	double temperatureGradient;
	std::string velAlgorithm;
	std::string vreAlgorithm;
	std::string PTCouplingMode;
	int coupledMode;
	std::string chemCompactionAlgorithm;
	std::string optimizationLevel;
	double OTTempDiff;
	double OSRTempDiff;
	double OTPresDiff;

	// check workflow type
	m_model.runOptionsManager().getWorkflowType(workflowType);
	if (workflowType.compare("DarcyFlow") == 0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Error> DarcyFlow workflowType detected! Migration from BPA to BPA2 Basin Aborted...";
		exit(12);
	}

	// get all other items for upgrade
	m_model.runOptionsManager().getTemperatureGradient(temperatureGradient);
	m_model.runOptionsManager().getVelAlgorithm(velAlgorithm);
	m_model.runOptionsManager().getVreAlgorithm(vreAlgorithm);
	m_model.runOptionsManager().getPTCouplingMode(PTCouplingMode);
	m_model.runOptionsManager().getCoupledMode(coupledMode);
	m_model.runOptionsManager().getChemicalCompactionAlgorithm(chemCompactionAlgorithm);
	m_model.runOptionsManager().getOptimisationLevel(optimizationLevel);
	m_model.runOptionsManager().getOptimalTotalTempDiff(OTTempDiff);
	m_model.runOptionsManager().getOptimalSourceRockTempDiff(OSRTempDiff);
	m_model.runOptionsManager().getOptimalTotalPresDiff(OTPresDiff);

	// check if PTCouplingMode needs to be upgraded
	runOptionsModelConverter.upgradePTCouplingMode(PTCouplingMode, coupledMode);
	m_model.runOptionsManager().setCoupledMode(coupledMode);
	m_model.runOptionsManager().setPTCouplingMode(PTCouplingMode);

	temperatureGradient = runOptionsModelConverter.upgradeTemperatureRange(temperatureGradient, "Temperature_Gradient");
	m_model.runOptionsManager().setTemperatureGradient(temperatureGradient);

	velAlgorithm = runOptionsModelConverter.upgradeVelAlgorithm(velAlgorithm);
	m_model.runOptionsManager().setVelAlgorithm(velAlgorithm);

	vreAlgorithm = runOptionsModelConverter.upgradeVreAlgorithm(vreAlgorithm);
	m_model.runOptionsManager().setVreAlgorithm(vreAlgorithm);

	chemCompactionAlgorithm = runOptionsModelConverter.upgradeChemicalCompactionAlgorithm(chemCompactionAlgorithm);
	m_model.runOptionsManager().setChemicalCompactionAlgorithm(chemCompactionAlgorithm);
#ifdef _PROGRADE_DEPRECATED   // Geometric-loop optimization retained "As-is"
	optimizationLevel = runOptionsModelConverter.upgradeOptimisationLevel(optimizationLevel);
	m_model.runOptionsManager().setOptimisationLevel(optimizationLevel);
#endif // _PROGRADE_DEPRECATED   
	OTTempDiff = runOptionsModelConverter.upgradeTemperatureRange(OTTempDiff, "OptimalTotalTempDiff");
	m_model.runOptionsManager().setOptimalTotalTempDiff(OTTempDiff);

	OSRTempDiff = runOptionsModelConverter.upgradeTemperatureRange(OSRTempDiff, "OptimalSourceRockTempDiff");
	m_model.runOptionsManager().setOptimalSourceRockTempDiff(OSRTempDiff);

	OTPresDiff = runOptionsModelConverter.upgradePressureRange(OTPresDiff, "OptimalTotalPresDiff");
	m_model.runOptionsManager().setOptimalTotalPresDiff(OTPresDiff);

	// Updating the Legacy flag in RunOptionsIoTbl from 1 to 0
	// Getter and Setter function for Legacy flag is not available in RunOptionsManagerImpl
	int legacy = 1;
	database::Table* RunOptionsIo_Tbl = m_ph->getTable("RunOptionsIoTbl");
	database::Record* rec = RunOptionsIo_Tbl->getRecord(static_cast<int>(0));
	legacy = rec->getValue<int>("Legacy");
	int updatedLegacy = runOptionsModelConverter.upgradeLegacyFlag(legacy);
	if (updatedLegacy != legacy)
	{
		rec->setValue<int>("Legacy", updatedLegacy);
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> " << "Legacy flag is updated from " << legacy << " to " << updatedLegacy;
	}

}