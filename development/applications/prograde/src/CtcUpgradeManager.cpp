//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "CtcUpgradeManager.h"

//std
#include <vector>

//utilities
#include "LogHandler.h"

//cmbAPI
#include "cmbAPI.h"
#include "CtcManager.h"

//DataAccess
#include "Interface/ProjectHandle.h"
#include "Interface/RunParameters.h"

using namespace mbapi;

//------------------------------------------------------------//

Prograde::CtcUpgradeManager::CtcUpgradeManager(mbapi::Model& model) :
   IUpgradeManager("crustal thickness calculator upgrade manager"), m_model(model)
{
   const auto ph = m_model.projectHandle();
   if (ph == nullptr) {
      throw std::invalid_argument(getName() + " cannot retreive the project handle from Cauldron data model");
   }
   m_ph = ph;
}

//------------------------------------------------------------//
void Prograde::CtcUpgradeManager::upgrade() {
  
   //Check the default value of FilterHalfWidth which is 5 in BPA-Cauldron and 10 in BPA2-BAsin. If not 5 then set its value in CTCIoTbl to the default value of BPA-Cauldron
   int FHWidth = 5;
   int temp;

   m_model.ctcManager().getFilterHalfWidthValue(temp);
   if (temp != 5)
   {
      m_model.ctcManager().setFilterHalfWidthValue(FHWidth);
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "CTCv1 default value is not detected for FilterHalfWidth";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Filter half width value is set to the default value of 5";
   }
   else
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "CTCv1 default value is detected for FilterHalfWidth";
   }

   double temp1,temp2;
   double ULContCrstRatio=1.0, ULOceaCrstRatio = 1.0;
   m_model.ctcManager().getUpperLowerContinentalCrustRatio(temp1);
   m_model.ctcManager().getUpperLowerOceanicCrustRatio(temp2);
   if (temp1 != 1.0)
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Default value of UpperLowerContinentalCrustRatio is not detected";
      
      m_model.ctcManager().setUpperLowerContinentalCrustRatio(ULContCrstRatio);  
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Default value of 1 for UpperLowerContinentalCrustRatio is set";
   }
   else
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Default value of 1 for UpperLowerContinentalCrustRatio is found";
   }
   if (temp2 != 1.0)
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Default value of UpperLowerOceanicCrustRatio is not detected";
      
      m_model.ctcManager().setUpperLowerContinentalCrustRatio(ULOceaCrstRatio);
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Default value of 1 for UpperLowerOceanicCrustRatio is set";
   }
   else
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Default value of 1 for UpperLowerOceanicCrustRatio is found";
   }
}

