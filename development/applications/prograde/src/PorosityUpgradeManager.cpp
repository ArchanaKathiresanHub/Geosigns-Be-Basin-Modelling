//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "PorosityUpgradeManager.h"

//std
#include <vector>

//utilities
#include "LogHandler.h"

//Prograde
#include "PorosityModelConverter.h"

//cmbAPI
#include "cmbAPI.h"
#include "LithologyManager.h"

//DataAccess
#include "Interface/ProjectHandle.h"
#include "Interface/RunParameters.h"

using namespace mbapi;

//------------------------------------------------------------//

Prograde::PorosityUpgradeManager::PorosityUpgradeManager( mbapi::Model& model ):
   IUpgradeManager( "porosity model upgrade manager" ), m_model( model )
{
   const auto ph = m_model.projectHandle();
   if (ph == nullptr) {
      throw std::invalid_argument( getName() + " cannot retreive the project handle from Cauldron data model" );
   }
   m_ph = ph ;
}

//------------------------------------------------------------//

void Prograde::PorosityUpgradeManager::upgrade() {
   Prograde::PorosityModelConverter modelConverter;
   auto lithologies = m_model.lithologyManager().lithologiesIDs();
   for(auto lithoId : lithologies){
      std::string lithoName;
      lithoName = m_model.lithologyManager().lithologyName(lithoId);
      mbapi::LithologyManager::PorosityModel porModel; // porosity calculation model
      std::vector<double> porModelPrms; // poro. model parameters, depends on the given model
      m_model.lithologyManager().porosityModel(lithoId, porModel, porModelPrms);
      std::string lithoDescription;              //Lithology description
      lithoDescription = m_model.lithologyManager().getDescription(lithoId);
      m_model.lithologyManager().setDescription(lithoId, modelConverter.upgradeDescription(lithoName, lithoDescription, porModel));
      m_model.lithologyManager().setPorosityModel(lithoId, modelConverter.upgradePorosityModel(lithoName, porModel),
         modelConverter.upgradeModelParameters(lithoName, porModel, porModelPrms));
   }
}
