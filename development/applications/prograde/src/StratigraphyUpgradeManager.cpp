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

using namespace mbapi;

//------------------------------------------------------------//

Prograde::StratigraphyUpgradeManager::StratigraphyUpgradeManager(Model& model) :
	IUpgradeManager("Stratigraphy upgrade manager"), m_model(model)
{
	const auto ph = m_model.projectHandle();
	if (ph == nullptr) {
		throw std::invalid_argument(getName() + " cannot retrieve the project handle from Cauldron data model");
	}
	m_ph = ph;
}

//------------------------------------------------------------//

void Prograde::StratigraphyUpgradeManager::upgrade()
{
	Prograde::StratigraphyModelConverter modelConverter;

	std::string name;
	std::string updated_name;
	std::string ptCouplingMode;
	double depoAge = 0;
	// Variable containing the number of surfaces crossing the DepoAge limits
	unsigned countDepoAgeCrossingLimits = 0;
	int	chemCompaction_RunOptionsIoTbl = 1;

	// Fetching the chemical compaction flag from RunOptionsIoTbl and if Hydrostatic mode is detected in PTCouplingMode, check the ChemicalCompaction should be 0.
	database::Table * runoptionsio_tbl = m_ph->getTable("RunOptionsIoTbl");
	database::Record * rec = runoptionsio_tbl->getRecord(0);
	chemCompaction_RunOptionsIoTbl = rec->getValue<int>("ChemicalCompaction");
	ptCouplingMode = rec->getValue<std::string>("PTCouplingMode");
	int	upgradedChemCompaction_RunOptionsIoTbl = modelConverter.checkChemicalCompaction(ptCouplingMode, chemCompaction_RunOptionsIoTbl);
	rec->setValue<int>("ChemicalCompaction", upgradedChemCompaction_RunOptionsIoTbl);//chemical compaction for Hydrostatic mode should be 0.

	// Updating the LayerName, SurfaceName and Fluidtype in StratIoTbl that is by removing the special characters (except underscore) from the names
	database::Table * stratIo_table = m_ph->getTable("StratIoTbl");
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Updating the SurfaceName, LayerName and Fluidtype in StratIoTbl by removing the special characters (except underscore) :";

	// Updating the LayerName, SurfaceName and Fluidtype in the StratioTBl
	for (size_t id = 0; id < stratIo_table->size(); ++id)
	{/// for loop on StratIoTbl starts here
		database::Record * rec = stratIo_table->getRecord(static_cast<int>(id));
		depoAge = rec->getValue<double>("DepoAge");

		name = rec->getValue<std::string>("SurfaceName");
		updated_name = modelConverter.upgradeName(name);
		if (name.compare(updated_name))
		{
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> At DepoAge : " << depoAge << " , SurfaceName : " << name << " is updated to " << updated_name;
			rec->setValue<std::string>("SurfaceName", updated_name);
		}

		name = rec->getValue<std::string>("LayerName");
		updated_name = modelConverter.upgradeName(name);
		if (name.compare(updated_name))
		{
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> At DepoAge : " << depoAge << " , LayerName : " << name << " is updated to " << updated_name;
			rec->setValue<std::string>("LayerName", updated_name);
		}

		name = rec->getValue<std::string>("Fluidtype");
		// Detecting user - defined fluids
		// Names to be updated on for the user-defined fluids
		if ((name != "Std. Water") && (name != "Std. Sea Water") && (name != "Std. Hyper Saline Water") && (name != "Std. Marine Water"))
		{
			updated_name = modelConverter.upgradeName(name);
			if (name.compare(updated_name))
			{
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> At DepoAge : " << depoAge << " , FluidType : " << name << " is updated to " << updated_name;
				rec->setValue<std::string>("Fluidtype", modelConverter.upgradeName(name));
			}
		}
	}/// for loop on StratIoTbl ends here

	 // Updating the LayerName, SurfaceName and Fluidtype in the respective tables
	 // CTCIoTbl
	database::Table * ctcIo_table = m_ph->getTable("CTCIoTbl");
	if (ctcIo_table->size() != 0)
	{
		for (size_t id = 0; id < ctcIo_table->size(); ++id)
		{
			database::Record * rec = ctcIo_table->getRecord(static_cast<int>(id));
			name = rec->getValue<std::string>("SurfaceName");
			updated_name = modelConverter.upgradeName(name);
			if (name.compare(updated_name))
			{
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Updating the SurfaceName in CTCIoTbl by replacing each special character with underscore :";
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> SurfaceName : " << name << " is updated to " << updated_name;
				rec->setValue<std::string>("SurfaceName", updated_name);
			}
		}/// loop ends here
	}
	// TwoWayTimeIoTbl
	database::Table * twowaytimeio_table = m_ph->getTable("TwoWayTimeIoTbl");
	if (twowaytimeio_table->size() != 0)
	{
		for (size_t id = 0; id < twowaytimeio_table->size(); ++id)
		{
			database::Record * rec = twowaytimeio_table->getRecord(static_cast<int>(id));
			name = rec->getValue<std::string>("SurfaceName");
			updated_name = modelConverter.upgradeName(name);
			if (name.compare(updated_name))
			{
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Updating the SurfaceName in TwoWayTimeIoTbl by replacing each special character with underscore :";
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> SurfaceName : " << name << " is updated to " << updated_name;
				rec->setValue<std::string>("SurfaceName", updated_name);
			}
		}/// loop ends here
	}
	// MobLayThicknessIoTbl
	database::Table * moblaythickio_table = m_ph->getTable("MobLayThicknIoTbl");
	if (moblaythickio_table->size() != 0)
	{
		for (size_t id = 0; id < moblaythickio_table->size(); ++id)
		{
			database::Record * rec = moblaythickio_table->getRecord(static_cast<int>(id));
			name = rec->getValue<std::string>("LayerName");
			updated_name = modelConverter.upgradeName(name);
			if (name.compare(updated_name))
			{
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Updating the LayerName in MobLayThicknIoTbl by replacing each special character with underscore :";
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> LayerName : " << name << " is updated to " << updated_name;
				rec->setValue<std::string>("LayerName", updated_name);
			}
		}/// loop ends here
	}
	// AllochthonLithoInteroIoTbl
	database::Table * allochthonlithointerpio_table = m_ph->getTable("AllochthonLithoInterpIoTbl");
	if (allochthonlithointerpio_table->size() != 0)
	{
		for (size_t id = 0; id < allochthonlithointerpio_table->size(); ++id)
		{
			database::Record * rec = allochthonlithointerpio_table->getRecord(static_cast<int>(id));
			name = rec->getValue<std::string>("LayerName");
			updated_name = modelConverter.upgradeName(name);
			if (name.compare(updated_name))
			{
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Updating the LayerName in AllochthonLithoInterpIoTbl by replacing each special character with underscore :";
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> LayerName : " << name << " is updated to " << updated_name;
				rec->setValue<std::string>("LayerName", updated_name);
			}
		}/// loop ends here
	}
	// AllochthonLithoDistribIoTbl
	database::Table * allochthonlithodistribio_table = m_ph->getTable("AllochthonLithoDistribIoTbl");
	if (allochthonlithodistribio_table->size() != 0)
	{
		for (size_t id = 0; id < allochthonlithodistribio_table->size(); ++id)
		{
			database::Record * rec = allochthonlithodistribio_table->getRecord(static_cast<int>(id));
			name = rec->getValue<std::string>("LayerName");
			updated_name = modelConverter.upgradeName(name);
			if (name.compare(updated_name))
			{
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Updating the LayerName in AllochthonLithoDistribIoTbl by replacing each special character with underscore :";
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> LayerName : " << name << " is updated to " << updated_name;
				rec->setValue<std::string>("LayerName", updated_name);
			}
		}/// loop ends here
	}
	// AllochthonLithoIoTbl
	database::Table * allochthonlithoio_table = m_ph->getTable("AllochthonLithoIoTbl");
	if (allochthonlithoio_table->size() != 0)
	{
		for (size_t id = 0; id < allochthonlithoio_table->size(); ++id)
		{
			database::Record * rec = allochthonlithoio_table->getRecord(static_cast<int>(id));
			name = rec->getValue<std::string>("LayerName");
			updated_name = modelConverter.upgradeName(name);
			if (name.compare(updated_name))
			{
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Updating the LayerName in AllochthonLithoIoTbl by replacing each special character with underscore :";
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> LayerName : " << name << " is updated to " << updated_name;
				rec->setValue<std::string>("LayerName", updated_name);
			}
		}/// loop ends here
	}
	// SourceRockLithoIoTbl
	database::Table * sourcerocklithoio_table = m_ph->getTable("SourceRockLithoIoTbl");
	if (sourcerocklithoio_table->size() != 0)
	{
		for (size_t id = 0; id < sourcerocklithoio_table->size(); ++id)
		{
			database::Record * rec = sourcerocklithoio_table->getRecord(static_cast<int>(id));
			name = rec->getValue<std::string>("LayerName");
			updated_name = modelConverter.upgradeName(name);
			if (name.compare(updated_name))
			{
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Updating the LayerName in SourceRockLithoIoTbl by replacing each special character with underscore :";
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> LayerName : " << name << " is updated to " << updated_name;
				rec->setValue<std::string>("LayerName", updated_name);
			}
		}/// loop ends here
	}
	// FluidtypeIoTbl
	database::Table * fluidtypeio_table = m_ph->getTable("FluidtypeIoTbl");
	if (fluidtypeio_table->size() != 0)
	{
		for (size_t id = 0; id < fluidtypeio_table->size(); ++id)
		{
			database::Record * rec = fluidtypeio_table->getRecord(static_cast<int>(id));
			name = rec->getValue<std::string>("Fluidtype");
			if ((name != "Std. Water") && (name != "Std. Sea Water") && (name != "Std. Hyper Saline Water") && (name != "Std. Marine Water"))
			{ //detecting user-defined fluid
				updated_name = modelConverter.upgradeName(name);
				if (name.compare(updated_name))
				{
					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Updating the Fluidtype in FluidtypeIoTbl by replacing each special character with underscore :";
					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> Fluidtype : " << name << " is updated to " << updated_name;
					rec->setValue<std::string>("Fluidtype", updated_name);
				}
			}
		}/// loop ends here
	}

	// ReservoirIoTbl - FormationName
	database::Table* ResIo_table = m_ph->getTable("ReservoirIoTbl");
	if (ResIo_table->size() != 0)
	{
		for (size_t id = 0; id < ResIo_table->size(); ++id)
		{
			database::Record* rec = ResIo_table->getRecord(static_cast<int>(id));
			name = rec->getValue<std::string>("FormationName");
			updated_name = modelConverter.upgradeName(name);
			if (name.compare(updated_name))
			{
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Updating the FormationName in ReservoirIoTbl by replacing each special character with underscore :";
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> FormationName : " << name << " is updated to " << updated_name;
				rec->setValue<std::string>("FormationName", updated_name);
			}
		}/// loop ends here
	}

	// Updating the StratioTbl for Surfaces for Depth & Thickness limits, LayeringIndex, ChemicalCompaction and DepoAge
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Updating the StratIoTbl for Depth, Thickness, LayeringIndex, ChemicalCompaction and DepoAge :";
	database::Table * stratIo_Table = m_ph->getTable("StratIoTbl");
	for (size_t id = 0; id < stratIo_Table->size(); ++id)
	{/// for loop on StratIoTbl begins here
		database::Record * rec = stratIo_Table->getRecord(static_cast<int>(id));
		name = rec->getValue<std::string>("SurfaceName");
		double depth = rec->getValue<double>("Depth");
		double thickness = rec->getValue<double>("Thickness");

		// Checking if depth and thickness scalar values to be in range if not then trim to the range boundaries
		double updatedDepth = modelConverter.upgradeDepthThickness(depth);
		double updatedThickness = modelConverter.upgradeDepthThickness(thickness);
		// Log the information if the values are trimmed
		if (updatedDepth != depth)
		{
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Depth value for the surface " << name << " is changed from " << depth << " to : " << updatedDepth << " as it is crossing the limits [-6380000,6380000]";
			rec->setValue<double>("Depth", updatedDepth);
		}
		if (updatedThickness != thickness)
		{
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Thickness value for the surface " << name << " is changed from " << thickness << " to : " << updatedThickness << " as it is crossing the limits [-6380000,6380000]";
			rec->setValue<double>("Thickness", updatedThickness);
		}

		// Updating the Layering Index of each layer
		// As per BPA2 standards, if MixModel is Homogeneous then the Layering Index should be -9999
		std::string mixModel = rec->getValue<std::string>("MixModel");
		double layeringIndex = rec->getValue<double>("LayeringIndex");
		double updatedLayeringIndex = modelConverter.upgradeLayeringIndex(mixModel, layeringIndex);
		// Log the information if the LayeringIndex is updated
		if (updatedLayeringIndex != layeringIndex)
		{
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> MixModel detected for the surface " << name << " is : " << mixModel << ". Hence, the LayeringIndex of the surface is changed from " << layeringIndex << " to " << updatedLayeringIndex << " as per BPA2 standards";
			rec->setValue<double>("LayeringIndex", updatedLayeringIndex);
		}

		// Chemical compaction cannot be applied to the basement
		if (id < (stratIo_Table->size() - 1))
		{
			int chemCompaction_layer = rec->getValue<int>("ChemicalCompaction");
			int updatedChemCompaction_layer = modelConverter.upgradeChemicalCompaction(upgradedChemCompaction_RunOptionsIoTbl, chemCompaction_layer);
			if (updatedChemCompaction_layer != chemCompaction_layer)
			{
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> ChemicalCompaction detected for the surface " << name << "  is : " << chemCompaction_layer << " ! Hence, changed to " << updatedChemCompaction_layer << " as in the RunOptionsIoTbl";
				rec->setValue<int>("ChemicalCompaction", updatedChemCompaction_layer);
			}
		}
		else
		{
			int chemCompaction_layer = rec->getValue<int>("ChemicalCompaction");
			// Chemical compaction is set to NULL at basement
			if (chemCompaction_layer != -9999)
			{
				rec->setValue<int>("ChemicalCompaction", -9999);
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> ChemicalCompaction for the surface (basement) " << name << " is changed from : " << chemCompaction_layer << " to -9999 (NULL)";
			}
		}

		depoAge = rec->getValue<double>("DepoAge");
		// Generating logs for the surfaces which have depoAge greater than 999
		if (depoAge > 999)
		{
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> SurfaceName : " << name << " with Deposition age : " << depoAge << " Ma is crossing the upper limit of 999 Ma";
			countDepoAgeCrossingLimits++;
		}
	}/// for loop on StratIoTbl ends here

	 // Failing the scenario for prograde if any of the surfaces exceed the age limit (0-999)
	if (countDepoAgeCrossingLimits)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Error> Surface with Deposition age greater than 999 Ma is detected; Migration from BPA to BPA2 Basin Aborted...";
		exit(21);
	}
}/// update function ends here
