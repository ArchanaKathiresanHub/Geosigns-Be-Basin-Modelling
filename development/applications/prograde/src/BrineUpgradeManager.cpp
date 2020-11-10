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

#include <algorithm>

using namespace mbapi;

//------------------------------------------------------------//

Prograde::BrineUpgradeManager::BrineUpgradeManager(Model& model) :
   IUpgradeManager("brine upgrade manager"), m_model(model)
{
   const auto ph = m_model.projectHandle();
   if (ph == nullptr) {
      throw std::invalid_argument(getName() + " cannot retrieve the project handle from Cauldron data model");
   }
   m_ph = ph;
}

//------------------------------------------------------------//

void Prograde::BrineUpgradeManager::upgrade() {
   Prograde::BrineModelConverter modelConverter;
   auto fluids = m_model.fluidManager().getFluidsID();
   std::vector<std::string> FluidTypesUsed = StratIoTblReferredFluids();
   std::vector<std::string> baseFluidForUserDefinedBrine;//To store the base fluid type for a userDefined brine
                                                         //std::string baseFluidForUserDefinedBrine;//To store the base fluid type for a userDefined brine
   
   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Updating FluidTypeIoTbl";
   for (auto flId : fluids) {

      std::string fluidName;

      m_model.fluidManager().getFluidName(flId, fluidName);

      int fluidUserDefined;                      //Fluid user defined flag
      m_model.fluidManager().getUserDefined(flId, fluidUserDefined);
      m_model.fluidManager().setUserDefined(flId, modelConverter.upgradeUserDefined(fluidName, fluidUserDefined));
      std::string fluidDescription;              //Fluid description
      m_model.fluidManager().getDescription(flId, fluidDescription);
      m_model.fluidManager().setDescription(flId, modelConverter.upgradeDescription(fluidName, fluidDescription));
      std::string heatCapType;              //Fluid HeatCaptype
      m_model.fluidManager().getHeatCapType(flId, heatCapType);
      m_model.fluidManager().setHeatCapType(flId, modelConverter.upgradeHeatCapType(fluidName, fluidDescription, heatCapType));
      std::string ThermCondType;              //Fluid HeatCaptype
      m_model.fluidManager().getThermCondType(flId, ThermCondType);
      m_model.fluidManager().setThermCondType(flId, modelConverter.upgradeThermCondType(fluidName, fluidDescription, ThermCondType));
      std::string OriginalDefinedBy;
      m_model.fluidManager().getDefinedBy(flId, OriginalDefinedBy);
      m_model.fluidManager().setDefinedBy(flId, modelConverter.upgradeDefinedBy(fluidName, OriginalDefinedBy));//Updating the definedBy field for each fluidName

      FluidManager::FluidDensityModel densModel; // Fluid density calculation model
      double refDens;
      m_model.fluidManager().densityModel(flId, densModel, refDens);
      double updatedRefDens = -9999;
      m_model.fluidManager().setDensityModel(flId, modelConverter.upgradeDensityModel(densModel, fluidName), updatedRefDens);
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> " << fluidName << ", Reference Density value is set to "<< updatedRefDens <<" (Undefined Value) from "<< refDens<<" as the model is updated to Calculated";

      FluidManager::CalculationModel seisVelModel; // Seismic velocity calculation model
      double refSeisVel;
      m_model.fluidManager().seismicVelocityModel(flId, seisVelModel, refSeisVel);
      double updatedRefSeisVel = -9999;
      m_model.fluidManager().setSeismicVelocityModel(flId, modelConverter.upgradeSeismicVelocityModel(seisVelModel, fluidName), updatedRefSeisVel);
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> " << fluidName << ", Reference SeisVelocity value is set to "<< updatedRefSeisVel<<" (Undefined Value) from " << refSeisVel << " as the model is updated to Calculated";

      m_model.fluidManager().getUserDefined(flId, fluidUserDefined);
      if (fluidUserDefined == 1)
      {
         for (size_t size = 0; size < (FluidTypesUsed.size() - 1); size++)
         {
            if (FluidTypesUsed[size] == fluidName)
            {

               m_model.fluidManager().getHeatCapType(flId, heatCapType);
               baseFluidForUserDefinedBrine.push_back(heatCapType);
            }
         }

      }
   }
   size_t countOfNotMatchingflId = 0;
   bool isMatch = false;
   int originalRowPosition = -1;
   int nextFluidTypeIOTbl_Index = -1;

   for (auto flId : fluids) {

      std::string fluidName;
      originalRowPosition++; // Original location of row entry to be checked
      nextFluidTypeIOTbl_Index++;

      m_model.fluidManager().getFluidName(nextFluidTypeIOTbl_Index, fluidName);
      size_t size = FluidTypesUsed.size() - 1;//size of StratIoTbl neglecting the basement entry
      size_t size_baseFluidForUserDefinedBrine = baseFluidForUserDefinedBrine.size();

      for (size_t iteratorFluidTypeUsed = 0; iteratorFluidTypeUsed < size; iteratorFluidTypeUsed++)
      {
         if (size_baseFluidForUserDefinedBrine > 0)
         {
            for (size_t iteratorBaseFluidForUserDefinedBrine = 0; iteratorBaseFluidForUserDefinedBrine < size_baseFluidForUserDefinedBrine; iteratorBaseFluidForUserDefinedBrine++)
            {
               std::string xyz = FluidTypesUsed[iteratorFluidTypeUsed];
               if (FluidTypesUsed[iteratorFluidTypeUsed] == fluidName || fluidName == baseFluidForUserDefinedBrine[iteratorBaseFluidForUserDefinedBrine])
               {
                  isMatch = true;
                  break;
               }
            }
         }
         else if (size_baseFluidForUserDefinedBrine == 0)
         {
            if (FluidTypesUsed[iteratorFluidTypeUsed] == fluidName)
            {
               isMatch = true;
               break;
            }
         }
      }
      if (isMatch)
      {
         // Don't remove from table
         isMatch = false;
      }
      else
      {
         countOfNotMatchingflId++;
         // remove record from table if not match
         m_model.removeRecordFromTable("FluidtypeIoTbl", (originalRowPosition + 1) - countOfNotMatchingflId);
		 LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> " << fluidName << " is not referred in StratIoTbl for the scenario; the row from FluidTypeIoTbl is deleted";
         nextFluidTypeIOTbl_Index--;
      }
   }
   this->ResetFltThCondIoTbl();
   this->ResetFltHeatCapIoTbl();

}

