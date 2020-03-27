//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "AlcUpgradeManager.h"
#define MERGEOceaToInputsHDF 1 // 1=YES, 0=No
#define MERGECrustToInputsHDF 1 //1=YES 0=NO
//utilities
#include "LogHandler.h"

//DataAccess
#include "GridMap.h"
#include "CrustFormation.h"
#include "MantleFormation.h"
#include "ProjectHandle.h"
#include "ContinentalCrustHistoryGenerator.h"
#include "PaleoFormationProperty.h"
#include "Snapshot.h"

//Prograde
#include "LegacyBasaltThicknessCalculator.h"
#include "AlcModelConverter.h"

//cmbAPI
#include "MapsManager.h"
#include "cmbAPI.h"
#include "BottomBoundaryManager.h"

#include <algorithm>

using namespace mbapi;

typedef std::pair<const std::shared_ptr<DataModel::AbstractSnapshot const>,
                  const std::shared_ptr<DataAccess::Interface::GridMap const>> SmartAbstractSnapshotSmartGridMapPair;

typedef std::map<const DataModel::AbstractSnapshot* const,
                 const DataAccess::Interface::GridMap* const,
                 DataModel::AbstractSnapshot::ComparePointers<const DataModel::AbstractSnapshot* const>> AbstractSnapshotVsGridMap;


Prograde::AlcUpgradeManager::AlcUpgradeManager( mbapi::Model& model ):
   IUpgradeManager( "ALC upgrade manager" ), m_model( model ), m_initialCrustalThickness( 0 )
{
   const auto ph = m_model.projectHandle();
   if (ph == nullptr) {
      throw std::invalid_argument( getName() + " cannot retreive the project handle from Cauldron data model" );
   }
   const auto crust = ph->getCrustFormation();
   if (crust == nullptr) {
      throw std::invalid_argument( getName() + " cannot retreive the crust from project handle" );
   }
   m_ph = ph ;
   m_crust = crust;
}

//------------------------------------------------------------//

