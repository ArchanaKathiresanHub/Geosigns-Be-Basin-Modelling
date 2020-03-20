//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "StratigraphyUpgradeManager.h"
#include "StratigraphyModelConverter.h"

//utilities
#include "LogHandler.h"

//cmbAPI
#include "cmbAPI.h"
#include "StratigraphyManager.h"

#include <algorithm>

using namespace mbapi;

//------------------------------------------------------------//

Prograde::StratigraphyUpgradeManager::StratigraphyUpgradeManager(Model& model) :
	IUpgradeManager("stratigraphy upgrade manager"), m_model(model)
{
	const auto ph = m_model.projectHandle();
	if (ph == nullptr) {
		throw std::invalid_argument(getName() + " cannot retreive the project handle from Cauldron data model");
	}
	m_ph = ph;
}

//------------------------------------------------------------//

void Prograde::StratigraphyUpgradeManager::upgrade() {
	Prograde::StratigraphyModelConverter modelConverter;
	
	std::string name;
	std::string updated_name;
	std::string ptCouplingMode;
	int	chemCompaction_RunOptionsIoTbl = 1;
	//fetching the chemical compaction flag from RunOptionsIoTbl and if Hydrostatic mode is detected in PTCouplingMode, check the ChemicalCompaction should be 0.
	database::Table * runoptionsio_tbl	= m_ph->getTable("RunOptionsIoTbl");
	for (size_t id = 0; id < runoptionsio_tbl->size(); ++id) {
		database::Record * rec = runoptionsio_tbl->getRecord(static_cast<int>(id));
		chemCompaction_RunOptionsIoTbl = rec->getValue<int>("ChemicalCompaction");
		ptCouplingMode = rec->getValue<std::string>("PTCouplingMode");
		int	upgradedChemCompaction_RunOptionsIoTbl = modelConverter.checkChemicalCompaction(ptCouplingMode, chemCompaction_RunOptionsIoTbl);
		rec->setValue<int>("ChemicalCompaction", upgradedChemCompaction_RunOptionsIoTbl);//chemical compaction for Hydrostatic mode should be 0.
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "ChemicalCompaction in RunOptionsIoTbl is : " << upgradedChemCompaction_RunOptionsIoTbl;
	}
	


	//updating the LayerName, SurfaceName and Fluidtype in StratIoTbl that is removing the special characters from the names
	database::Table * stratIo_table = m_ph->getTable("StratIoTbl");
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the SurfaceName, LayerName and Fluidtype in StratIoTbl :";

	for (size_t id = 0; id < stratIo_table->size(); ++id) {
		database::Record * rec = stratIo_table->getRecord(static_cast<int>(id));

		name = rec->getValue<std::string>("SurfaceName");
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "SurfaceName detected :" << name;
		rec->setValue<std::string>("SurfaceName", modelConverter.upgradeName(name));

		name = rec->getValue<std::string>("LayerName");
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "LayerName detected :" << name;
		rec->setValue<std::string>("LayerName", modelConverter.upgradeName(name));
		
		name = rec->getValue<std::string>("Fluidtype");
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Fluidtype detected :" << name;
		if ((name != "Std. Water") && (name != "Std. Sea Water") && (name != "Std. Hyper Saline Water")) { //detecting user-defined fluid
			rec->setValue<std::string>("Fluidtype", modelConverter.upgradeName(name));
		}
	}

	//updating the LayerName, SurfaceName and Fluidtype in the respective tables
	// CTCIoTbl
	database::Table * ctcIo_table = m_ph->getTable("CTCIoTbl");
	if (ctcIo_table->size() != 0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the SurfaceName in CTCIoTbl :";
		for (size_t id = 0; id < ctcIo_table->size(); ++id) {
			database::Record * rec = ctcIo_table->getRecord(static_cast<int>(id));
			name = rec->getValue<std::string>("SurfaceName");
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "SurfaceName detected :" << name;
			rec->setValue<std::string>("SurfaceName", modelConverter.upgradeName(name));
		}
	}
	//PalinspasticIoTbl
	database::Table * palinspasticio_table = m_ph->getTable("PalinspasticIoTbl");
	if (palinspasticio_table->size() != 0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the SurfaceName in PalinspasticIoTbl :";
		for (size_t id = 0; id < palinspasticio_table->size(); ++id) {
			database::Record * rec = palinspasticio_table->getRecord(static_cast<int>(id));
			name = rec->getValue<std::string>("SurfaceName");
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "SurfaceName detected :" << name;
			rec->setValue<std::string>("SurfaceName", modelConverter.upgradeName(name));
		}
	}
	//TwoWayTimeIoTbl
	database::Table * twowaytimeio_table = m_ph->getTable("TwoWayTimeIoTbl");
	if (twowaytimeio_table->size() != 0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the SurfaceName in TwoWayTimeIoTbl :";
		for (size_t id = 0; id < twowaytimeio_table->size(); ++id) {
			database::Record * rec = twowaytimeio_table->getRecord(static_cast<int>(id));
			name = rec->getValue<std::string>("SurfaceName");
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "SurfaceName detected :" << name;
			rec->setValue<std::string>("SurfaceName", modelConverter.upgradeName(name));
		}
	}
	//TouchstoneMapIoTbl
	database::Table * touchstonemapio_table = m_ph->getTable("TouchstoneMapIoTbl");
	if (touchstonemapio_table->size() != 0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the SurfaceName in TouchstoneMapIoTbl :";
		for (size_t id = 0; id < touchstonemapio_table->size(); ++id) {
			database::Record * rec = touchstonemapio_table->getRecord(static_cast<int>(id));
			name = rec->getValue<std::string>("SurfaceName");
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "SurfaceName detected :" << name;
			rec->setValue<std::string>("SurfaceName", modelConverter.upgradeName(name));
		}
	}
	//MobLayThicknessIoTbl
	database::Table * moblaythickio_table = m_ph->getTable("MobLayThicknIoTbl");
	if (moblaythickio_table->size() != 0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the LayerName in MobLayThicknIoTbl :";
		for (size_t id = 0; id < moblaythickio_table->size(); ++id) {
			database::Record * rec = moblaythickio_table->getRecord(static_cast<int>(id));
			name = rec->getValue<std::string>("LayerName");
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "LayerName detected :" << name;
			rec->setValue<std::string>("LayerName", modelConverter.upgradeName(name));
		}
	}
	//AllochthonLithoInteroIoTbl
	database::Table * allochthonlithointerpio_table = m_ph->getTable("AllochthonLithoInterpIoTbl");
	if (allochthonlithointerpio_table->size() != 0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the LayerName in AllochthonLithoInterpIoTbl :";
		for (size_t id = 0; id < allochthonlithointerpio_table->size(); ++id) {
			database::Record * rec = allochthonlithointerpio_table->getRecord(static_cast<int>(id));
			name = rec->getValue<std::string>("LayerName");
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "LayerName detected :" << name;
			rec->setValue<std::string>("LayerName", modelConverter.upgradeName(name));
		}
	}
	//AllochthonLithoDistribIoTbl
	database::Table * allochthonlithodistribio_table = m_ph->getTable("AllochthonLithoDistribIoTbl");
	if (allochthonlithodistribio_table->size() != 0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the LayerName in AllochthonLithoDistribIoTbl :";
		for (size_t id = 0; id < allochthonlithodistribio_table->size(); ++id) {
			database::Record * rec = allochthonlithodistribio_table->getRecord(static_cast<int>(id));
			name = rec->getValue<std::string>("LayerName");
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "LayerName detected :" << name;
			rec->setValue<std::string>("LayerName", modelConverter.upgradeName(name));
		}
	}
	//AllochthonLithoIoTbl
	database::Table * allochthonlithoio_table = m_ph->getTable("AllochthonLithoIoTbl");
	if (allochthonlithoio_table->size() != 0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the LayerName in AllochthonLithoIoTbl :";
		for (size_t id = 0; id < allochthonlithoio_table->size(); ++id) {
			database::Record * rec = allochthonlithoio_table->getRecord(static_cast<int>(id));
			name = rec->getValue<std::string>("LayerName");
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "LayerName detected :" << name;
			rec->setValue<std::string>("LayerName", modelConverter.upgradeName(name));
		}
	}
	//SourceRockLithoIoTbl
	database::Table * sourcerocklithoio_table = m_ph->getTable("SourceRockLithoIoTbl");
	if (sourcerocklithoio_table->size() != 0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the LayerName in SourceRockLithoIoTbl :";
		for (size_t id = 0; id < sourcerocklithoio_table->size(); ++id) {
			database::Record * rec = sourcerocklithoio_table->getRecord(static_cast<int>(id));
			name = rec->getValue<std::string>("LayerName");
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "LayerName detected :" << name;
			rec->setValue<std::string>("LayerName", modelConverter.upgradeName(name));
		}
	}
	//FluidtypeIoTbl
	database::Table * fluidtypeio_table = m_ph->getTable("FluidtypeIoTbl");
	if (fluidtypeio_table->size() != 0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the Fluidtype in FluidtypeIoTbl :";
		for (size_t id = 0; id < fluidtypeio_table->size(); ++id) {
			database::Record * rec = fluidtypeio_table->getRecord(static_cast<int>(id));
			name = rec->getValue<std::string>("Fluidtype");
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Fluidtype detected :" << name;
			if ((name != "Std. Water") && (name != "Std. Sea Water") && (name != "Std. Hyper Saline Water")) { //detecting user-defined fluid
				rec->setValue<std::string>("Fluidtype", modelConverter.upgradeName(name));
			}
		}
	}

	//Updating the StratioTbl for Surfaces for Depth & Thickness limits, LayeringIndex, ChemicalCompaction and DepoAge
	database::Table * stratIo_Table = m_ph->getTable("StratIoTbl");

	unsigned num = 0;
	for (size_t id = 0; id < stratIo_Table->size(); ++id) {
		database::Record * rec = stratIo_Table->getRecord(static_cast<int>(id));
		name = rec->getValue<std::string>("SurfaceName");
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the Surface " << name << " in StratIoTbl for Depth & Thickness, LayeringIndex, ChemicalCompaction and DepoAge :";
	
		double depth = rec->getValue<double>("Depth");
		double thickness = rec->getValue<double>("Thickness");
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Depth value for the surface " << name << " is : " << depth;
		rec->setValue<double>("Depth", modelConverter.upgradeDepthThickness(depth));
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Thickness value for the surface " << name << " is : " << thickness;
		rec->setValue<double>("Thickness", modelConverter.upgradeDepthThickness(thickness));

		std::string mixModel = rec->getValue<std::string>("MixModel"); 
		double layeringIndex = rec->getValue<double>("LayeringIndex");
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "MixModel for the surface " << name << " is : " << mixModel;
		rec->setValue<double>("LayeringIndex", modelConverter.upgradeLayeringIndex(mixModel, layeringIndex));
		
		if (id < (stratIo_Table->size() - 1)){ //Chemical compaction cannot be applied to the basement
			int chemCompaction_layer = rec->getValue<int>("ChemicalCompaction");
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "ChemicalCompaction for the surface " << name << " is : " << chemCompaction_layer;
			rec->setValue<int>("ChemicalCompaction", modelConverter.upgradeChemicalCompaction(chemCompaction_RunOptionsIoTbl, chemCompaction_layer));
		}
		else {
			int chemCompaction_layer = rec->getValue<int>("ChemicalCompaction");
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "ChemicalCompaction for the surface " << name << " is : " << chemCompaction_layer;
			if (chemCompaction_layer != -9999) {
				rec->setValue<int>("ChemicalCompaction", -9999); //chemical compaction set to NULL at basement
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "ChemicalCompaction for the surface " << name << " is set to NUll (-9999)";
			}
		}
		
	
		double age = rec->getValue<double>("DepoAge");
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Deposition age for the surface "<<name<<" is : "<<age;

		if (age >= 999) {
			if (age > 999) {
				rec->setValue<double>("DepoAge", 999);
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Deposition age exceeds the limit (0-999). So, the value is changed to 999";
			}
			num++;
		}
	}

	//failing the scenario if more than one surfaces exceed the age limit (0-999)
	if (num > 1) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "More than one surfaces are have DepoAge >=999 (maximum limit); Scenario Rejected";
		exit(200);
	}
}