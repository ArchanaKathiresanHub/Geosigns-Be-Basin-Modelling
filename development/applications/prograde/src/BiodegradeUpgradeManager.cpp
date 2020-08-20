//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "BiodegradeConverter.h"
#include "BiodegradeUpgradeManager.h"

//utilities
#include "LogHandler.h"

//cmbAPI
#include "cmbAPI.h"
#include <map>

using namespace mbapi;

Prograde::BiodegradeUpgradeManager::BiodegradeUpgradeManager(mbapi::Model& model) :
   IUpgradeManager("Biodegradation upgrade manager"), m_model(model)
{
   const auto ph = m_model.projectHandle();
   if (ph == nullptr) {
      throw std::invalid_argument(getName() + " cannot retrieve the project handle from Cauldron data model");
   }
   m_ph = ph;
}

// To populate standard biodegradation factors (default values) which are used to compare the vlaues in BioDegradIoTbl
void GetStandardBioDegrdFactor(std::map<std::string, double> & mapBioDegConstants) {

   mapBioDegConstants = {
                           { "MaxBioTemp", 80.0 },
                           { "TempConstant", 70.0 },
                           { "TimeConstant", 0.5 },
                           { "BioRate", 0.3 },
                           { "C1_BioFactor", 0.0008 },
                           { "C2_BioFactor", 0.0005 },
                           { "C3_BioFactor", 0.0003 },
                           { "C4_BioFactor", 0.0008 },
                           { "C5_BioFactor", 0.001 },
                           { "N2_BioFactor", 0.001 },
                           { "COx_BioFactor", 1.0 },
                           { "C6_14Aro_BioFactor", 0.007 },
                           { "C6_14Sat_BioFactor", 0.009 },
                           { "C15Aro_BioFactor", 0.05 },
                           { "C15Sat_BioFactor", 0.011 },
                           { "resins_BioFactor", 0.061 },
                           { "asphaltenes_BioFactor", 0.1 } 
                        };

}

void Prograde::BiodegradeUpgradeManager::upgrade() {

   std::map<std::string, double> mapBioDegConstants;
   Prograde::BiodegradeConverter bdrConv;

   // populate standard biodegradation factors (default values)
   GetStandardBioDegrdFactor(mapBioDegConstants);

   for (auto& itr : mapBioDegConstants) {
      double valueFromP3dFile;

      // get biodegradation constant from BioDegradIoTbl in project3d file
      m_model.biodegradeManager().getBioConstant(itr.first, valueFromP3dFile);
      // check and update biodegradation constants if it deviates from default value or deviates from valid range
      bdrConv.upgradeBioConstants(itr.first, itr.second, valueFromP3dFile);
      // set biodegradation constant in project3d file
      m_model.biodegradeManager().setBioConstant(itr.first, valueFromP3dFile);
   }

}