void Prograde::AlcUpgradeManager::upgrade() {
   if (isLegacyAlc()) {
	  Prograde::AlcModelConverter modelConverter;

	  // Check the DepoAge of basement layer and set it to 999 if it crosses the upper limit of 999; basement age will be used to clip the rows till basement age
	  database::Table * stratIo_Table = m_ph->getTable("StratIoTbl");
	  size_t id = stratIo_Table->size() - 1;
	  database::Record * rec = stratIo_Table->getRecord(static_cast<int>(id));
	  double basementAge = rec->getValue<double>("DepoAge");
	  if (basementAge > 999) {
		   basementAge = 999; 
		   rec->setValue<double>("DepoAge", 999);
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Basement DepoAge crossed the upper limit (0-999); DepoAge is set to 999";
	  }
	   
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP ) << "legacy ALC detected (v2016.11)";
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS ) << "upgrading project to latest ALC";

	  //updating BasementIoTbl
	  LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating BasementIoTbl";
	  database::Table * BasementIo_Table = m_ph->getTable("BasementIoTbl");
	  for (size_t id = 0; id < BasementIo_Table->size(); ++id) {
		  database::Record * rec = BasementIo_Table->getRecord(static_cast<int>(id));

		  std::string BottomBoundaryModel = rec->getValue<std::string>("BottomBoundaryModel");
		  rec->setValue<std::string>("BottomBoundaryModel", modelConverter.updateBottomBoundaryModel(BottomBoundaryModel));

		  double TopAsthenoTemp = rec->getValue<double>("TopAsthenoTemp");
		  if (TopAsthenoTemp != 1333) {
			  LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "The value of TopAsthenoTemp is : "<<TopAsthenoTemp;
		  }

		  std::string TopCrustHeatProdGrid = rec->getValue<std::string>("TopCrustHeatProdGrid");
		  if (!TopCrustHeatProdGrid.compare("")) {
			  double TopCrustHeatProd = rec->getValue<double>("TopCrustHeatProd");
			  rec->setValue<double>("TopCrustHeatProd", modelConverter.updateTopCrustHeatProd(TopCrustHeatProd));
		  }
		  /*
		  else{
		  
		  //conditions for maps - TopCrustHeatProdGrid range [0,1000]
		  
		  }
		  */
		  double LithoMantleThickness = rec->getValue<double>("LithoMantleThickness");
		  double InitialLthMntThickns = rec->getValue<double>("InitialLthMntThickns");
		  double FixedCrustThickness = rec->getValue<double>("FixedCrustThickness");
		  rec->setValue<double>("LithoMantleThickness", 0);
		  LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "LithoMantleThickness is changed from " << LithoMantleThickness << " to 0";
		  rec->setValue<double>("InitialLthMntThickns", 0);
		  LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "InitialLthMntThickns is changed from " << InitialLthMntThickns << " to 0";
		  rec->setValue<double>("FixedCrustThickness", 0);
		  LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "FixedCrustThickness is changed from " << FixedCrustThickness << " to 0";

		  double InitialLithosphericMantleThickness = rec->getValue<double>("InitialLithosphericMantleThickness");
		  rec->setValue<double>("InitialLithosphericMantleThickness", modelConverter.updateInitialLithosphericMantleThickness(InitialLithosphericMantleThickness));
	  }
      createCrustThickness();
      computeBasaltThickness();
	  writeContCrustalThicknessIoTbl(basementAge);
      writeOceaCrustalThicknessIoTbl(basementAge); //basement age is passed as argument to limit the OceaCrustalThicknessIoTbl till the basement age
	  m_model.clearTable("BasaltThicknessIoTbl");
	  m_model.clearTable("CrustIoTbl");
	  m_model.clearTable("MntlHeatFlowIoTbl");

	  //updating GridMapIoTbl for deleting the references of the maps in the tables cleared
	  std::string GridMapReferredBy;
	  auto GridMapId = m_model.ctcManager().getGridMapID();
	  for (int tsId = 0; tsId < GridMapId.size(); tsId++) {
		  m_model.ctcManager().getGridMapTablename(tsId, GridMapReferredBy);
		  if (GridMapReferredBy == "BasaltThicknessIoTbl" || GridMapReferredBy == "CrustIoTbl" || GridMapReferredBy == "MntlHeatFlowIoTbl") {
			  m_model.removeRecordFromTable("GridMapIoTbl", tsId);
			  tsId--;
		  }
	  }

      //upgrading the crust property model to standard conductivity crust

      BottomBoundaryManager::CrustPropertyModel crstPropModel;
      m_model.bottomBoundaryManager().getCrustPropertyModel(crstPropModel);
      m_model.bottomBoundaryManager().setCrustPropertyModel(modelConverter.upgradeAlcCrustPropModel(crstPropModel));

      //upgrading the mantle property model to high conductivity mantle model
      BottomBoundaryManager::MantlePropertyModel mntlPropModel;
      m_model.bottomBoundaryManager().getMantlePropertyModel(mntlPropModel);
      m_model.bottomBoundaryManager().setMantlePropertyModel(modelConverter.upgradeAlcMantlePropModel(mntlPropModel));

      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS ) << "ALC upgrade done";
   }
   else {
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP ) << "production ALC detected (v2018.01+)";
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS ) << "no ALC upgrade needed";
   }
}

//------------------------------------------------------------//

bool Prograde::AlcUpgradeManager::isLegacyAlc() const {
   if (m_ph->isALC()) {
      const DataAccess::Interface::GridMap* presentDayBasaltThickness = m_crust->getBasaltThicknessMap();
      const DataAccess::Interface::GridMap* crustMeltOnsetMap = m_crust->getCrustThicknessMeltOnsetMap();
      return (presentDayBasaltThickness != nullptr or crustMeltOnsetMap != nullptr);
   }
   else{
      return false;
   }
}

//------------------------------------------------------------//

void Prograde::AlcUpgradeManager::createCrustThickness() {
   m_ph->setActivityOutputGrid( m_ph->getInputGrid() );
   m_ph->loadCrustThinningHistory();
   m_ph->addCrustThinningHistoryMaps();
   m_ph->correctCrustThicknessHistory();
   m_ph->initialiseValidNodes(false);
   DataAccess::Interface::ContinentalCrustHistoryGenerator generator(*m_ph->getActivityOutputGrid(), *m_crust, m_ph->getValidator(), m_ph->getGlobalOperations() );
   generator.createCrustThickness(DataAccess::Interface::IMPROVED_LITHOSPHERE_CALCULATOR_LINEAR_ELEMENT_MODE);
   m_initialCrustalThickness = generator.getInitialCrustalThickness();
   m_crustThicknessHistory = generator.getcrustThicknessHistory();
}

