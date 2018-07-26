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
   // Density model
   const char * FluidManagerImpl::s_densityModelFieldName = "DensModel";
   const char * FluidManagerImpl::s_densityFieldName = "Density";
   // Seismic Velocity model
   const char * FluidManagerImpl::s_seismicVelocityModelFieldName = "SeisVelocityModel";
   const char * FluidManagerImpl::s_seismicVelocityFieldName = "SeisVelocity";

// Constructor
FluidManagerImpl::FluidManagerImpl()
{
   m_db = nullptr;
   m_fluidIoTbl = nullptr;
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
}