std::vector<std::string> Prograde::BrineUpgradeManager::StratIoTblReferredFluids()
{
   auto layerId = m_model.fluidManager().getLayerID();
   std::string FluidName;
   std::vector<std::string> fluids;//vector to store the fluid types specified in the StratIoTbl corresponding to each layer
   for (auto tsId : layerId)
   {
      m_model.fluidManager().getBrineType(tsId, FluidName);
      fluids.push_back(FluidName);
   }
   //Removing the fluid types from the vector fluids which are repeated
   auto end = fluids.end();
   for (auto i = fluids.begin(); i != end; i++)
   {
      end = std::remove(i + 1, end, *i);
   }
   fluids.erase(end, fluids.end());

   return fluids;
}

std::vector<std::string> Prograde::BrineUpgradeManager::ThermCondtypeReferred()
{
   auto fluidsId = m_model.fluidManager().getFluidsID();
   std::string ThCondType;
   std::vector<std::string> thCondType;//vector to store thermConductivity types specified for each id of FluidtypeIoTbl
   for (auto tsId : fluidsId)
   {
      m_model.fluidManager().getThermCondType(tsId, ThCondType);
      thCondType.push_back(ThCondType);
   }
   //Removing the thermConductivity types from the vector thCondType which are repeated
   auto end = thCondType.end();
   for (auto i = thCondType.begin(); i != end; i++)
   {
      end = std::remove(i + 1, end, *i);
   }
   thCondType.erase(end, thCondType.end());

   return thCondType;
}