//------------------------------------------------------------//

void Prograde::AlcUpgradeManager::computeBasaltThickness() {

   const DataAccess::Interface::PaleoFormationPropertyList* paleoFormations  = m_crust->getPaleoThicknessHistory();
   const DataAccess::Interface::GridMap* presentDayBasaltThickness           = m_crust->getBasaltThicknessMap();
   const DataAccess::Interface::GridMap* crustMeltOnsetMap                   = m_crust->getCrustThicknessMeltOnsetMap();
   const DataAccess::Interface::Grid* referenceGrid                          = m_ph->getInputGrid();
   const DataAccess::Interface::ObjectFactory* factory                       = m_ph->getFactory();

   AbstractSnapshotVsGridMap crustThicknesses;
   std::for_each(paleoFormations->begin(), paleoFormations->end(), [&crustThicknesses] (const DataAccess::Interface::PaleoFormationProperty *const it)
   {
      const DataAccess::Interface::GridMap* const gridMap = it->getMap( DataAccess::Interface::CrustThinningHistoryInstanceThicknessMap );
      const DataModel::AbstractSnapshot* const snapshot = it->getSnapshot();
      crustThicknesses.insert( std::pair<const DataModel::AbstractSnapshot* const, const DataAccess::Interface::GridMap* const>(snapshot, gridMap) );
   });

   try {
      LegacyBasaltThicknessCalculator legacyBasaltThicknessCalculator( crustThicknesses,
                                                                       m_crustThicknessHistory,
                                                                       presentDayBasaltThickness,
                                                                       crustMeltOnsetMap,
                                                                       m_initialCrustalThickness,
                                                                       m_ph->getValidator(),
                                                                       referenceGrid,
                                                                       factory );
      legacyBasaltThicknessCalculator.compute(m_basaltThicknessHistory);
   }
   catch ( std::invalid_argument& ex ) {
      LogHandler( LogHandler::ERROR_SEVERITY ) << "One of the Legacy Advanced Lithosphere Calculator (ALC) input is invalid (see details below)";
      LogHandler( LogHandler::ERROR_SEVERITY ) << ex.what();
   }
   catch( std::runtime_error& ex ) {
      LogHandler( LogHandler::ERROR_SEVERITY ) << "The Legacy Advanced Lithosphere Calculator (ALC) could not compute the Effective Crustal Thickness (see details below)";
      LogHandler( LogHandler::ERROR_SEVERITY ) << ex.what();
   }
   catch (...) {
      LogHandler( LogHandler::ERROR_SEVERITY ) << "The Legacy Advanced Lithosphere Calculator encounterred a fatal error (unkown details)";
   }

   //clean memory
   delete paleoFormations;
}




/////////////////////////////////////////////////////////////////////////////////////////////////





