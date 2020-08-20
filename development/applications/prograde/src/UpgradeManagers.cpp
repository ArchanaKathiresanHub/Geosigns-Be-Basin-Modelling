//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "UpgradeManagers.h"

//std
#include <vector>
#include <algorithm>
#include <memory>

//Prograde
#include "AlcUpgradeManager.h"
#include "PorosityUpgradeManager.h"
#include "PermeabilityUpgradeManager.h"
#include "BrineUpgradeManager.h"
#include "ReservoirUpgradeManager.h"
#include "BiodegradeUpgradeManager.h"
#include "BasicCrustThinningUpgradeManager.h"
#include "CtcUpgradeManager.h"
#include "FracturePressureUpgradeManager.h"
#include "ProjectIoTblUpgradeManager.h"
#include "TopBoundaryUpgradeManager.h"
#include "RunOptionsUpgradeManager.h"
#include "LithologyUpgradeManager.h"
#include "SgsUpgradeManager.h"
#include "SourceRockUpgradeManager.h"
#include "StratigraphyUpgradeManager.h"
#include "HeatFlowModeUpgradeManager.h"
#include "FaultCutUpgradeManager.h"
#include "GridMapIoTblUpgradeManager.h"

//Prograde
#include "IUpgradeManager.h"

//utilities
#include "LogHandler.h"

#define isDoubleExponential 0 //1=Yes, 0=No => implies if the deprecated SM porosity model is to be upgraded to double exponential model of BPA2 or not  

void Prograde::UpgradeManagers::runAll() const{
   ///I. Create list of upgrade managers
   std::vector<std::shared_ptr<Prograde::IUpgradeManager>> managers;

   // 1. Project details upgradation 
   managers.emplace_back(std::unique_ptr<Prograde::ProjectIoTblUpgradeManager>(new Prograde::ProjectIoTblUpgradeManager(m_model)));

   // 2. Strat Io Table upgradation 
   managers.emplace_back(std::unique_ptr<Prograde::StratigraphyUpgradeManager>(new Prograde::StratigraphyUpgradeManager(m_model)));
   
   //3. //@brief Upgrading ALCv1 to ILCLEM mode of BPA2
   managers.emplace_back(std::unique_ptr<Prograde::AlcUpgradeManager>(new Prograde::AlcUpgradeManager(m_model)));

   //4. //@brief Upgrading legacy lithotype inputs to the standards of BPA2 lithotype 
   /*@details Upgrading the legacy system defined lithotype names, deprecated SM porosity model to Exponential model, deprecated permeability models to multipoint model etc. */
   managers.emplace_back(std::unique_ptr<Prograde::LithologyUpgradeManager>(new Prograde::LithologyUpgradeManager(m_model)));

   //@brief Upgrading Soil mechanics compaction model to Double exponential compaction model. 
   /*@details Upgrading Soil Mechanics to Double Exponential model is discarded as a new approach to convert Soil Mechanics Model to Single Exponential model 
			  is finalized for porosity model upgradation. The upgradation of deprecated SM porosity model to Single Exponential is taken care in the "LithologyUpgradeManager"*/
#if isDoubleExponential
	managers.emplace_back(std::unique_ptr<Prograde::PorosityUpgradeManager>(new Prograde::PorosityUpgradeManager(m_model)));
#endif

   //@brief Upgrading deprecated Mudstone and Sandstone permeability models to Multipoint permeability model.
   //@details This upgradation strategy is replaced by new strategy proposed by Lorcan and is implemented in the "LithologyUpgradeManager"
   //managers.emplace_back(std::unique_ptr<Prograde::PermeabilityUpgradeManager>(new Prograde::PermeabilityUpgradeManager(m_model)));

   //5. //@brief Upgrading density and seismic velocity calculations of Brines
   /*@details Constant models to calculate brine density and seismic vel are upgraded to modified B&W model, legacy "Std. Merine water" is upgraded to user-defined brine,
			  Cleared brine details from FluidTypeIotbl if the brine is nor referred in the model, upgraded HeatCaptype/ ThermCondtype of each brine to the respective brine 
			  upgrading the */
   managers.emplace_back(std::unique_ptr<Prograde::BrineUpgradeManager>(new Prograde::BrineUpgradeManager(m_model)));

   //6. BPA1 reservoir layer-wise parameters to BPA2 reservoir global parameters
   managers.emplace_back(std::unique_ptr<Prograde::ReservoirUpgradeManager>(new Prograde::ReservoirUpgradeManager(m_model)));

   //7. Biodegradation
   managers.emplace_back(std::unique_ptr<Prograde::BiodegradeUpgradeManager>(new Prograde::BiodegradeUpgradeManager(m_model)));

   //8. Basic crust thinning bottom boundary model to the new Crust Thinning model
   managers.emplace_back(std::unique_ptr<Prograde::BasicCrustThinningUpgradeManager>(new Prograde::BasicCrustThinningUpgradeManager(m_model)));

   //9. CTCv1 to CTCv2
   managers.emplace_back(std::unique_ptr<Prograde::CtcUpgradeManager>(new Prograde::CtcUpgradeManager(m_model)));

   //10. Fracture pressure 
   managers.emplace_back(std::unique_ptr<Prograde::FracturePressureUpgradeManager>(new Prograde::FracturePressureUpgradeManager(m_model)));

   //11. Top Boundary conditions
   managers.emplace_back(std::unique_ptr<Prograde::TopBoundaryUpgradeManager>(new Prograde::TopBoundaryUpgradeManager(m_model)));

   //12. Run Options Io Table
   managers.emplace_back(std::unique_ptr<Prograde::RunOptionsUpgradeManager>(new Prograde::RunOptionsUpgradeManager(m_model)));

   //13. Shale Gas Io Table upgradation
   managers.emplace_back(std::unique_ptr<Prograde::SgsUpgradeManager>(new Prograde::SgsUpgradeManager(m_model)));

   //14. SourceRockLitho and Strat Io Table upgradation
   managers.emplace_back(std::unique_ptr<Prograde::SourceRockUpgradeManager>(new Prograde::SourceRockUpgradeManager(m_model)));

   //15. //@brief Upgrading HeatFlow mode related inputs 
   managers.emplace_back(std::unique_ptr<Prograde::HeatFlowModeUpgradeManager>(new Prograde::HeatFlowModeUpgradeManager(m_model)));

   //16. FaultCut related tables upgradation
   managers.emplace_back(std::unique_ptr<Prograde::FaultCutUpgradeManager>(new Prograde::FaultCutUpgradeManager(m_model)));

   // Other managers to be added in the same way BUT BEFORE GridMapIoTblUpgradeManager 
   // Add other managers HERE

   // GridMapIoTbl upgradation - deletion of unnecessary GridMaps from GridMapIoTbl
   managers.emplace_back(std::unique_ptr<Prograde::GridMapIoTblUpgradeManager>(new Prograde::GridMapIoTblUpgradeManager(m_model)));

   ///II. Run all upgrade managers
   std::for_each(managers.begin(), managers.end(), [] ( std::shared_ptr<Prograde::IUpgradeManager> manager)
   {
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP ) << "running " + manager->getName();
      manager->upgrade();
   });
}