void Prograde::BrineUpgradeManager::ResetFltThCondIoTbl()
{
   auto ThCondId = m_model.fluidManager().getFluidThCondID();
   double TemperatureValue, PressVal, ThCondVal;
   std::vector<double> TempIndexValuesAll, PressureValAll, ThCondValAll;
   for (auto flId : ThCondId) {
      m_model.fluidManager().getThermalCondTblTempIndex(flId, TemperatureValue);
      TempIndexValuesAll.push_back(TemperatureValue);
      m_model.fluidManager().getThermalCondTblPressure(flId, PressVal);
      PressureValAll.push_back(PressVal);
      m_model.fluidManager().getThermalCond(flId, ThCondVal);
      ThCondValAll.push_back(ThCondVal);
   }
   m_model.clearTable("FltThCondIoTbl");
   std::vector<std::string> TherCondTypeUsed = ThermCondtypeReferred();

   for (size_t ThCondType = 0; ThCondType < TherCondTypeUsed.size(); ThCondType++)
   {
      for (auto flId : ThCondId)
      {
         m_model.addRowToTable("FltThCondIoTbl");
      }
   }

   auto ThCondIdUpdated = m_model.fluidManager().getFluidThCondID();
   size_t thCondIdCount = 0;

   for (size_t ThCondType = 0; ThCondType < TherCondTypeUsed.size(); ThCondType++)
   {
      int count = -1;
      for (size_t id = 0; id < ThCondId.size(); id++)
      {
         count++;

         m_model.setTableValue("FltThCondIoTbl", thCondIdCount, "Fluidtype", TherCondTypeUsed[ThCondType]);
         m_model.setTableValue("FltThCondIoTbl", thCondIdCount, "TempIndex", TempIndexValuesAll[count]);
         m_model.setTableValue("FltThCondIoTbl", thCondIdCount, "Pressure", PressureValAll[count]);
         m_model.setTableValue("FltThCondIoTbl", thCondIdCount, "ThCond", ThCondValAll[count]);

         thCondIdCount++;
      }
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> FltThCondIoTbl has been updated to include the property values for " << TherCondTypeUsed[ThCondType];
   }

}

void Prograde::BrineUpgradeManager::ResetFltHeatCapIoTbl()
{
   auto HeatCapId = m_model.fluidManager().getFluidHeatCapID();
   double HCTemperatureVal, HCPressVal, HeatCapVal;
   std::vector<double> HCTempIndexValuesAll, HCPressureValAll, HeatCapValAll;
   for (auto flId : HeatCapId) {
      m_model.fluidManager().getHeatCapTblTempIndex(flId, HCTemperatureVal);
      HCTempIndexValuesAll.push_back(HCTemperatureVal);
      m_model.fluidManager().getHeatCapTblPressure(flId, HCPressVal);
      HCPressureValAll.push_back(HCPressVal);
      m_model.fluidManager().getHeatCap(flId, HeatCapVal);
      HeatCapValAll.push_back(HeatCapVal);

   }
   m_model.clearTable("FltHeatCapIoTbl");
   std::vector<std::string> HeatCapTypeUsed = ThermCondtypeReferred();//HeatCapType and ThCondType values are same

   for (size_t ThCondType = 0; ThCondType < HeatCapTypeUsed.size(); ThCondType++)
   {
      for (auto flId : HeatCapId)
      {
         m_model.addRowToTable("FltHeatCapIoTbl");
      }
   }
   size_t heatCapIdCount = 0;

   for (size_t HeatCapType = 0; HeatCapType < HeatCapTypeUsed.size(); HeatCapType++)
   {
      int rowCount = -1;
      for (size_t id = 0; id < HeatCapId.size(); id++)
      {
         rowCount++;
         m_model.setTableValue("FltHeatCapIoTbl", heatCapIdCount, "Fluidtype", HeatCapTypeUsed[HeatCapType]);
         m_model.setTableValue("FltHeatCapIoTbl", heatCapIdCount, "TempIndex", HCTempIndexValuesAll[rowCount]);
         m_model.setTableValue("FltHeatCapIoTbl", heatCapIdCount, "Pressure", HCPressureValAll[rowCount]);
         m_model.setTableValue("FltHeatCapIoTbl", heatCapIdCount, "HeatCapacity", HeatCapValAll[rowCount]);

         heatCapIdCount++;
      }
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> FltHeatCapIoTbl has been updated to include the property values for " << HeatCapTypeUsed[HeatCapType];
   }
}