//--------------------------------------------------------------//
void Prograde::AlcUpgradeManager::writeContCrustalThicknessIoTbl(double basement_age) {
	/*/
	const DataAccess::Interface::PaleoFormationPropertyList* paleoFormations = m_crust->getPaleoThicknessHistory();
	SmartAbstractSnapshotVsSmartGridMap crustThicknesses;
	std::for_each(paleoFormations->begin(), paleoFormations->end(), [&crustThicknesses](const DataAccess::Interface::PaleoFormationProperty *const it)
	{
		const DataAccess::Interface::GridMap* const gridMap = it->getMap(DataAccess::Interface::CrustThinningHistoryInstanceThicknessMap);
		const DataModel::AbstractSnapshot* const snapshot = it->getSnapshot();
		crustThicknesses.insert(std::pair<const DataModel::AbstractSnapshot* const, const DataAccess::Interface::GridMap* const>(snapshot, gridMap));
	});
	*/

	database::Table * contCrustalThicknessIo_Table = m_ph->getTable("ContCrustalThicknessIoTbl");
	for (size_t id = 0; id < contCrustalThicknessIo_Table->size(); ++id) {
		database::Record * rec = contCrustalThicknessIo_Table->getRecord(static_cast<int>(id));
		double age = rec->getValue<double>("Age");
		if (age > basement_age) {
			m_model.removeRecordFromTable("ContCrustalThicknessIoTbl", id);
			--id;
		}
	}
	/*
	std::size_t rowNumber = contCrustalThicknessIo_Table->size();
	std::for_each(crustThicknesses.begin(), crustThicknesses.end(), [this, &rowNumber, basement_age](const SmartAbstractSnapshotSmartGridMapPair& pair)
	{
			const auto age = pair.first->getTime();
			if (age > basement_age) return; // return from the function when age reaches basement age
			if (age == basement_age) {
				const auto gridMap = const_cast<DataAccess::Interface::GridMap*>(pair.second.get());

#if MERGECrustToInputsHDF
				const auto outputFileName = "Inputs.HDF";
#else
				const auto outputFileName = "Input.HDF";
#endif      
				const auto mapName = "ContCrustThicknessInterpolated" + std::to_string(age);
				const auto refferedTable = "ContCrustalThicknessIoTbl";
				const auto ageField = "Age";
				const auto thicknessGridField = "ThicknessGrid";

#if MERGECrustToInputsHDF
				size_t mapsSequenceNbr = Utilities::Numerical::NoDataIDValue;
#else
				size_t mapsSequenceNbr = 0;
#endif
				
				m_model.mapsManager().generateMap(refferedTable, mapName, gridMap, mapsSequenceNbr, outputFileName);
				
				m_model.addRowToTable(refferedTable);
				m_model.setTableValue(refferedTable, rowNumber, ageField, age);
				m_model.setTableValue(refferedTable, rowNumber, thicknessGridField, mapName);
				
			}
	});
	*/
	
}






//------------------------------------------------------------//

void Prograde::AlcUpgradeManager::writeOceaCrustalThicknessIoTbl(double basement_age) {
	
	// In OceaCrustalThicknessIoTbl, times with only system generated snapshots are required. 
	database::Table * snapshotIo_Table = m_ph->getTable("SnapshotIoTbl");
	std::vector<double>timeWithSystemSnapshots;
	for (size_t id = 0; id < snapshotIo_Table->size(); ++id) {
		std::string snapshotType;
		double time = 0;
		database::Record * rec = snapshotIo_Table->getRecord(static_cast<int>(id));
		snapshotType = rec->getValue<std::string>("TypeOfSnapshot");
		time = rec->getValue<double>("Time");
		if (!snapshotType.compare("System Generated")) {
			timeWithSystemSnapshots.push_back(time); //collecting the times of system generated snapshots in a vector
		}
	}
   std::size_t rowNumber = 0;
   std::for_each(m_basaltThicknessHistory.begin(), m_basaltThicknessHistory.end(), [this, &rowNumber, basement_age, timeWithSystemSnapshots] (const SmartAbstractSnapshotSmartGridMapPair& pair)
   {
      const auto age = pair.first->getTime();

	  for (int i = 0; i < timeWithSystemSnapshots.size(); i++) {
		  if (age == timeWithSystemSnapshots[i]) {
			  if (age > basement_age) return; // return from the function when age reaches basement age
			  const auto gridMap = const_cast<DataAccess::Interface::GridMap*>(pair.second.get());

#if MERGEOceaToInputsHDF
			  const auto outputFileName = "Inputs.HDF";
#else
			  const auto outputFileName = "Input.HDF";
#endif      
			  const auto mapName = "OceanicCrustThicknessFromLegacyALC_" + std::to_string(age);
			  const auto refferedTable = "OceaCrustalThicknessIoTbl";
			  const auto ageField = "Age";
			  const auto thicknessGridField = "ThicknessGrid";

#if MERGEOceaToInputsHDF
			  size_t mapsSequenceNbr = Utilities::Numerical::NoDataIDValue;
#else
			  size_t mapsSequenceNbr = 0;
#endif
			  m_model.mapsManager().generateMap(refferedTable, mapName, gridMap, mapsSequenceNbr, outputFileName);
			  m_model.addRowToTable(refferedTable);
			  m_model.setTableValue(refferedTable, rowNumber, ageField, age);
			  m_model.setTableValue(refferedTable, rowNumber, thicknessGridField, mapName);
			  rowNumber++;
		  }
	  }
   } );
}
