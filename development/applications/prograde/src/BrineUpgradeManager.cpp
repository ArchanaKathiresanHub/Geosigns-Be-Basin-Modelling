//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "BrineUpgradeManager.h"
#include "BrineModelConverter.h"

//utilities
#include "LogHandler.h"

//cmbAPI
#include "cmbAPI.h"
#include "FluidManager.h"

//DataAccess
#include "Interface/ProjectHandle.h"
#include "Interface/RunParameters.h"

using namespace mbapi;

//------------------------------------------------------------//

Prograde::BrineUpgradeManager::BrineUpgradeManager(Model& model) :
   IUpgradeManager("brine upgrade manager"), m_model(model)
{
   const auto ph = m_model.projectHandle();
   if (ph == nullptr) {
      throw std::invalid_argument(getName() + " cannot retreive the project handle from Cauldron data model");
   }
   m_ph = ph;
}

//------------------------------------------------------------//

void Prograde::BrineUpgradeManager::upgrade() {
   Prograde::BrineModelConverter modelConverter;
   auto fluids = m_model.fluidManager().getFluidsID();
   for (auto flId : fluids) {
      std::string fluidName;
      m_model.fluidManager().getFluidName(flId, fluidName);

      int fluidUserDefined;                      //Fluid user defined flag
      m_model.fluidManager().getUserDefined(flId, fluidUserDefined);
      m_model.fluidManager().setUserDefined(flId, modelConverter.upgradeUserDefined(fluidName, fluidUserDefined));
      std::string fluidDescription;              //Fluid description
      m_model.fluidManager().getDescription(flId, fluidDescription);
      m_model.fluidManager().setDescription(flId, modelConverter.upgradeDescription(fluidName, fluidDescription));

      FluidManager::FluidDensityModel densModel; // Fluid density calculation model
      double refDens;
      m_model.fluidManager().densityModel(flId, densModel, refDens);
      m_model.fluidManager().setDensityModel(flId, modelConverter.upgradeDensityModel(densModel, fluidName), refDens);

      FluidManager::CalculationModel seisVelModel; // Seismic velocity calculation model
      double refSeisVel;
      m_model.fluidManager().seismicVelocityModel(flId, seisVelModel, refSeisVel);
      m_model.fluidManager().setSeismicVelocityModel(flId, modelConverter.upgradeSeismicVelocityModel(seisVelModel, fluidName), refSeisVel);

   }
}
