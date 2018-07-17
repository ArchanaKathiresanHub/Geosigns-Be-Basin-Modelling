//                                                                      
// Copyright (C) 2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by CGI.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file BiodegradeManagerImpl.cpp
/// @brief This file keeps API which deal with biodegradation constants

#include "BiodegradeManagerImpl.h"

#include <stdexcept>
#include <string>


namespace mbapi
{

   const char * BiodegradeManagerImpl::s_BiodegradeTableName = "BioDegradIoTbl";

   // Constructor
   BiodegradeManagerImpl::BiodegradeManagerImpl()
   {
      //throw std::runtime_error( "Not implemented yet" );
   }

   // Copy operator
   BiodegradeManagerImpl & BiodegradeManagerImpl::operator = (const BiodegradeManagerImpl & /*otherBiodegMgr*/)
   {
      throw std::runtime_error("Not implemented yet");
      return *this;
   }

   // Set project database. Reset all
   void BiodegradeManagerImpl::setDatabase(database::ProjectFileHandlerPtr pfh)
   {
      m_db = pfh;

      m_BiodegradeIoTbl = m_db->getTable(s_BiodegradeTableName);

   }

   // Get Biodegradation constants
   ErrorHandler::ReturnCode BiodegradeManagerImpl::getBioConstant(const std::string & BioConsName, double & BioConsFromP3dFile)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_BiodegradeIoTbl) { throw Exception(NonexistingID) << s_BiodegradeTableName << " table could not be found in project"; }
         database::Record * rec = m_BiodegradeIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in Biodegradation table: "; }

         BioConsFromP3dFile = rec->getValue<double>(BioConsName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Set Biodegradation constants
   ErrorHandler::ReturnCode BiodegradeManagerImpl::setBioConstant(const std::string & BioConsName, double & BioConsFromP3dFile)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_BiodegradeIoTbl) { throw Exception(NonexistingID) << s_BiodegradeTableName << " table could not be found in project"; }
         database::Record * rec = m_BiodegradeIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in Biodegradation table: "; }

         rec->setValue<double>(BioConsName, BioConsFromP3dFile);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

}