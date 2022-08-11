//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ReservoirUpgradeManager.h"
#include "ReservoirConverter.h"
//Prograde class to update the GridMapIoTbl if any GridMap is removed from any table
#include "GridMapIoTblUpgradeManager.h"
/**Static function named 'Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap()' is defined for the operation
* Overload 1: Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap("tableName"); //clears all the map references ReferredBy the table "tableName" from GridMapIoTbl
* Overload 2: Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap("tableName","mapName"); //clears the map reference of the "mapName" ReferredBy "tableName" from GridMapIoTbl
*/

//utilities
#include "LogHandler.h"
#include "NumericFunctions.h"

//cmbAPI
#include "cmbAPI.h"
#include "ReservoirManager.h"
#include "StratigraphyManager.h"

//DataAccess
#include "ProjectHandle.h"
#include "RunParameters.h"

using namespace mbapi;

//------------------------------------------------------------//

Prograde::ReservoirUpgradeManager::ReservoirUpgradeManager(Model& model) :
   IUpgradeManager("Reservoir upgrade manager"), m_model(model)
{
   const auto ph = m_model.projectHandle();
   if (ph == nullptr) {
      throw std::invalid_argument(getName() + " cannot retrieve the project handle from Cauldron data model");
   }
   m_ph = ph;
}

//------------------------------------------------------------//

