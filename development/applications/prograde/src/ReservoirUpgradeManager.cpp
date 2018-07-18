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

//DataAccess
#include "Interface/ProjectHandle.h"
#include "Interface/RunParameters.h"

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

void Prograde::ReservoirUpgradeManager::upgrade() {

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
   Prograde::ReservoirConverter  reservoirConverter;
   double trapCapacity; // Trap capacity
   int bioDegradInd;
   int oilToGasCrackingInd;
   int blockingInd;
   double blockingPermeability;
   // loop over all reservoirs and get correspont fields and appply business logic 
   for (size_t resId : reservoirs) {
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
      reservoirConverter.blockingPermeabilityLogic(resId, blockingPermeability, globalBlockingPermeability);
   }
   //
   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "number of reservoirs detected is " << reservoirs.size();
   for (size_t resId : reservoirs) { 
      std::string resName;
      m_model.reservoirManager().getResName(resId, resName);
// setting globle TrapCapacity
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "upgrading TrapCapacity to minimum TrapCapacity among all reservoirs for " << resName;
      m_model.reservoirManager().getResCapacity(resId, trapCapacity);
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Original TrapCapacity is '"<< trapCapacity<< "' and updated TrapCapacity is '"<<minTrapCapa<<"' for " << resName;
      m_model.reservoirManager().setResCapacity(resId, minTrapCapa);
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "TrapCapacity upgrade done for " << resName;
// setting globle BioDegradInd
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "upgrading BioDegradInd: TRUE(i.e., 1), if it is true for any one reservoir else FALSE(i.e., 0) for " << resName;
      m_model.reservoirManager().getResBioDegradInd(resId, bioDegradInd);
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Original BioDegradInd is '" << bioDegradInd << "' and updated BioDegradInd is '" << globalBioDegradInd << "' for " << resName;
      m_model.reservoirManager().setResBioDegradInd(resId, globalBioDegradInd);
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "BioDegradInd upgrade done for " << resName;
//setting globle OilToGasCrackingInd
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "upgrading OilToGasCrackingInd: TRUE, if it is true for any one reservoir else FALSE for " << resName;
      m_model.reservoirManager().getResOilToGasCrackingInd(resId, oilToGasCrackingInd);
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Original OilToGasCrackingInd is '" << oilToGasCrackingInd << "' and updated OilToGasCrackingInd is '" << globalOilToGasCrackingInd << "' for " << resName;
      m_model.reservoirManager().setResOilToGasCrackingInd(resId, globalOilToGasCrackingInd);
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "OilToGasCrackingInd upgrade done for " << resName;
//setting globle BlockingInd
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "upgrading BlockingInd: TRUE, if it is true for all reservoir else FALSE for " << resName;
      m_model.reservoirManager().getResBlockingInd(resId, blockingInd);
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Original BlockingInd is '" << blockingInd << "' and updated BlockingInd is '" << globalBlockingInd << "' for " << resName;
      m_model.reservoirManager().setResBlockingInd(resId, globalBlockingInd);
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "BlockingInd upgrade done for " << resName;
//setting globle BlockingPermeability
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "upgrading BlockingPermeability: if it is true and all reservoir have same BlockingPermeability value then that value is the global  else default global valkue is 1e-09 for " << resName;
      m_model.reservoirManager().getResBlockingPermeability(resId, blockingPermeability);
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Original BlockingPermeability is '" << blockingPermeability << "' and updated BlockingPermeability is '" << globalBlockingPermeability << "' for " << resName;
      m_model.reservoirManager().setResBlockingPermeability(resId, globalBlockingPermeability);
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "BlockingPermeability upgrade done for " << resName;
//setting globle DiffusionInd
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "upgrading DiffusionInd: there is no DiffusionInd in BPA1 and hence its default value is FALSE for " << resName;
      m_model.reservoirManager().setResDiffusionInd(resId, globalDiffusionInd);
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "DiffusionInd upgrade done for " << resName;
//setting globle MinOilColumnHeight
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "upgrading MinOilColumnHeight: there is no MinOilColumnHeight in BPA1 and hence its default value is 1m for " << resName;
      m_model.reservoirManager().setResMinOilColumnHeight(resId, globalMinOilColumnHeight);
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "MinOilColumnHeight upgrade done for " << resName;
//setting globle MinGasColumnHeight
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "upgrading MinGasColumnHeight: there is no MinGasColumnHeight in BPA1 and hence its default value is 1m for " << resName;
      m_model.reservoirManager().setResMinGasColumnHeight(resId, globalMinGasColumnHeight);
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "MinGasColumnHeight upgrade done for " << resName;
//setting globle ResBlockingPorosity
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "upgrading BlockingPorosity: there is no BlockingPorosity in BPA1 and hence its default value is 0 for " << resName;
      m_model.reservoirManager().setResBlockingPorosity(resId, globalBlockingPorosity);
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "BlockingPorosity upgrade done for " << resName;
   }
// Updating ReservoirOptionsIoTbl
// Globle values are obtained from ReservoirIoTbl
   size_t resId = 0;   // there is only one row in ReservoirOptionsIoTable, hence resID = 0 ;
   m_model.reservoirManager().setResOptionsCapacity(resId, minTrapCapa);
   
   m_model.reservoirManager().setResOptionsBioDegradInd(resId, globalBioDegradInd);
   
   m_model.reservoirManager().setResOptionsOilToGasCrackingInd(resId, globalOilToGasCrackingInd);
   
   m_model.reservoirManager().setResOptionsBlockingInd(resId, globalBlockingInd);
   
   m_model.reservoirManager().setResOptionsBlockingPermeability(resId, globalBlockingPermeability);
   
   m_model.reservoirManager().setResOptionsDiffusionInd(resId, globalDiffusionInd);
   
   m_model.reservoirManager().setResOptionsMinOilColumnHeight(resId, globalMinOilColumnHeight);
   
   m_model.reservoirManager().setResOptionsMinGasColumnHeight(resId, globalMinGasColumnHeight);
   
   m_model.reservoirManager().setResOptionsBlockingPorosity(resId, globalBlockingPorosity);
   
}