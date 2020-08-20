//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "PermeabilityUpgradeManager.h"

//std
#include <vector>

//utilities
#include "LogHandler.h"

//Prograde
#include "SandstoneToMultipointPermeability.h"
#include "MudstoneToMultipointPermeability.h"

//cmbAPI
#include "cmbAPI.h"
#include "LithologyManager.h"

//DataAccess
#include "ProjectHandle.h"
#include "RunParameters.h"

using namespace mbapi;

//------------------------------------------------------------//

Prograde::PermeabilityUpgradeManager::PermeabilityUpgradeManager( mbapi::Model& model ):
   IUpgradeManager( "permeability model upgrade manager" ), m_model( model )
{
   const auto ph = m_model.projectHandle();
   if (ph == nullptr) {
      throw std::invalid_argument( getName() + " cannot retrieve the project handle from Cauldron data model" );
   }
   m_ph = ph ;
}

//------------------------------------------------------------//

void Prograde::PermeabilityUpgradeManager::upgrade() {

   auto lithologies = m_model.lithologyManager().lithologiesIDs();
   for(auto lithoId : lithologies){
      mbapi::LithologyManager::PermeabilityModel prmModel; // permeability calculation model
      std::vector<double> modelPrms; // perm. model parameters, depends on the given model
      std::vector<double> mpPor;     // for multi-point perm. model the porosity values vector. Empty for other models
      std::vector<double> mpPerm;    // for multi-point perm. model the log. of perm values vector. Empty for other models.
      m_model.lithologyManager().permeabilityModel(lithoId, prmModel, modelPrms, mpPor, mpPerm);
      mbapi::LithologyManager::PorosityModel porModel; // porosity calculation model
      std::vector<double> porModelPrms; // poro. model parameters, depends on the given model
      m_model.lithologyManager().porosityModel(lithoId, porModel, porModelPrms);
      switch(prmModel){
         case mbapi::LithologyManager::PermSandstone:
         {
            LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP ) << "legacy sandstone permeability model detected";
            LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS ) << "upgrading project to multipoint permeability model";
            SandstoneToMultipointPermeability sandstoneConverter({modelPrms[LithologyManager::AnisotropySand],     // permeability anisotropy
                                                                  modelPrms[LithologyManager::PermSurfSand],       // depositional permeability
                                                                  modelPrms[LithologyManager::SandClayPercentage], // permeability sand/clay ration [%]
                                                                  porModelPrms[LithologyManager::PhiSurf],         // depositional porosity
                                                                  porModelPrms[LithologyManager::PhiMin]});        // minimum mechanical porosity
            sandstoneConverter.compute(modelPrms, mpPor, mpPerm);
            m_model.lithologyManager().setPermeabilityModel(lithoId, LithologyManager::PermMultipoint, modelPrms, mpPor, mpPerm);
            LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS ) << "permeability model upgrade done";
            break;
         }
         case mbapi::LithologyManager::PermMudstone:
         {
            LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP ) << "legacy mudstone permeability model detected";
            LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS ) << "upgrading project to multipoint permeability model";
            const auto chemicalCompaction = m_ph->getRunParameters()->getChemicalCompaction();
            MudstoneToMultipointPermeability mudstoneConverter({modelPrms[LithologyManager::AnisotropyMud],          // permeability anisotropy
                                                                modelPrms[LithologyManager::PermSurfMud],            // depositional permeability
                                                                modelPrms[LithologyManager::SensitivityCoef],        // permeability sensitivity coefficient
                                                                porModelPrms[LithologyManager::PhiSurf],             // depositional porosity
                                                                porModelPrms[LithologyManager::CompactionCoef]*1e-2, // compaction coefficient [1/Mpa]
                                                                porModelPrms[LithologyManager::PhiMin],              // minimum mechanical porosity
                                                                chemicalCompaction});                                // chemical compaction
            mudstoneConverter.compute(modelPrms, mpPor, mpPerm);
            m_model.lithologyManager().setPermeabilityModel(lithoId, LithologyManager::PermMultipoint, modelPrms, mpPor, mpPerm);
            LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS ) << "permeability model upgrade done";
            break;
         }
         case mbapi::LithologyManager::PermNone:
         case mbapi::LithologyManager::PermImpermeable:
         case mbapi::LithologyManager::PermMultipoint:
         case mbapi::LithologyManager::PermUnknown:
         default:
            LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP ) << "latest permeability model detected";
            LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS ) << "no permeability model upgrade needed";
            break;
      }
   }
}