void Prograde::ReservoirUpgradeManager::upgrade()
{
   double minTrapCapa = std::numeric_limits<double>::max(); // Minimum trap capacity
   int globalBioDegradInd = 0;
   int globalOilToGasCrackingInd = 0;
   int globalDiffusionInd = 0;
   double globalMinOilColumnHeight = 1.0;
   double globalMinGasColumnHeight = 1.0;
   int globalBlockingInd = 1;
   double globalBlockingPermeability = 1e-9;
   double globalBlockingPorosity = 0;
   auto reservoirs = m_model.reservoirManager().getReservoirsID();
   auto stratigraphy = m_model.stratigraphyManager().layersIDs();
   Prograde::ReservoirConverter  reservoirConverter;
   double trapCapacity; // Trap capacity
   int bioDegradInd;
   int oilToGasCrackingInd;
   int blockingInd;
   double blockingPermeability;
   std::string resName, activityMode;
   int firstTime = 0;
  
   if(reservoirs.size()!=0)
   {
	   // loop over all reservoirs whose activity mode is not equal to "NeverActive" and get correspont fields and appply business logic to find the global value. 
	   //"NeverActive" reservoirs are skipped as these records are decided to be deleted from the ReservoirIoTbl
	   for (size_t resId : reservoirs) 
	   {
		   m_model.reservoirManager().getResActivityMode(resId, activityMode);
		  
		   if (activityMode.compare("NeverActive") != 0)
		   {
			   firstTime++;
			   // getting TrapCapacity
			   m_model.reservoirManager().getResCapacity(resId, trapCapacity);
			   reservoirConverter.trapCapacityLogic(trapCapacity, minTrapCapa);
			   // getting BioDegradInd 
			   m_model.reservoirManager().getResBioDegradInd(resId, bioDegradInd);
			   reservoirConverter.bioDegradIndLogic(bioDegradInd, globalBioDegradInd);
			   // getting OilToGasCrackingInd         
			   m_model.reservoirManager().getResOilToGasCrackingInd(resId, oilToGasCrackingInd);
			   reservoirConverter.oilToGasCrackingIndLogic(oilToGasCrackingInd, globalOilToGasCrackingInd);
			   // getting BlockingInd         
			   m_model.reservoirManager().getResBlockingInd(resId, blockingInd);
			   reservoirConverter.blockingIndLogic(blockingInd, globalBlockingInd);
			   // getting BlockingPermeability        
			   m_model.reservoirManager().getResBlockingPermeability(resId, blockingPermeability);
			   if (firstTime == 1)
			   {
				   globalBlockingPermeability = blockingPermeability;
			   }
			   reservoirConverter.blockingPermeabilityLogic(blockingPermeability, globalBlockingPermeability);
		   }
	   }
	   //
	   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Total number of reservoirs detected is " << reservoirs.size();
	   
	   int countOfDelResId = 0;
	   bool isMatch = false;//bool to check whether the activity mode for a reservoir Id is "NeverActive" or not. If it is true then upgrade the fields with global values, if it is false then remove the record
	   int originalResIDPosition = -1;
	   int nextResId_Index = -1;
	   // loop over all reservoirs and reset the local values with that of the global values for each of the reservoirs whose activity mode is not "NeverActive". Remove the reservoir entry whose activity mode is "NeverActive" 
	   for (size_t resId : reservoirs) 
	   { 
		   double activityStartAge, stratigraphicAge, minOilColHeight, minGasColHeight;
		   originalResIDPosition++;
		   nextResId_Index++;

		   m_model.reservoirManager().getResName(nextResId_Index, resName);
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Reservoir name found in the ReservoirIoTbl is " << resName;

		   m_model.reservoirManager().getResActivityMode(nextResId_Index, activityMode);
		   if (activityMode != "NeverActive")
		   {
			   isMatch = true; // No need to remove the record from the table and upgrade the necessary fields to the corresponding global values
		   }
		   if (isMatch)
		   {
			   // setting globle TrapCapacity
			   m_model.reservoirManager().getResCapacity(nextResId_Index, trapCapacity);
			   if (!NumericFunctions::isEqual(minTrapCapa, trapCapacity, 1e-6))
			   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Original TrapCapacity is '" << trapCapacity << "' and updated TrapCapacity is '" << minTrapCapa << "' for " << resName;
			   m_model.reservoirManager().setResCapacity(nextResId_Index, minTrapCapa);
			   
			   // setting globle BioDegradInd
			   m_model.reservoirManager().getResBioDegradInd(nextResId_Index, bioDegradInd);
			   if(bioDegradInd!= globalBioDegradInd)
			   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Upgrading BioDegradInd to the global value (TRUE, if it is true for any of the detected reservoirs else FALSE). Original BioDegradInd is '" << bioDegradInd << "' which is upgraded to global value of '" << globalBioDegradInd << "' for " << resName;
			   m_model.reservoirManager().setResBioDegradInd(nextResId_Index, globalBioDegradInd);
			   
			   //setting globle OilToGasCrackingInd
			   m_model.reservoirManager().getResOilToGasCrackingInd(nextResId_Index, oilToGasCrackingInd);
			   m_model.reservoirManager().setResOilToGasCrackingInd(nextResId_Index, globalOilToGasCrackingInd);
			   if(oilToGasCrackingInd!= globalOilToGasCrackingInd)
			   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Upgrading OilToGasCrackingInd to the global value (TRUE, if it is true for any of the detected reservoirs else FALSE). Original OilToGasCrackingInd is '" << oilToGasCrackingInd << "' which is upgraded to the global value of '" << globalOilToGasCrackingInd << "' for " << resName;
			   
			   //setting globle BlockingInd
			   m_model.reservoirManager().getResBlockingInd(nextResId_Index, blockingInd);
			   if(blockingInd!= globalBlockingInd)
			   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Upgrading BlockingInd to the global value (TRUE, if it is true for all reservoir else FALSE). Original BlockingInd is '" << blockingInd << "' which is upgraded to the global value of '" << globalBlockingInd << "' for " << resName;
			   m_model.reservoirManager().setResBlockingInd(nextResId_Index, globalBlockingInd);
			   
			   //setting globle BlockingPermeability threshold value
			   m_model.reservoirManager().getResBlockingPermeability(nextResId_Index, blockingPermeability);
			   if (globalBlockingInd == 0)
			   {
				   m_model.reservoirManager().setResBlockingPermeability(nextResId_Index, 1.0e-09);
				   if (!NumericFunctions::isEqual(blockingPermeability, 1.0e-09, 1e-3))
				   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Upgrading BlockingPermeability (if BlockingInd is true and all reservoir have same BlockingPermeability value then that value is the global, else set the default value i.e., 1e-09). Original BlockingPermeability is '" << blockingPermeability << "' which is upgraded to the default global value of 1.0e-09, as the blockingInd is off for " << resName;
			   }
			   else
			   {
				   if (!NumericFunctions::isEqual(blockingPermeability, globalBlockingPermeability, 1e-3))
				   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Upgrading BlockingPermeability (if BlockingInd is true and all reservoir have same BlockingPermeability value then that value is the global, else set the default value i.e., 1e-09). Original BlockingPermeability is '" << blockingPermeability << "' which is upgraded to the global value of '" << globalBlockingPermeability << "' for " << resName;
				   m_model.reservoirManager().setResBlockingPermeability(nextResId_Index, globalBlockingPermeability);
			   }

			   //setting globle DiffusionInd
			   m_model.reservoirManager().setResDiffusionInd(nextResId_Index, globalDiffusionInd);
			   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Upgrading DiffusionInd: there is no DiffusionInd in BPA1. Setting its default value to FALSE for " << resName;
			   
			   //setting globle MinOilColumnHeight
			   m_model.reservoirManager().getResMinOilColumnHeight(nextResId_Index, minOilColHeight);
			   m_model.reservoirManager().setResMinOilColumnHeight(nextResId_Index, globalMinOilColumnHeight);
			   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Upgrading MinOilColumnHeight from '" << minOilColHeight << "' to BPA2 default value of 1m for " << resName;
			   
			   //setting globle MinGasColumnHeight
			   m_model.reservoirManager().getResMinGasColumnHeight(nextResId_Index, minGasColHeight);
			   m_model.reservoirManager().setResMinGasColumnHeight(nextResId_Index, globalMinGasColumnHeight);
			   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Upgrading MinGasColumnHeight from '" << minGasColHeight << "' to BPA2 default value of 1m for " << resName;
			   
			   //setting globle ResBlockingPorosity
			   m_model.reservoirManager().setResBlockingPorosity(nextResId_Index, globalBlockingPorosity);
			   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Upgrading BlockingPorosity: there is no BlockingPorosity in BPA1 and hence its default value is 0 for " << resName;

			   //Safeguard to avoid ActivityStartAge values less than the reservoir depo age. If it is less then upgrade its value to the depo age
			   m_model.reservoirManager().getResActivityStartAge(nextResId_Index, activityStartAge);
			   std::string resFormationName = m_model.tableValueAsString("ReservoirIoTbl", nextResId_Index, "FormationName");
			   for (size_t stratigraphyId : stratigraphy)
			   {
				   std::string layerName = m_model.stratigraphyManager().layerName(stratigraphyId);

				   if (layerName.compare(resFormationName) == 0)
				   {
					   m_model.stratigraphyManager().getDepoAge(stratigraphyId, stratigraphicAge);
					   m_model.reservoirManager().setResActivityStartAge(nextResId_Index, reservoirConverter.upgradeActivityStartAge(activityMode, stratigraphicAge, activityStartAge));
					   break;
				   }
			   }

			   isMatch = false;

		   }
		   else
		   {
			   //remove the record since the activity mode is found to be "NeverActive"
			   countOfDelResId++;
			   m_model.removeRecordFromTable("ReservoirIoTbl", (originalResIDPosition + 1) - countOfDelResId);
			   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> Activity mode is found to be 'NeverActive' for the current reservoir layer. Deleting this record from the ReservoirIoTbl";
			   nextResId_Index--;
		   }
	   }
	   // Updating ReservoirOptionsIoTbl
	   // Globle values are obtained from ReservoirIoTbl
	   size_t resId = 0;   // there is only one row in ReservoirOptionsIoTable, hence resID = 0 ;
	   m_model.reservoirManager().setResOptionsCapacity(resId, minTrapCapa);
	   m_model.reservoirManager().setResOptionsBioDegradInd(resId, globalBioDegradInd);
	   m_model.reservoirManager().setResOptionsOilToGasCrackingInd(resId, globalOilToGasCrackingInd);
	   m_model.reservoirManager().setResOptionsBlockingInd(resId, globalBlockingInd);
	   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Creating ReservoirOptionsIoTable with the global parameters determined from the legacy ReservoirIoTbl inputs";
	   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Setting TrapCapacity : " << minTrapCapa;
	   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Setting BioDegradInd : " << globalBioDegradInd;
	   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Setting OilToGasCrackingInd : " << globalOilToGasCrackingInd;
	   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Setting BlockingInd : " << globalBlockingInd;

	   if (globalBlockingInd == 0)
	   {
		   m_model.reservoirManager().setResOptionsBlockingPermeability(resId, 1.0e-09);
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Setting BlockingPermeability : 1.0e-09";
	   }
	   else
	   {
		   m_model.reservoirManager().setResOptionsBlockingPermeability(resId, globalBlockingPermeability);
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Setting BlockingPermeability : "<< globalBlockingPermeability;
	   }

	   m_model.reservoirManager().setResOptionsDiffusionInd(resId, globalDiffusionInd);
	   m_model.reservoirManager().setResOptionsMinOilColumnHeight(resId, globalMinOilColumnHeight);
	   m_model.reservoirManager().setResOptionsMinGasColumnHeight(resId, globalMinGasColumnHeight);
	   m_model.reservoirManager().setResOptionsBlockingPorosity(resId, globalBlockingPorosity);
	   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Setting DiffusionInd : " << globalDiffusionInd;
	   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Setting MinOilColumnHeight : " << globalMinOilColumnHeight;
	   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Setting MinGasColumnHeight : " << globalMinGasColumnHeight;
	   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Setting BlockingPorosity : " << globalBlockingPorosity;
   }


   /*	clearing the reference from GridmapIoTbl which are no longer used in ReservoirIoTbl
   *	Since all the grid map related columns in ReservoirIoTbl are deprecated except NetToGrossGrid and ThicknessGrid
   *	So, all the grid map references from GridMapIoTbl related to ReservoirIoTbl are cleared 
   *	except the maps in NetToGrossGrid and ThicknessGrid
   */

   // Collect the maps present in the NetToGrossGrid and ThicknessGrid columns of the ReservoirIoTbl in a vector
   std::string mapName;
   std::vector<std::string> mapsToBeRetained;
   database::Table* reservoirIoTbl = m_ph->getTable("ReservoirIoTbl");
   for (size_t id = 0; id < reservoirIoTbl->size(); ++id)
   {
	   const database::Record* rec = reservoirIoTbl->getRecord(static_cast<int>(id));
	   mapName = rec->getValue<std::string>("NetToGrossGrid");
	   if(mapName.compare(""))
		   mapsToBeRetained.push_back(mapName);
	   mapName = rec->getValue<std::string>("ThicknessGrid");
	   if (mapName.compare(""))
		   mapsToBeRetained.push_back(mapName);
   }
   /// note that the gridmaps are collected after "NeverActive" reservoirs are cleared.
   /// hence, if there is any NetToGrossGrid or ThicknessGrid map was present initially in a NeverActive reservoir,
   /// that will also get cleared

   // Collect the grid maps present in the GridMapIoTbl referred by ReservoirIoTbl in another vector
   std::string referredBy;
   std::vector<std::string> gridmapsReservoirGridmapIoTbl;
   database::Table* gridmapIoTbl = m_ph->getTable("GridMapIoTbl");
   for (size_t id = 0; id < gridmapIoTbl->size(); ++id)
   {
	   const database::Record* rec = gridmapIoTbl->getRecord(static_cast<int>(id));
	   referredBy = rec->getValue<std::string>("ReferredBy");
	   if (!referredBy.compare("ReservoirIoTbl"))
	   {
		   mapName = rec->getValue<std::string>("MapName");
		   gridmapsReservoirGridmapIoTbl.push_back(mapName);
	   }
   }

   // Identify the ReservoirIoTbl maps to be cleared from GridMapIoTbl

   /// if GridMapIoTbl does not have any Reservoir related maps, then there is no need to clear anything
   if (!gridmapsReservoirGridmapIoTbl.empty())
   {
	   /// clear all the maps referred by ReservoirIoTbl except those in the mapsToBeRetained list   
	   /// loop through all the Reservoir maps in GridMapIoTbl
	   for (const auto& reservoirMapGridmapIoTbl : gridmapsReservoirGridmapIoTbl)
	   {
		   bool toBeRetained = false; //flag to check if the map should be retained

		   /// check if the ReservoirMap in GridMapIoTbl should be retained 
		   for (const auto& gridmap : mapsToBeRetained)
		   {
			   /// if yes, then change the toBeRetained flag to true
			   if (!reservoirMapGridmapIoTbl.compare(gridmap))
			   {
				   toBeRetained = true;
				   break;
			   }
		   }

		   /// toBeRetained is false for a Reservoir map in GridMapIoTbl, then that map is cleared from GridMapIoTbl
		   if (!toBeRetained)
		   {
			   Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap("ReservoirIoTbl", reservoirMapGridmapIoTbl);
			   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Gridmap " << reservoirMapGridmapIoTbl << " ReferredBy ReservoirIoTbl will be cleared by GridMapIoTbl Upgrade Manager";
		   }
	   }
   }

	// DetectedReservoirIoTbl is deprecated, hence all the maps in GridMapIoTbl related to DetectedReservoirIoTbl are cleared
   Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap("DetectedReservoirIoTbl");
}