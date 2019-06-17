//                                                                      
// Copyright (C) 2012-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file FluidManagerImpl.C
/// @brief This file keeps API implementation for manipulating fluids in Cauldron model

#include "FluidManagerImpl.h"
#include <stdexcept>
#include <string>

namespace mbapi
{

   const char * FluidManagerImpl::s_fluidTypesTableName = "FluidtypeIoTbl";
   const char * FluidManagerImpl::s_fluidTypeFieldName = "Fluidtype";
   const char * FluidManagerImpl::s_descriptionFieldName = "Description";
   const char * FluidManagerImpl::s_userDefinedFieldName = "UserDefined";
   const char * FluidManagerImpl::s_fluidIoTblHeatCapTypeFieldName = "HeatCaptype";
   const char * FluidManagerImpl::s_fluidIoTblThermCondTypeFieldName = "ThermCondtype";
   const char * FluidManagerImpl::s_definedByFieldName = "DefinedBy";
   // Density model
   const char * FluidManagerImpl::s_densityModelFieldName = "DensModel";
   const char * FluidManagerImpl::s_densityFieldName = "Density";
   // Seismic Velocity model
   const char * FluidManagerImpl::s_seismicVelocityModelFieldName = "SeisVelocityModel";
   const char * FluidManagerImpl::s_seismicVelocityFieldName = "SeisVelocity";

   //stratIoTbl 
   const char * FluidManagerImpl::s_startLayersTableName = "StratIoTbl";
   const char * FluidManagerImpl::s_stratIoTblFluidTypeFieldName = "Fluidtype";

   //FltThCondIoTbl 
   const char * FluidManagerImpl::s_fltThCondTableName = "FltThCondIoTbl";
   const char * FluidManagerImpl::s_fltThCondTableTempIndexFieldName = "TempIndex";
   const char * FluidManagerImpl::s_fltThCondTablePressureFieldName = "Pressure";
   const char * FluidManagerImpl::s_fltThCondFieldName = "ThCond";

