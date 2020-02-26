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
#include "MapsManager.h"

//utilities
#include "LogHandler.h"

//Prograde
#include "CtcModelConverter.h"

//cmbAPI
#include "cmbAPI.h"
#include "CtcManager.h"

//DataAccess
#include "ProjectHandle.h"
#include "RunParameters.h"

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
   Prograde::CtcModelConverter modelConverter;

   int TableSize=m_model.tableSize("CTCIoTbl");

   if (TableSize != 0)
   {
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
      
      m_model.ctcManager().setUpperLowerOceanicCrustRatio(ULOceaCrstRatio);
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Default value of 1 for UpperLowerOceanicCrustRatio is set";
   }
   else
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Default value of 1 for UpperLowerOceanicCrustRatio is found";
   }
   std::string EndRiftingMapName;
   double EndRiftingAge;
   
   m_model.ctcManager().getEndRiftingAge(EndRiftingAge);
   
   if (EndRiftingAge == -9999)
   {
      m_model.ctcManager().getEndRiftingAgeMap(EndRiftingMapName);
      mbapi::MapsManager::MapID id = m_model.mapsManager().findID(EndRiftingMapName);
      double min, max;//to store the minimum and maximum value from end rifting map
      m_model.mapsManager().mapValuesRange(id, min, max);
      EndRiftingAge = min;
   }
   std::string tectonicContext, depthMap, thicknessMap;
   double depth, thickness;

   m_model.ctcManager().getResidualDepthAnomalyScalar(depth);
   m_model.ctcManager().getResidualDepthAnomalyMap(depthMap);
   m_model.ctcManager().getBasaltMeltThicknessValue(thickness);
   m_model.ctcManager().getBasaltMeltThicknessMap(thicknessMap);

   auto timesteps = m_model.ctcManager().getStratigraphyTblLayerID();
   ///////////////////////////////////////////////////////////////////////////////////add check if ctcriftinghistoryiotbl alreasy exists; if it exists then it means that it is a bpa2 scenario then that doesnot need to be changed
  // if (m_model.tableSize("CTCRiftingHistoryIoTbl") == 0) {}
	   
   for (auto tsId : timesteps)
   {
      double age;
      m_model.addRowToTable("CTCRiftingHistoryIoTbl");
     
      //copying Age from StratIoTbl to CTCRiftingHistoryIoTbl
      m_model.ctcManager().getDepoAge(tsId, age);
      m_model.ctcManager().setCTCRiftingHistoryTblAge(tsId, age);

      m_model.ctcManager().setTectonicFlag(tsId, modelConverter.upgradeTectonicFlag(age, EndRiftingAge));
      m_model.ctcManager().getTectonicFlag(tsId, tectonicContext);
      
      if (tectonicContext == "Flexural Basin")
      {
         m_model.ctcManager().setRiftingTblResidualDepthAnomalyScalar(tsId,depth);
         m_model.ctcManager().setRiftingTblResidualDepthAnomalyMap(tsId, depthMap);
         m_model.ctcManager().setRiftingTblBasaltMeltThicknessScalar(tsId, thickness);
         m_model.ctcManager().setRiftingTblBasaltMeltThicknessMap(tsId, thicknessMap);
      }
     
   }
   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "CTCRiftingHistoryIoTbl is created";
   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Age field of CTCRiftingHistoryIoTbl is populated as per the depositional age of the StratIoTbl";
   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "TectonicFlag field of CTCRiftingHistoryIoTbl is populated for each age as per the algorithm";
   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "DeltaSL and DeltaSLGrid fields of CTCRiftingHistoryIoTbl are populated for each age";
   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "HBu and HBuGrid fields of CTCRiftingHistoryIoTbl are populated for each age";
   
   ////////////////////////////////////////////////////////////////////////////////////
   auto GridMapId = m_model.ctcManager().getGridMapID();
   std::string basaltThicknessMap,rdaMap,GridMapIoMapName,GridMapIoTblName,GridMapReferredBy;
   m_model.ctcManager().getBasaltMeltThicknessMap(basaltThicknessMap);
   m_model.ctcManager().getResidualDepthAnomalyMap(rdaMap);
   GridMapIoTblName = "CTCRiftingHistoryIoTbl";
	  for(int tsId=0; tsId < GridMapId.size(); tsId ++){
      m_model.ctcManager().getGridMapIoTblMapName(tsId, GridMapIoMapName);
      m_model.ctcManager().getGridMapTablename(tsId, GridMapReferredBy);
      if (basaltThicknessMap == GridMapIoMapName || rdaMap== GridMapIoMapName)
      {
         m_model.ctcManager().setGridMapTablename(tsId, GridMapIoTblName);
      }
      if (GridMapIoMapName != basaltThicknessMap && GridMapIoMapName != rdaMap && GridMapReferredBy == "CTCIoTbl")
      {
         m_model.removeRecordFromTable("GridMapIoTbl", tsId);
		 tsId--;
      } 
   }
   m_model.ctcManager().setBasaltMeltThicknessMap("");
   m_model.ctcManager().setResidualDepthAnomalyMap("");
   m_model.ctcManager().setEndRiftingAgeMap("");

   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "GridMapIoTbl needs to be updated";
   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Maps are referred for CTCRiftingHistoryIoTbl";
   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Maps related to deprecated fields of CTCIoTbl are removed ";

   }
   else
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "CTC is not used in the selected scenario";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "No upgradation is required related to CTC";
   }
}

