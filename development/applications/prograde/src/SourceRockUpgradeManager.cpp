//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SourceRockUpgradeManager.h"
#include "SourceRockConverter.h"

//utilities
#include "LogHandler.h"

//cmbAPI
#include "cmbAPI.h"
#include "StratigraphyManager.h"
#include "SourceRockManager.h"


//DataAccess
#include "ProjectHandle.h"

using namespace mbapi;

//------------------------------------------------------------//

Prograde::SourceRockUpgradeManager::SourceRockUpgradeManager(Model& model) :
	IUpgradeManager("SourceRock upgrade manager"), m_model(model)
{
	const auto ph = m_model.projectHandle();
	if (ph == nullptr) {
		throw std::invalid_argument(getName() + " cannot retreive the project handle from Cauldron data model");
	}
	m_ph = ph;
}

//------------------------------------------------------------//
void Prograde::SourceRockUpgradeManager::upgrade() {
	Prograde::SourceRockConverter modelConverter;

	database::Table * stratIo_table = m_ph->getTable("StratIoTbl");
	database::Table * sourceRock_table = m_ph->getTable("SourceRockLithoIoTbl");
	std::string layerName;
	std::vector<std::string>sourceRockTypeName;
	std::vector<std::string>bpa2SourceRockTypeName;
	double bpaHcVre05, bpaScVre05, bpaEa, bpaAsphalteneDE, bpaResinDE, bpaC15AroDE, bpaC15SatDE, VESlimit, VREthreshold;
	double hcVre05, scVre05, ea;
	std::string legacySourceRockType, legacyBaseSourceRockType, bpa2BaseSourceRockType, bpa2SourceRockType, legacyVESlimitIndicator, legacyVREoptimization;
	
	for (size_t id = 0; id < stratIo_table->size(); ++id)
	{
		bpa2SourceRockTypeName.resize(0);
		sourceRockTypeName.resize(0);
		if (m_model.stratigraphyManager().isSourceRockMixingEnabled(id) && m_model.stratigraphyManager().isSourceRockActive(id))
		{
			layerName = m_model.stratigraphyManager().layerName(id);
			sourceRockTypeName.resize(0);
			sourceRockTypeName = m_model.stratigraphyManager().sourceRockTypeName(id);			
			if (sourceRockTypeName.size()>0)
			{				
				for (size_t sourceRockId = 0; sourceRockId < sourceRock_table->size(); ++sourceRockId)
				{
					if (m_model.sourceRockManager().layerName(sourceRockId).compare(layerName) == 0)
					{							
						if (m_model.sourceRockManager().sourceRockType(sourceRockId).compare(sourceRockTypeName[0]) == 0)
						{

							std::string baseSourceRockTypeName = m_model.sourceRockManager().baseSourceRockType(sourceRockId);
							
							LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "For source rock mixing, source rock type and base source rock type are respectively '" << sourceRockTypeName[0] << "' and '" << baseSourceRockTypeName << "'";
							bpaHcVre05 = m_model.sourceRockManager().hcIni(sourceRockId);
							bpaScVre05 = m_model.sourceRockManager().scIni(sourceRockId);
							bpaEa = m_model.sourceRockManager().preAsphActEnergy(sourceRockId);
							bpaAsphalteneDE = m_model.sourceRockManager().getAsphalteneDiffusionEnergy(sourceRockId);
							bpaResinDE = m_model.sourceRockManager().getResinDiffusionEnergy(sourceRockId);
							bpaC15AroDE = m_model.sourceRockManager().getC15AroDiffusionEnergy(sourceRockId);
							bpaC15SatDE = m_model.sourceRockManager().getC15SatDiffusionEnergy(sourceRockId);
							legacyVESlimitIndicator = m_model.sourceRockManager().getVESlimitIndicator(sourceRockId);
							VESlimit = m_model.sourceRockManager().getVESlimit(sourceRockId);
							legacyVREoptimization = m_model.sourceRockManager().getVREoptimization(sourceRockId);
							VREthreshold = m_model.sourceRockManager().getVREthreshold(sourceRockId);

							hcVre05 = modelConverter.upgradeHcVre05(sourceRockTypeName[0], baseSourceRockTypeName, bpaHcVre05, bpaScVre05);
							scVre05 = modelConverter.upgradeScVre05(sourceRockTypeName[0], baseSourceRockTypeName, bpaScVre05);
							ea = modelConverter.upgradeEa(sourceRockTypeName[0], baseSourceRockTypeName, bpaEa, bpaScVre05);
							modelConverter.upgradeSourceRockName(sourceRockTypeName[0], baseSourceRockTypeName, bpaScVre05, bpa2SourceRockType, bpa2BaseSourceRockType);
							modelConverter.upgradeDiffusionEnergy(baseSourceRockTypeName, bpaScVre05, bpaAsphalteneDE, bpaResinDE, bpaC15AroDE, bpaC15SatDE);
							modelConverter.upgradeVREthreshold(legacyVREoptimization, VREthreshold);
							modelConverter.upgradeVESlimit(legacyVESlimitIndicator, VESlimit);
							

							m_model.sourceRockManager().setHCIni(sourceRockId, hcVre05);
							m_model.sourceRockManager().setSCIni(sourceRockId, scVre05);
							m_model.sourceRockManager().setPreAsphActEnergy(sourceRockId, ea);							
							m_model.sourceRockManager().setSourceRockType(sourceRockId, bpa2SourceRockType);
							m_model.sourceRockManager().setBaseSourceRockType(sourceRockId, bpa2BaseSourceRockType);
							m_model.sourceRockManager().setAsphalteneDiffusionEnergy(sourceRockId, bpaAsphalteneDE);
							m_model.sourceRockManager().setResinDiffusionEnergy(sourceRockId, bpaResinDE);
							m_model.sourceRockManager().setC15AroDiffusionEnergy(sourceRockId, bpaC15AroDE);
							m_model.sourceRockManager().setC15SatDiffusionEnergy(sourceRockId, bpaC15SatDE);
							m_model.sourceRockManager().setVREthreshold(sourceRockId, VREthreshold);
							m_model.sourceRockManager().setVESlimit(sourceRockId, VESlimit);
							
							
							bpa2SourceRockTypeName.emplace_back(bpa2SourceRockType);

							size_t found0 = sourceRockTypeName[0].find("Literature");
							size_t found1 = sourceRockTypeName[1].find("Literature");
							if (found0 != std::string::npos || found1 != std::string::npos || baseSourceRockTypeName.compare("Type_I_II_Cenozoic_Marl_kin") == 0)
							{
								LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Note: Literature source is identified in source rock mixing and BPA2 does not allow literature source rocks in source rock mixing'";
								
							}
						}
						else if (m_model.sourceRockManager().sourceRockType(sourceRockId).compare(sourceRockTypeName[1]) == 0)
						{
							std::string baseSourceRockTypeName = m_model.sourceRockManager().baseSourceRockType(sourceRockId);							
							LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "For source rock mixing, source rock type and base source rock type are respectively '" << sourceRockTypeName[1] << "' and '" << baseSourceRockTypeName<<"'";
							bpaHcVre05 = m_model.sourceRockManager().hcIni(sourceRockId);
							bpaScVre05 = m_model.sourceRockManager().scIni(sourceRockId);
							bpaEa = m_model.sourceRockManager().preAsphActEnergy(sourceRockId);
							bpaAsphalteneDE = m_model.sourceRockManager().getAsphalteneDiffusionEnergy(sourceRockId);
							bpaResinDE = m_model.sourceRockManager().getResinDiffusionEnergy(sourceRockId);
							bpaC15AroDE = m_model.sourceRockManager().getC15AroDiffusionEnergy(sourceRockId);
							bpaC15SatDE = m_model.sourceRockManager().getC15SatDiffusionEnergy(sourceRockId);
							legacyVREoptimization = m_model.sourceRockManager().getVREoptimization(sourceRockId);
							VREthreshold = m_model.sourceRockManager().getVREthreshold(sourceRockId);
							legacyVESlimitIndicator = m_model.sourceRockManager().getVESlimitIndicator(sourceRockId);
							VESlimit = m_model.sourceRockManager().getVESlimit(sourceRockId);
							

							hcVre05 = modelConverter.upgradeHcVre05(sourceRockTypeName[1], baseSourceRockTypeName, bpaHcVre05, bpaScVre05);
							scVre05 = modelConverter.upgradeScVre05(sourceRockTypeName[1], baseSourceRockTypeName, bpaScVre05);
							ea = modelConverter.upgradeEa(sourceRockTypeName[1], baseSourceRockTypeName, bpaEa, bpaScVre05);
							modelConverter.upgradeSourceRockName(sourceRockTypeName[1], baseSourceRockTypeName, bpaScVre05, bpa2SourceRockType, bpa2BaseSourceRockType);
							modelConverter.upgradeDiffusionEnergy(baseSourceRockTypeName, bpaScVre05, bpaAsphalteneDE, bpaResinDE, bpaC15AroDE, bpaC15SatDE);
							modelConverter.upgradeVREthreshold(legacyVREoptimization, VREthreshold);
							modelConverter.upgradeVESlimit(legacyVESlimitIndicator, VESlimit);
							

							m_model.sourceRockManager().setHCIni(sourceRockId, hcVre05);
							m_model.sourceRockManager().setSCIni(sourceRockId, scVre05);
							m_model.sourceRockManager().setPreAsphActEnergy(sourceRockId, ea);							
							m_model.sourceRockManager().setSourceRockType(sourceRockId, bpa2SourceRockType);
							m_model.sourceRockManager().setBaseSourceRockType(sourceRockId, bpa2BaseSourceRockType);
							m_model.sourceRockManager().setAsphalteneDiffusionEnergy(sourceRockId, bpaAsphalteneDE);
							m_model.sourceRockManager().setResinDiffusionEnergy(sourceRockId, bpaResinDE);
							m_model.sourceRockManager().setC15AroDiffusionEnergy(sourceRockId, bpaC15AroDE);
							m_model.sourceRockManager().setC15SatDiffusionEnergy(sourceRockId, bpaC15SatDE);
							m_model.sourceRockManager().setVREthreshold(sourceRockId, VREthreshold);
							m_model.sourceRockManager().setVESlimit(sourceRockId, VESlimit);
							

							bpa2SourceRockTypeName.emplace_back(bpa2SourceRockType);

							size_t found0 = sourceRockTypeName[0].find("Literature");
							size_t found1 = sourceRockTypeName[1].find("Literature");
							if (found0 != std::string::npos || found1 != std::string::npos || baseSourceRockTypeName.compare("Type_I_II_Cenozoic_Marl_kin") == 0)
							{
								LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Note: Literature source is identified in source rock mixing and BPA2 does not allow literature source rocks in source rock mixing'";
								
							}

						}
						m_model.stratigraphyManager().setSourceRockTypeName(id, bpa2SourceRockTypeName);
					}
				}
			}
		}
		else if (!m_model.stratigraphyManager().isSourceRockMixingEnabled(id) && m_model.stratigraphyManager().isSourceRockActive(id))
		{

			sourceRockTypeName.resize(0);
			sourceRockTypeName = m_model.stratigraphyManager().sourceRockTypeName(id);
			//updating the source rock type and base source rock type in SourceRockLithoIoTbl	
			for (size_t sourceRockId = 0; sourceRockId < sourceRock_table->size(); ++sourceRockId)
			{
				if (m_model.sourceRockManager().sourceRockType(sourceRockId).compare(sourceRockTypeName[0]) == 0)
				{
					std::string baseSourceRockTypeName = m_model.sourceRockManager().baseSourceRockType(sourceRockId);

					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "For source rock non-mixing, source rock type and base source rock type are respectively '" << sourceRockTypeName[0] << "' and '" << baseSourceRockTypeName << "'";

					bpaScVre05 = m_model.sourceRockManager().scIni(sourceRockId);

					bpaAsphalteneDE = m_model.sourceRockManager().getAsphalteneDiffusionEnergy(sourceRockId);
					bpaResinDE = m_model.sourceRockManager().getResinDiffusionEnergy(sourceRockId);
					bpaC15AroDE = m_model.sourceRockManager().getC15AroDiffusionEnergy(sourceRockId);
					bpaC15SatDE = m_model.sourceRockManager().getC15SatDiffusionEnergy(sourceRockId);
					legacyVREoptimization = m_model.sourceRockManager().getVREoptimization(sourceRockId);
					VREthreshold = m_model.sourceRockManager().getVREthreshold(sourceRockId);
					legacyVESlimitIndicator = m_model.sourceRockManager().getVESlimitIndicator(sourceRockId);
					VESlimit = m_model.sourceRockManager().getVESlimit(sourceRockId);
					


					modelConverter.upgradeSourceRockName(sourceRockTypeName[0], baseSourceRockTypeName, bpaScVre05, bpa2SourceRockType, bpa2BaseSourceRockType);
					modelConverter.upgradeDiffusionEnergy(baseSourceRockTypeName, bpaScVre05, bpaAsphalteneDE, bpaResinDE, bpaC15AroDE, bpaC15SatDE);
					modelConverter.upgradeVREthreshold(legacyVREoptimization, VREthreshold);
					modelConverter.upgradeVESlimit(legacyVESlimitIndicator, VESlimit);
					

					m_model.sourceRockManager().setSourceRockType(sourceRockId, bpa2SourceRockType);
					m_model.sourceRockManager().setBaseSourceRockType(sourceRockId, bpa2BaseSourceRockType);
					m_model.sourceRockManager().setAsphalteneDiffusionEnergy(sourceRockId, bpaAsphalteneDE);
					m_model.sourceRockManager().setResinDiffusionEnergy(sourceRockId, bpaResinDE);
					m_model.sourceRockManager().setC15AroDiffusionEnergy(sourceRockId, bpaC15AroDE);
					m_model.sourceRockManager().setC15SatDiffusionEnergy(sourceRockId, bpaC15SatDE);
					m_model.sourceRockManager().setVREthreshold(sourceRockId, VREthreshold);
					m_model.sourceRockManager().setVESlimit(sourceRockId, VESlimit);
					
					bpa2SourceRockTypeName.resize(0);
					bpa2SourceRockTypeName.emplace_back(bpa2SourceRockType);
					m_model.stratigraphyManager().setSourceRockTypeName(id, bpa2SourceRockTypeName);

					double legacyHiIni = m_model.sourceRockManager().hiIni(sourceRockId);
					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "BPA Cauldron HiIni value is identified as '" << legacyHiIni << "'";
				}				

			}
			
		}
	}
}

	