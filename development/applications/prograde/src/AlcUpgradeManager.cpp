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
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP ) << "legacy ALC detected (v2016.11)";
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS ) << "upgrading project to latest ALC";
      createCrustThickness();
      computeBasaltThickness();
      writeOceaCrustalThicknessIoTbl();
      cleanBasaltThicknessIoTbl();

      Prograde::AlcModelConverter modelConverter;

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
   else
   {
	   if (m_ph->isFixedTempBasement()) LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Basic Crust Thinning mode detected";
	   else if (m_ph->isFixedHeatFlow())  LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Fixed Heat Flow mode detected";
	   else  LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Bottom Boundary Condition : Unknown";
	   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "no ALC upgrade needed";
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

//------------------------------------------------------------//

void Prograde::AlcUpgradeManager::writeOceaCrustalThicknessIoTbl() {
   std::size_t rowNumber = 0;
   std::for_each(m_basaltThicknessHistory.begin(), m_basaltThicknessHistory.end(), [this, &rowNumber] (const SmartAbstractSnapshotSmartGridMapPair& pair)
   {
      const auto age = pair.first->getTime();
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
   } );
}

//------------------------------------------------------------//

void Prograde::AlcUpgradeManager::cleanBasaltThicknessIoTbl() const {
   m_model.clearTable("BasaltThicknessIoTbl");
}


