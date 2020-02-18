//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SgsUpgradeManager.h"
#include "SgsConverter.h"

//utilities
#include "LogHandler.h"

//cmbAPI
#include "cmbAPI.h"
#include "SourceRockManager.h"
#include "SgsManager.h"


//DataAccess
#include "ProjectHandle.h"

using namespace mbapi;

//------------------------------------------------------------//

Prograde::SgsUpgradeManager::SgsUpgradeManager(Model& model):
	IUpgradeManager("SGS upgrade manager"), m_model(model)
{
	const auto ph = m_model.projectHandle();
	if (ph == nullptr) {
		throw std::invalid_argument(getName() + " cannot retreive the project handle from Cauldron data model");
	}
	m_ph = ph;
}

//------------------------------------------------------------//
void Prograde::SgsUpgradeManager::upgrade() {
	Prograde::SgsConverter modelConverter;

	int legacyApplyAdsorption, legacyAdsorptionTOCDependent, legacyComputeOTGC, bpa2AdsorptionTOCDependent;
	std::string legacyAdsorptionCapacityFunctionName, bpa2AdsorptionCapacityFunctionName, bpa2WhichAdsorptionSimulator;
	int isToc = 1;
	double legacyCoefficientA, legacyCoefficientB, bpa2CoefficientA, bpa2CoefficientB;

	database::Table * sourceRock_table = m_ph->getTable("SourceRockLithoIoTbl");
	database::Table * iws_table = m_ph->getTable("IrreducibleWaterSaturationIoTbl");
	for (size_t sourceRockId = 0; sourceRockId < sourceRock_table->size(); ++sourceRockId)
	{		
		m_model.sourceRockManager().getAdsoptionList(sourceRockId, legacyApplyAdsorption, legacyAdsorptionTOCDependent, legacyComputeOTGC, legacyAdsorptionCapacityFunctionName);
		
		if(legacyApplyAdsorption)
		{			
			if (legacyAdsorptionTOCDependent)
			{
				if (legacyComputeOTGC)
					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "'" << legacyAdsorptionCapacityFunctionName << "'" << " adsorption model is selected and OTGC is on";
				else
					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "'" << legacyAdsorptionCapacityFunctionName << "'" << " adsorption model is selected and OTGC is off";
				
				bpa2AdsorptionTOCDependent = modelConverter.upgradeAdsorptionTOCDependent(legacyAdsorptionTOCDependent);
				bpa2AdsorptionCapacityFunctionName = modelConverter.upgradeAdsorptionCapacityFunctionName(legacyAdsorptionTOCDependent, legacyAdsorptionCapacityFunctionName);
				m_model.sourceRockManager().setAdsorptionTOCDependent(sourceRockId, bpa2AdsorptionTOCDependent);
				m_model.sourceRockManager().setAdsorptionCapacityFunctionName(sourceRockId, bpa2AdsorptionCapacityFunctionName);				
				if(isToc)
				{ 
					isToc = 0;
					database::Table * langmuirAdsorptionCapacityIsothermSet_table = m_ph->getTable("LangmuirAdsorptionCapacityIsothermSetIoTbl");
					std::string langName = m_model.tableValueAsString("LangmuirAdsorptionCapacityTOCFunctionIoTbl", 0, "LangmuirName");
					size_t id = langmuirAdsorptionCapacityIsothermSet_table->size();
					m_model.addRowToTable("LangmuirAdsorptionCapacityIsothermSetIoTbl");
					m_model.addRowToTable("LangmuirAdsorptionCapacityIsothermSetIoTbl");
					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "updating LangmuirAdsorptionCapacityIsothermSetIoTbl with the following parameters";
					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "LangmuirName				LangmuirTemperature[C]		LangmuirPressure[MPa]	LangmuirVolume[cc/gm]";
					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Low Langmuir Isotherm				20						6.8						2.0E-5";
					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Low Langmuir Isotherm				115						7.48					1.0E-5";

					m_model.sgsManager().setLangmuirName(id, "Low Langmuir Isotherm");
					m_model.sgsManager().setLangmuirTemperature(id, 20);
					m_model.sgsManager().setLangmuirPressure(id, 6.8);
					m_model.sgsManager().setLangmuirVolume(id, 2.0e-5);

					id += 1;
					m_model.sgsManager().setLangmuirName(id, "Low Langmuir Isotherm");
					m_model.sgsManager().setLangmuirTemperature(id, 115);
					m_model.sgsManager().setLangmuirPressure(id, 7.48);
					m_model.sgsManager().setLangmuirVolume(id, 1.0e-5);
										
					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Clear LangmuirAdsorptionCapacityTOCFunctionIoTbl";
					m_model.clearTable("LangmuirAdsorptionCapacityTOCFunctionIoTbl");
				}				

			}
			else
			{
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "'" << legacyAdsorptionCapacityFunctionName << "'" << " adsorption model is selected and OTGC is "<< "'" << legacyComputeOTGC << "'";
				bpa2AdsorptionCapacityFunctionName = modelConverter.upgradeAdsorptionCapacityFunctionName(legacyAdsorptionTOCDependent, legacyAdsorptionCapacityFunctionName);
				m_model.sourceRockManager().setAdsorptionCapacityFunctionName(sourceRockId, bpa2AdsorptionCapacityFunctionName);
				database::Table * langmuirAdsorptionCapacityIsothermSet_table = m_ph->getTable("LangmuirAdsorptionCapacityIsothermSetIoTbl");				
				size_t tableSize = langmuirAdsorptionCapacityIsothermSet_table->size();
				if (tableSize)
				{
					for (size_t Id = 0; Id < tableSize; ++Id)
					{
						if("Langmuir Isotherm" == m_model.sgsManager().getLangmuirName(Id))
						{
							m_model.sgsManager().setLangmuirName(Id, "Default Langmuir Isotherm");
						}
					}

				}


			}			
			bpa2WhichAdsorptionSimulator = modelConverter.upgradeWhichAdsorptionSimulator(legacyComputeOTGC);			
			m_model.sourceRockManager().setWhichAdsorptionSimulator(sourceRockId, bpa2WhichAdsorptionSimulator);
		}
		
	}

	for (size_t iwsId = 0; iwsId < iws_table->size(); ++iwsId)
	{
		legacyCoefficientA = m_model.sgsManager().getCoefficientA(iwsId);
		legacyCoefficientB = m_model.sgsManager().getCoefficientB(iwsId);
		modelConverter.upgradeIrreducibleWaterSaturationCoefficients(legacyCoefficientA, legacyCoefficientB, bpa2CoefficientA, bpa2CoefficientB);
		m_model.sgsManager().setCoefficientA(iwsId, bpa2CoefficientA);
		m_model.sgsManager().setCoefficientB(iwsId, bpa2CoefficientB);
	}
}
