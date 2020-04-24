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
	   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "CTCv1 is detected for this scenario.";
	   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating CTCIoTbl";
	   
	   int FHWidth = 5;//Default value for FilterHalfWidth in BPA2
	   int temp;
	   //Check the default value of FilterHalfWidth which is 5 in BPA-Cauldron and 10 in BPA2-BAsin. If not 5 then set its value in CTCIoTbl to the default value of BPA-Cauldron
	   m_model.ctcManager().getFilterHalfWidthValue(temp);
	   if (temp != FHWidth){
		   m_model.ctcManager().setFilterHalfWidthValue(FHWidth);
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Setting the FilterHalfWidth value to BPA2 default value:"<< FHWidth <<" from "<< temp;
	   }
	   else{
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "BPA2 default value is detected for FilterHalfWidth. No updation needed.";
	   }
	   
	   double temp1;
	   double ULContCrstRatio = 1.0, ULOceaCrstRatio = 1.0;//Default values for BPA2
	   m_model.ctcManager().getUpperLowerContinentalCrustRatio(temp1);
	   if (temp1 != ULContCrstRatio)
	   {
		   m_model.ctcManager().setUpperLowerContinentalCrustRatio(ULContCrstRatio);  
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Default value of UpperLowerContinentalCrustRatio is not detected. Updating its value to "<< ULContCrstRatio << " from "<< temp1;
	   }
	   else{
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Default value of 1 for UpperLowerContinentalCrustRatio is found. No updation needed.";
	   }

	   m_model.ctcManager().getUpperLowerOceanicCrustRatio(temp1);
	   if (temp1 != ULOceaCrstRatio)
	   {   
		   m_model.ctcManager().setUpperLowerOceanicCrustRatio(ULOceaCrstRatio);
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Default value of UpperLowerOceanicCrustRatio is not detected. Updating its value to " << ULOceaCrstRatio << " from " << temp1;
	   }
	   else{
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Default value of 1 for UpperLowerOceanicCrustRatio is found. No updation needed.";
	   }
	   
	   std::string EndRiftingMapName;
	   double EndRiftingAge;
	   
	   m_model.ctcManager().getEndRiftingAge(EndRiftingAge);
	   
	   if (EndRiftingAge == DataAccess::Interface::DefaultUndefinedScalarValue) 
	   {
		   m_model.ctcManager().getEndRiftingAgeMap(EndRiftingMapName);
		   mbapi::MapsManager::MapID id = m_model.mapsManager().findID(EndRiftingMapName);
		   double min, max;//to store the minimum and maximum value from end rifting map
		   if (ErrorHandler::NoError != m_model.mapsManager().mapValuesRange(id, min, max))
			   throw ErrorHandler::Exception(m_model.mapsManager().errorCode()) << m_model.mapsManager().errorMessage();
		   EndRiftingAge = min;
	   } 
	  
	   std::string tectonicContext, rdaMap, MaximumThicknessOfBasaltMeltMap;
	   double depth, thickness;

	   m_model.ctcManager().getResidualDepthAnomalyScalar(depth);
	   m_model.ctcManager().getResidualDepthAnomalyMap(rdaMap);//Getting the map name for DeltaSLGrid field from CTCIoTbl
	   m_model.ctcManager().getBasaltMeltThicknessValue(thickness);
	   m_model.ctcManager().getBasaltMeltThicknessMap(MaximumThicknessOfBasaltMeltMap);//Getting the map name for HBuGrid field from CTCIoTbl

	   if (m_model.tableSize("CTCRiftingHistoryIoTbl") != 0) {
		   m_model.clearTable("CTCRiftingHistoryIoTbl");
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "CTCRiftingHistoryIoTbl is cleared";
	   }
	   
	   auto timesteps = m_model.ctcManager().getStratigraphyTblLayerID();
	   for (auto tsId : timesteps)
	   {
		   double age;
		   m_model.addRowToTable("CTCRiftingHistoryIoTbl");

		   //copying Age from StratIoTbl to CTCRiftingHistoryIoTbl
		   m_model.ctcManager().getDepoAge(tsId, age);
		   m_model.ctcManager().setCTCRiftingHistoryTblAge(tsId, age);

		   m_model.ctcManager().setTectonicFlag(tsId, modelConverter.upgradeTectonicFlag(age, EndRiftingAge));
		   m_model.ctcManager().getTectonicFlag(tsId, tectonicContext);

		   //if the "tectonicContext=Flexural Basins", then
		   //					->copy DeltaSL/DeltaSLGrid from legacy CTCIoTbl directly, 
		   //					->HBu/HBuGrid from legacy CTCIoTbl directly 
		   //if the "tectonicContext=Active Rifting or Passive Margin", then 
		   //					->DeltaSL/DeltaSLGrid is set to the default value of 0 
		   //					->HBu/HBuGrid is set to the default value of 7000
		   //The set method is not written explicitely for "Active Rifting" and "Passive Margin" case, as the defaults values are specified in the BasinModellerProperty.Spec file.
		   if (tectonicContext == "Flexural Basin")
		   {
			   m_model.ctcManager().setRiftingTblResidualDepthAnomalyScalar(tsId, depth);
			   m_model.ctcManager().setRiftingTblResidualDepthAnomalyMap(tsId, rdaMap);
			   m_model.ctcManager().setRiftingTblBasaltMeltThicknessScalar(tsId, thickness);
			   m_model.ctcManager().setRiftingTblBasaltMeltThicknessMap(tsId, MaximumThicknessOfBasaltMeltMap);
		   }
	   }
	   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "CTCRiftingHistoryIoTbl is created corresponding to each depoAge of StratIoTbl";
	   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Age field of CTCRiftingHistoryIoTbl is populated as per the depoAge of the StratIoTbl";
	   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "TectonicFlag field of CTCRiftingHistoryIoTbl is populated for each age as per the algorithm";
	   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "DeltaSL and DeltaSLGrid fields of CTCRiftingHistoryIoTbl are populated for each age";
	   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "HBu and HBuGrid fields of CTCRiftingHistoryIoTbl are populated for each age";
	   
	   auto GridMapId = m_model.ctcManager().getGridMapID();
	   std::string GridMapIoMapName,UpdatedTblNameForCTC,GridMapReferredBy;
	  
	   UpdatedTblNameForCTC = "CTCRiftingHistoryIoTbl";
	   
	   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the GridMapIotbl";
	   //Updating the table name in the GridMapIoTbl from "CTCIoTbl" to "CTCRiftingHistoryIoTbl" for HBuGrid (Maximum thickness of Basalt melt) and DeltaSLGrid (RDA map)
	   //Removing other map references from CTCIoTbl other than these two.
	   
	   for (int tsId = 0; tsId < GridMapId.size(); tsId++)
	   {
		   m_model.ctcManager().getGridMapIoTblMapName(tsId, GridMapIoMapName);
		   m_model.ctcManager().getGridMapTablename(tsId, GridMapReferredBy);

		   if (MaximumThicknessOfBasaltMeltMap == GridMapIoMapName || rdaMap == GridMapIoMapName)
		   {
			   m_model.ctcManager().setGridMapTablename(tsId, UpdatedTblNameForCTC);
			   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "The refering table for Map:" << GridMapIoMapName << " is updated to CTCRiftingHistoryIoTbl from CTCIoTbl";
		   }
		   if (GridMapIoMapName != MaximumThicknessOfBasaltMeltMap && GridMapIoMapName != rdaMap && GridMapReferredBy == "CTCIoTbl")
		   {
			   m_model.removeRecordFromTable("GridMapIoTbl", tsId);
			   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Legacy Map:" << GridMapIoMapName << " related to deprecated fields of CTCIoTbl is removed from GridMapIoTbl";
			   GridMapId = m_model.ctcManager().getGridMapID();
			   tsId--;
		   }
#if 0
		   //Remove basalt referrences from the GridMapIoTbl..This part is removed from here as it is taken care in the current implementation of ALCUpgrade manager
		   if (GridMapReferredBy == "BasaltThicknessIoTbl")
		   {
			   m_model.removeRecordFromTable("GridMapIoTbl", tsId);
			   tsId--;
		   }
#endif
	   }
	   
	   //Setting the "HBuGrid" and "DeltaSLGrid" fields of CTCIoTbl to null-string as these fields are moved to CTCRiftingHistoryIoTbl in BPA2
	   if (MaximumThicknessOfBasaltMeltMap.compare(""))
	   {
		   m_model.ctcManager().setBasaltMeltThicknessMap("");
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "The HBuGrid field of CTCIoTbl is updated to 'blank' as it is not a part of BPA2 CTCIoTbl";
	   }
	   if (rdaMap.compare(""))
	   {
		   m_model.ctcManager().setResidualDepthAnomalyMap("");
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "The DeltaSLGrid field of CTCIoTbl is updated to 'blank', as it is not a part of BPA2 CTCIoTbl";
	   }
	   if (EndRiftingMapName.compare(""))
	   {
		   m_model.setTableValue("CTCIoTbl", 0, "TRIniGrid", "");;
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "The TRIniGrid field of CTCIoTbl is updated to 'blank', as it is not a part of BPA2 CTCIoTbl";
	   }

   }
   else
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "CTC is not used in the selected scenario";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "No upgradation is required related to CTC";
   }
}

