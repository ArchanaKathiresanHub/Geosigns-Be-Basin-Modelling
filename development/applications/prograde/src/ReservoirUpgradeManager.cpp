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

//utilities
#include "LogHandler.h"

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
      throw std::invalid_argument(getName() + " cannot retreive the project handle from Cauldron data model");
   }
   m_ph = ph;
}

//------------------------------------------------------------//

void Prograde::ReservoirUpgradeManager::upgrade()
{
   double minTrapCapa = numeric_limits<double>::max(); // Minimum trap capacity
   int globalBioDegradInd = 0;
   int globalOilToGasCrackingInd = 0;
   int globalDiffusionInd = 0;
   double globalMinOilColumnHeight = 1.0;
   double globalMinGasColumnHeight = 1.0;
   int globalBlockingInd = 1;
   double globalBlockingPermeability;
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
			   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Original TrapCapacity is '" << trapCapacity << "' and updated TrapCapacity is '" << minTrapCapa << "' for " << resName;
			   m_model.reservoirManager().setResCapacity(nextResId_Index, minTrapCapa);
			   
			   // setting globle BioDegradInd
			   m_model.reservoirManager().getResBioDegradInd(nextResId_Index, bioDegradInd);
			   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Upgrading BioDegradInd to the global value (TRUE, if it is true for any of the detected reservoirs else FALSE). Original BioDegradInd is '" << bioDegradInd << "' which is upgraded to global value of '" << globalBioDegradInd << "' for " << resName;
			   m_model.reservoirManager().setResBioDegradInd(nextResId_Index, globalBioDegradInd);
			   
			   //setting globle OilToGasCrackingInd
			   m_model.reservoirManager().getResOilToGasCrackingInd(nextResId_Index, oilToGasCrackingInd);
			   m_model.reservoirManager().setResOilToGasCrackingInd(nextResId_Index, globalOilToGasCrackingInd);
			   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Upgrading OilToGasCrackingInd to the global value (TRUE, if it is true for any of the detected reservoirs else FALSE). Original OilToGasCrackingInd is '" << oilToGasCrackingInd << "' which is upgraded to the global value of '" << globalOilToGasCrackingInd << "' for " << resName;
			   
			   //setting globle BlockingInd
			   m_model.reservoirManager().getResBlockingInd(nextResId_Index, blockingInd);
			   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Upgrading BlockingInd to the global value (TRUE, if it is true for all reservoir else FALSE). Original BlockingInd is '" << blockingInd << "' which is upgraded to the global value of '" << globalBlockingInd << "' for " << resName;
			   m_model.reservoirManager().setResBlockingInd(nextResId_Index, globalBlockingInd);
			   
			   //setting globle BlockingPermeability threshold value
			   m_model.reservoirManager().getResBlockingPermeability(nextResId_Index, blockingPermeability);
			   if (globalBlockingInd == 0)
			   {
				   m_model.reservoirManager().setResBlockingPermeability(nextResId_Index, 1.0e-09);
				   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Upgrading BlockingPermeability (if BlockingInd is true and all reservoir have same BlockingPermeability value then that value is the global, else set the default value i.e., 1e-09). Original BlockingPermeability is '" << blockingPermeability << "' which is upgraded to the default global value of 1.0e-09, as the blockingInd is off for " << resName;
			   }
			   else
			   {
				   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Upgrading BlockingPermeability (if BlockingInd is true and all reservoir have same BlockingPermeability value then that value is the global, else set the default value i.e., 1e-09). Original BlockingPermeability is '" << blockingPermeability << "' which is upgraded to the global value of '" << globalBlockingPermeability << "' for " << resName;
				   m_model.reservoirManager().setResBlockingPermeability(nextResId_Index, globalBlockingPermeability);
			   }

			   //setting globle DiffusionInd
			   m_model.reservoirManager().setResDiffusionInd(nextResId_Index, globalDiffusionInd);
			   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Upgrading DiffusionInd: there is no DiffusionInd in BPA1. Setting its default value to FALSE for " << resName;
			   
			   //setting globle MinOilColumnHeight
			   m_model.reservoirManager().getResMinOilColumnHeight(nextResId_Index, minOilColHeight);
			   m_model.reservoirManager().setResMinOilColumnHeight(nextResId_Index, globalMinOilColumnHeight);
			   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Upgrading MinOilColumnHeight from '" << minOilColHeight << "' to BPA2 default value of 1m for " << resName;
			   
			   //setting globle MinGasColumnHeight
			   m_model.reservoirManager().getResMinGasColumnHeight(nextResId_Index, minGasColHeight);
			   m_model.reservoirManager().setResMinGasColumnHeight(nextResId_Index, globalMinGasColumnHeight);
			   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Upgrading MinGasColumnHeight from '" << minGasColHeight << "' to BPA2 default value of 1m for " << resName;
			   
			   //setting globle ResBlockingPorosity
			   m_model.reservoirManager().setResBlockingPorosity(nextResId_Index, globalBlockingPorosity);
			   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Upgrading BlockingPorosity: there is no BlockingPorosity in BPA1 and hence its default value is 0 for " << resName;

			   //Safeguard to avoid ActivityStartAge values less than the reservoir depo age. If it is less then upgrade its value to the depo age
			   m_model.reservoirManager().getResActivityStartAge(nextResId_Index, activityStartAge);
			   for (size_t stratigraphyId : stratigraphy)
			   {
				   std::string layerName = m_model.stratigraphyManager().layerName(stratigraphyId);

				   if (layerName.compare(resName) == 0)
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
			   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Activity mode is found to be 'NeverActive' for the current reservoir layer. Deleting this record from the ReservoirIoTbl";
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
	   if (globalBlockingInd == 0)
	   {
		   m_model.reservoirManager().setResOptionsBlockingPermeability(resId, 1.0e-09);
	   }
	   else
	   {
		   m_model.reservoirManager().setResOptionsBlockingPermeability(resId, globalBlockingPermeability);
	   }

	   m_model.reservoirManager().setResOptionsDiffusionInd(resId, globalDiffusionInd);
	   m_model.reservoirManager().setResOptionsMinOilColumnHeight(resId, globalMinOilColumnHeight);
	   m_model.reservoirManager().setResOptionsMinGasColumnHeight(resId, globalMinGasColumnHeight);
	   m_model.reservoirManager().setResOptionsBlockingPorosity(resId, globalBlockingPorosity);
   }
   else
   {
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Number of reservoirs detected is 0. No update is to be done";
   }
   
}