   //FltHeatCapIoTbl
   const char * FluidManagerImpl::s_fltHeatCapTableName = "FltHeatCapIoTbl";
   const char * FluidManagerImpl::s_fltHeatCapTableTempIndexFieldName = "TempIndex";
   const char * FluidManagerImpl::s_fltHeatCapTablePressureFieldName = "Pressure";
   const char * FluidManagerImpl::s_fltHeatCapFieldName = "HeatCapacity";

// Constructor
FluidManagerImpl::FluidManagerImpl()
{
   m_db = nullptr;
   m_fluidIoTbl = nullptr;
   m_stratIoTbl = nullptr;
   m_fltThCondIoTbl = nullptr;
   m_fltHeatCapIoTbl = nullptr;
}

// Copy operator
FluidManagerImpl & FluidManagerImpl::operator = ( const FluidManagerImpl & /*otherFluidMgr*/ )
{
   throw std::runtime_error( "Not implemented yet" );
   return *this;
}

// Set project database. Reset all
void FluidManagerImpl::setDatabase(database::ProjectFileHandlerPtr pfh)
{
   m_db = pfh;

   m_fluidIoTbl = m_db->getTable(s_fluidTypesTableName);
   m_stratIoTbl = m_db->getTable(s_startLayersTableName);
   m_fltThCondIoTbl = m_db->getTable(s_fltThCondTableName);
   m_fltHeatCapIoTbl = m_db->getTable(s_fltHeatCapTableName);

}

// Get list of fluids in the model
// return array with IDs of different fluids defined in the model
std::vector<FluidManager::FluidID> FluidManagerImpl::getFluidsID() const
{
   std::vector<FluidID> flIDs; // if m_lithIoTbl does not exist - return empty array
   if (m_fluidIoTbl)
   {
      // fill IDs array with increasing indexes
      flIDs.resize(m_fluidIoTbl->size(), 0);
      for (size_t i = 0; i < flIDs.size(); ++i) flIDs[i] = static_cast<FluidID>(i);
   }

   return flIDs;
}

// Create new fluid
// return ID of the new Fluid
FluidManager::FluidID FluidManagerImpl::createNewFluid()
{
   throw std::runtime_error( "Not implemented yet" );
}

// Get fluid name for
// [in] id fluid ID
// [out] fluidName on succes has a fluid name, or empty string otherwise
// return NoError on success or NonexistingID on error
ErrorHandler::ReturnCode FluidManagerImpl::getFluidName( FluidID id, std::string & fluidName )
{
   if (errorCode() != NoError) resetError();

   try
   {
      if (!m_fluidIoTbl) { throw Exception(NonexistingID) << s_fluidTypesTableName << " table could not be found in project"; }
      database::Record * rec = m_fluidIoTbl->getRecord(static_cast<int>(id));
      if (!rec) { throw Exception(NonexistingID) << "No fluid type with such ID: " << id; }
      fluidName = rec->getValue<std::string>(s_fluidTypeFieldName);
   }
   catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
   return NoError;
}

// Get Description for fluid type
ErrorHandler::ReturnCode FluidManagerImpl::getDescription(const FluidID id, std::string & myDescription)
{
   if (errorCode() != NoError) resetError();

   try
   {
      if (!m_fluidIoTbl) { throw Exception(NonexistingID) << s_fluidTypesTableName << " table could not be found in project"; }
      database::Record * rec = m_fluidIoTbl->getRecord(static_cast<int>(id));
      if (!rec) { throw Exception(NonexistingID) << "No fluid type with such ID: " << id; }
      myDescription = rec->getValue<std::string>(s_descriptionFieldName);
   }
   catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
   return NoError;
}

// Set Description for fluid type
ErrorHandler::ReturnCode FluidManagerImpl::setDescription(const FluidID id, const std::string & myDescription)
{
   if (errorCode() != NoError) resetError();

   try
   {
      if (!m_fluidIoTbl) { throw Exception(NonexistingID) << s_fluidTypesTableName << " table could not be found in project"; }
      database::Record * rec = m_fluidIoTbl->getRecord(static_cast<int>(id));
      if (!rec) { throw Exception(NonexistingID) << "No fluid type with such ID: " << id; }
      rec->setValue<std::string>(s_descriptionFieldName, myDescription);
   }
   catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
   return NoError;
}

// Get User Defined flag
ErrorHandler::ReturnCode FluidManagerImpl::getUserDefined(const FluidID id, int & myUserDefined)
{
   if (errorCode() != NoError) resetError();

   try
   {
      if (!m_fluidIoTbl) { throw Exception(NonexistingID) << s_fluidTypesTableName << " table could not be found in project"; }
      database::Record * rec = m_fluidIoTbl->getRecord(static_cast<int>(id));
      if (!rec) { throw Exception(NonexistingID) << "No fluid type with such ID: " << id; }
      myUserDefined = rec->getValue<int>(s_userDefinedFieldName);
   }
   catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
   return NoError;
}

// Set User Defined flag
ErrorHandler::ReturnCode FluidManagerImpl::setUserDefined(const FluidID id, const int myUserDefined)
{
   if (errorCode() != NoError) resetError();

   try
   {
      if (!m_fluidIoTbl) { throw Exception(NonexistingID) << s_fluidTypesTableName << " table could not be found in project"; }
      database::Record * rec = m_fluidIoTbl->getRecord(static_cast<int>(id));
      if (!rec) { throw Exception(NonexistingID) << "No fluid type with such ID: " << id; }
      rec->setValue<int>(s_userDefinedFieldName, myUserDefined);
   }
   catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
   return NoError;
}

// Get fluid density model
ErrorHandler::ReturnCode FluidManagerImpl::densityModel(const FluidID id, FluidDensityModel & densModel, double & refDens)
{
   if (errorCode() != NoError) resetError();
   try
   {

      // if table does not exist - report error
      if (!m_fluidIoTbl) { throw Exception(NonexistingID) << s_fluidTypesTableName << " table could not be found in project"; }

      database::Record * rec = m_fluidIoTbl->getRecord(static_cast<int>(id));
      if (!rec) { throw Exception(NonexistingID) << "No fluid type with such ID: " << id; }

      const std::string & densModelName = rec->getValue<std::string>(s_densityModelFieldName);
      if (densModelName == "User Defined") densModel = Constant;
      else if (densModelName == "Calculated") densModel = Calculated;
      else { throw Exception(UndefinedValue) << "Unknown density model:" << densModelName; }
      refDens = rec->getValue<double>(s_densityFieldName);
   }
   catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

   return NoError;
}

// Set fluid density model
ErrorHandler::ReturnCode FluidManagerImpl::setDensityModel(const FluidID id, const FluidDensityModel densModel, const double refDens)
{
   if (errorCode() != NoError) resetError();
   try
   {
      // if table does not exist - report error
      if (!m_fluidIoTbl) { throw Exception(NonexistingID) << s_fluidTypesTableName << " table could not be found in project"; }

      database::Record * rec = m_fluidIoTbl->getRecord(static_cast<int>(id));
      if (!rec) { throw Exception(NonexistingID) << "No fluid type with such ID: " << id; }

      switch (densModel)
      {
      case Constant:
         rec->setValue<std::string>(s_densityModelFieldName, "User Defined");
         rec->setValue<double>(s_densityFieldName, refDens);
         break;

      case Calculated:
         rec->setValue<std::string>(s_densityModelFieldName, "Calculated");
         rec->setValue<double>(s_densityFieldName, refDens);
         break;

      default: throw Exception(UndefinedValue) << "Unknown density model:" << densModel;
      }
   }
   catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

   return NoError;
}

// Get fluid seismic velocity model
ErrorHandler::ReturnCode FluidManagerImpl::seismicVelocityModel(const FluidID id, CalculationModel & seisVelModel, double & refSeisVel)
{
   if (errorCode() != NoError) resetError();
   try
   {

      // if table does not exist - report error
      if (!m_fluidIoTbl) { throw Exception(NonexistingID) << s_fluidTypesTableName << " table could not be found in project"; }

      database::Record * rec = m_fluidIoTbl->getRecord(static_cast<int>(id));
      if (!rec) { throw Exception(NonexistingID) << "No fluid type with such ID: " << id; }

      const std::string & seisVelModelName = rec->getValue<std::string>(s_seismicVelocityModelFieldName);
      if (seisVelModelName == "User Defined") seisVelModel = ConstantModel;
      else if (seisVelModelName == "Calculated") seisVelModel = CalculatedModel;
      else { throw Exception(UndefinedValue) << "Unknown seismic velocity model:" << seisVelModelName; }
      refSeisVel = rec->getValue<double>(s_seismicVelocityFieldName);
   }
   catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

   return NoError;
}

// Set fluid density model
ErrorHandler::ReturnCode FluidManagerImpl::setSeismicVelocityModel(const FluidID id, const CalculationModel seisVelModel, const double refSeisVel)
{
   if (errorCode() != NoError) resetError();
   try
   {
      // if table does not exist - report error
      if (!m_fluidIoTbl) { throw Exception(NonexistingID) << s_fluidTypesTableName << " table could not be found in project"; }

      database::Record * rec = m_fluidIoTbl->getRecord(static_cast<int>(id));
      if (!rec) { throw Exception(NonexistingID) << "No fluid type with such ID: " << id; }

      switch (seisVelModel)
      {
      case ConstantModel:
         rec->setValue<std::string>(s_seismicVelocityModelFieldName, "User Defined");
         rec->setValue<double>(s_seismicVelocityFieldName, refSeisVel);
         break;

      case CalculatedModel:
         rec->setValue<std::string>(s_seismicVelocityModelFieldName, "Calculated");
         rec->setValue<double>(s_seismicVelocityFieldName, refSeisVel);
         break;

      default: throw Exception(UndefinedValue) << "Unknown seismic velocity model:" << seisVelModel;
      }
   }
   catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

   return NoError;
}

// Get list of stratigraphic layers defined in the StratIoTbl
// return array with IDs of different layers in the model
std::vector<FluidManager::FluidID> FluidManagerImpl::getLayerID() const
{
   std::vector<LayerID> layerIDs; 
   if (m_stratIoTbl)
   {
      // fill IDs array with increasing indexes
      layerIDs.resize(m_stratIoTbl->size(), 0);
      for (size_t i = 0; i < layerIDs.size(); ++i) layerIDs[i] = static_cast<LayerID>(i);
   }

   return layerIDs;
}
// Get brine type for a layer ID defined in the StratIoTbl
ErrorHandler::ReturnCode FluidManagerImpl::getBrineType(LayerID id, std::string & brine) 
{
   if (errorCode() != NoError) resetError();
   try
   {
      if (!m_stratIoTbl) { throw Exception(NonexistingID) << s_startLayersTableName << " table could not be found in project"; }
      database::Record * rec = m_stratIoTbl->getRecord(static_cast<int>(id));
      if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
      brine = rec->getValue<std::string>(s_stratIoTblFluidTypeFieldName);
   }
   catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
   return NoError;
}
// Get heat capacity type specified in the FluidTypeIoTbl for a particular fluid id
ErrorHandler::ReturnCode FluidManagerImpl::getHeatCapType(FluidID id, std::string & HeatCapType)
{
   if (errorCode() != NoError) resetError();
   try
   {
      if (!m_fluidIoTbl) { throw Exception(NonexistingID) << s_fluidTypesTableName << " table could not be found in project"; }
      database::Record * rec = m_fluidIoTbl->getRecord(static_cast<int>(id));
      if (!rec) { throw Exception(NonexistingID) << "No fluid type with such ID: " << id; }
      HeatCapType = rec->getValue<std::string>(s_fluidIoTblHeatCapTypeFieldName);
   }
   catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
   return NoError;
}

// Set HeatCaptype for a particular fluid id
ErrorHandler::ReturnCode FluidManagerImpl::setHeatCapType(FluidID id, const std::string & HeatCapType)
{
   if (errorCode() != NoError) resetError();

   try
   {
      if (!m_fluidIoTbl) { throw Exception(NonexistingID) << s_fluidTypesTableName << " table could not be found in project"; }
      database::Record * rec = m_fluidIoTbl->getRecord(static_cast<int>(id));
      if (!rec) { throw Exception(NonexistingID) << "No fluid type with such ID: " << id; }
      rec->setValue<std::string>(s_fluidIoTblHeatCapTypeFieldName, HeatCapType);
   }
   catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
   return NoError;
}

// Get thermal conductivity type specified in the FluidTypeIoTbl for a particular fluid id
ErrorHandler::ReturnCode FluidManagerImpl::getThermCondType(FluidID id, std::string & ThermCondType)
{
   if (errorCode() != NoError) resetError();
   try
   {
      if (!m_fluidIoTbl) { throw Exception(NonexistingID) << s_fluidTypesTableName << " table could not be found in project"; }
      database::Record * rec = m_fluidIoTbl->getRecord(static_cast<int>(id));
      if (!rec) { throw Exception(NonexistingID) << "No fluid type with such ID: " << id; }
      ThermCondType = rec->getValue<std::string>(s_fluidIoTblThermCondTypeFieldName);
   }
   catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
   return NoError;
}

// Set thermal conductivity for a particular fluid id
ErrorHandler::ReturnCode FluidManagerImpl::setThermCondType(FluidID id, const std::string & ThermCondType)
{
   if (errorCode() != NoError) resetError();

   try
   {
      if (!m_fluidIoTbl) { throw Exception(NonexistingID) << s_fluidTypesTableName << " table could not be found in project"; }
      database::Record * rec = m_fluidIoTbl->getRecord(static_cast<int>(id));
      if (!rec) { throw Exception(NonexistingID) << "No fluid type with such ID: " << id; }
      rec->setValue<std::string>(s_fluidIoTblThermCondTypeFieldName, ThermCondType);
   }
   catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
   return NoError;
}
/// Get list of ids from FltThCondIoTbl
std::vector<FluidManager::FluidID> FluidManagerImpl::getFluidThCondID() const
{
   std::vector<FluidThCondID> thCondIDs;
   if (m_fltThCondIoTbl)
   {
      // fill IDs array with increasing indexes
      thCondIDs.resize(m_fltThCondIoTbl->size(), 0);
      for (size_t i = 0; i < thCondIDs.size(); ++i) thCondIDs[i] = static_cast<FluidThCondID>(i);
   }

   return thCondIDs;
}
// Get temperature index specified in the FltThCondIoTbl for a particular id
ErrorHandler::ReturnCode FluidManagerImpl::getThermalCondTblTempIndex(FluidThCondID id, double & Temperature)
{
   if (errorCode() != NoError) resetError();
   try
   {
      if (!m_fltThCondIoTbl) { throw Exception(NonexistingID) << s_fltThCondTableName << " table could not be found in project"; }
      database::Record * rec = m_fltThCondIoTbl->getRecord(static_cast<int>(id));
      if (!rec) { throw Exception(NonexistingID) << "No fluid type with such ID: " << id; }
      Temperature = rec->getValue<double>(s_fltThCondTableTempIndexFieldName);
   }
   catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
   return NoError;
}

// Set temperature index specified in the FltThCondIoTbl for a particular id
ErrorHandler::ReturnCode FluidManagerImpl::setThermalCondTblTempIndex(FluidThCondID id, const double & Temperature)
{
   if (errorCode() != NoError) resetError();

   try
   {
      if (!m_fltThCondIoTbl) { throw Exception(NonexistingID) << s_fltThCondTableName << " table could not be found in project"; }
      database::Record * rec = m_fltThCondIoTbl->getRecord(static_cast<int>(id));
      if (!rec) { throw Exception(NonexistingID) << "No fluid type with such ID: " << id; }
      rec->setValue<double>(s_fltThCondTableTempIndexFieldName, Temperature);
   }
   catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
   return NoError;
}
// Get Pressure field value of FltThCondIoTbl for a particular id
ErrorHandler::ReturnCode FluidManagerImpl::getThermalCondTblPressure(FluidThCondID id, double & Pressure)
{
   if (errorCode() != NoError) resetError();
   try
   {
      if (!m_fltThCondIoTbl) { throw Exception(NonexistingID) << s_fltThCondTableName << " table could not be found in project"; }
      database::Record * rec = m_fltThCondIoTbl->getRecord(static_cast<int>(id));
      if (!rec) { throw Exception(NonexistingID) << "No fluid type with such ID: " << id; }
      Pressure = rec->getValue<double>(s_fltThCondTablePressureFieldName);
   }
   catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
   return NoError;
}
// Set temperature index specified in the FltThCondIoTbl for a particular id
ErrorHandler::ReturnCode FluidManagerImpl::setThermalCondTblPressure(FluidThCondID id, const double & Pressure)
{
   if (errorCode() != NoError) resetError();

   try
   {
      if (!m_fltThCondIoTbl) { throw Exception(NonexistingID) << s_fltThCondTableName << " table could not be found in project"; }
      database::Record * rec = m_fltThCondIoTbl->getRecord(static_cast<int>(id));
      if (!rec) { throw Exception(NonexistingID) << "No fluid type with such ID: " << id; }
      rec->setValue<double>(s_fltThCondTablePressureFieldName, Pressure);
   }
   catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
   return NoError;
}
// Get ThCond field value of FltThCondIoTbl for a particular id
ErrorHandler::ReturnCode FluidManagerImpl::getThermalCond(FluidThCondID id, double & ThCond)
{
   if (errorCode() != NoError) resetError();
   try
   {
      if (!m_fltThCondIoTbl) { throw Exception(NonexistingID) << s_fltThCondTableName << " table could not be found in project"; }
      database::Record * rec = m_fltThCondIoTbl->getRecord(static_cast<int>(id));
      if (!rec) { throw Exception(NonexistingID) << "No fluid type with such ID: " << id; }
      ThCond = rec->getValue<double>(s_fltThCondFieldName);
   }
   catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
   return NoError;
}
/// Get list of ids from FltHeatCapIoTbl
std::vector<FluidManager::FluidID> FluidManagerImpl::getFluidHeatCapID() const
{
   std::vector<FluidHeatCapID> heatCapIDs;
   if (m_fltHeatCapIoTbl)
   {
      // fill IDs array with increasing indexes
      heatCapIDs.resize(m_fltHeatCapIoTbl->size(), 0);
      for (size_t i = 0; i < heatCapIDs.size(); ++i) heatCapIDs[i] = static_cast<FluidHeatCapID>(i);
   }

   return heatCapIDs;
}
// Get temperature index specified in the FltHeatCapIoTbl for a particular id
ErrorHandler::ReturnCode FluidManagerImpl::getHeatCapTblTempIndex(FluidHeatCapID id, double & Temperature)
{
   if (errorCode() != NoError) resetError();
   try
   {
      if (!m_fltHeatCapIoTbl) { throw Exception(NonexistingID) << s_fltHeatCapTableName << " table could not be found in project"; }
      database::Record * rec = m_fltHeatCapIoTbl->getRecord(static_cast<int>(id));
      if (!rec) { throw Exception(NonexistingID) << "No fluid type with such ID: " << id; }
      Temperature = rec->getValue<double>(s_fltHeatCapTableTempIndexFieldName);
   }
   catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
   return NoError;
}
// Get pressure specified in the FltHeatCapIoTbl for a particular id
ErrorHandler::ReturnCode FluidManagerImpl::getHeatCapTblPressure(FluidHeatCapID id, double & Pressure)
{
   if (errorCode() != NoError) resetError();
   try
   {
      if (!m_fltHeatCapIoTbl) { throw Exception(NonexistingID) << s_fltHeatCapTableName << " table could not be found in project"; }
      database::Record * rec = m_fltHeatCapIoTbl->getRecord(static_cast<int>(id));
      if (!rec) { throw Exception(NonexistingID) << "No fluid type with such ID: " << id; }
      Pressure = rec->getValue<double>(s_fltHeatCapTablePressureFieldName);
   }
   catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
   return NoError;
}
// Get HeatCapacity specified in the FltHeatCapIoTbl for a particular id
ErrorHandler::ReturnCode FluidManagerImpl::getHeatCap(FluidHeatCapID id, double & HeatCap)
{
   if (errorCode() != NoError) resetError();
   try
   {
      if (!m_fltHeatCapIoTbl) { throw Exception(NonexistingID) << s_fltHeatCapTableName << " table could not be found in project"; }
      database::Record * rec = m_fltHeatCapIoTbl->getRecord(static_cast<int>(id));
      if (!rec) { throw Exception(NonexistingID) << "No fluid type with such ID: " << id; }
      HeatCap = rec->getValue<double>(s_fltHeatCapFieldName);
   }
   catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
   return NoError;
}

// Get brine type (system or userDefined) for a particular fluid id of the FluidTypeIoTbl
ErrorHandler::ReturnCode FluidManagerImpl::getDefinedBy(FluidID id, std::string & DefinedBy)
{
   if (errorCode() != NoError) resetError();
   try
   {
      if (!m_fluidIoTbl) { throw Exception(NonexistingID) << s_fluidTypesTableName << " table could not be found in project"; }
      database::Record * rec = m_fluidIoTbl->getRecord(static_cast<int>(id));
      if (!rec) { throw Exception(NonexistingID) << "No fluid type with such ID: " << id; }
      DefinedBy = rec->getValue<std::string>(s_definedByFieldName);
   }
   catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
   return NoError;
}

// Set brine type (system or userDefined) for a particular fluid id of the FluidTypeIoTb
ErrorHandler::ReturnCode FluidManagerImpl::setDefinedBy(FluidID id, const std::string & DefinedBy)
{
   if (errorCode() != NoError) resetError();

   try
   {
      if (!m_fluidIoTbl) { throw Exception(NonexistingID) << s_fluidTypesTableName << " table could not be found in project"; }
      database::Record * rec = m_fluidIoTbl->getRecord(static_cast<int>(id));
      if (!rec) { throw Exception(NonexistingID) << "No fluid type with such ID: " << id; }
      rec->setValue<std::string>(s_definedByFieldName, DefinedBy);
   }
   catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
   return NoError;
}


}