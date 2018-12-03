//                                                                      
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by CGI.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file CtcManager.h
/// @brief This file keeps API declaration for manipulating CTC inputs in Cauldron model

#include "CtcManagerImpl.h"
#include <stdexcept>
#include <string>

namespace mbapi
{
   const char * CtcManagerImpl::s_ctcTableName = "CTCIoTbl"; 
   const char * CtcManagerImpl::s_FilterHalfWidthFieldName = "FilterHalfWidth";
   const char * CtcManagerImpl::s_ULContCrustRatioFieldName = "UpperLowerContinentalCrustRatio";
   const char * CtcManagerImpl::s_ULOceaCrustRatioFieldName = "UpperLowerOceanicCrustRatio";
   
   // Constructor
   CtcManagerImpl::CtcManagerImpl()
   {
      m_db = nullptr;
      m_ctcIoTbl = nullptr;
   }

   // Copy operator
   CtcManagerImpl & CtcManagerImpl::operator = (const CtcManagerImpl &)
   {
      throw std::runtime_error("Not implemented yet");
      return *this;
   }

   // Set project database. Reset all
   void CtcManagerImpl::setDatabase(database::ProjectFileHandlerPtr pfh)
   {
      m_db = pfh;
      m_ctcIoTbl = m_db->getTable(s_ctcTableName);
      
   }
   
   // Get FilterHalfWidth value
   ErrorHandler::ReturnCode CtcManagerImpl::getFilterHalfWidthValue( int & FiltrHalfWidth)
   {
      if (errorCode() != NoError) resetError(); 
      try
      {
         if (!m_ctcIoTbl) { throw Exception(NonexistingID) << s_ctcTableName << " table could not be found in project"; }
         database::Record * rec = m_ctcIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in CTC Io table: "; }
         FiltrHalfWidth = rec->getValue<int>(s_FilterHalfWidthFieldName);
      }
      
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Set FilterHalfWidth value
   ErrorHandler::ReturnCode CtcManagerImpl::setFilterHalfWidthValue(int & FiltrHalfWidth)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_ctcIoTbl) { throw Exception(NonexistingID) << s_ctcTableName << " table could not be found in project"; }
         database::Record * rec = m_ctcIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in CTC Io table: "; }
         rec->setValue<int>(s_FilterHalfWidthFieldName, FiltrHalfWidth);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
  
   // Get Upper Lower Continental Crust Ratio value
   ErrorHandler::ReturnCode CtcManagerImpl::getUpperLowerContinentalCrustRatio(double & ULContCrustRatio)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         if (!m_ctcIoTbl) { throw Exception(NonexistingID) << s_ctcTableName << " table could not be found in project"; }
         database::Record * rec = m_ctcIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in CTC Io table: "; }
         ULContCrustRatio = rec->getValue<double>(s_ULContCrustRatioFieldName);
      }

      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Set Upper Lower Continental Crust Ratio value
   ErrorHandler::ReturnCode CtcManagerImpl::setUpperLowerContinentalCrustRatio(double & ULContCrustRatio)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_ctcIoTbl) { throw Exception(NonexistingID) << s_ctcTableName << " table could not be found in project"; }
         database::Record * rec = m_ctcIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in CTC Io table: "; }
         rec->setValue<double>(s_ULContCrustRatioFieldName, ULContCrustRatio);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Get Upper Lower Oceanic Crust Ratio value
   ErrorHandler::ReturnCode CtcManagerImpl::getUpperLowerOceanicCrustRatio(double & ULOceanicCrustRatio)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         if (!m_ctcIoTbl) { throw Exception(NonexistingID) << s_ctcTableName << " table could not be found in project"; }
         database::Record * rec = m_ctcIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in CTC Io table: "; }
         ULOceanicCrustRatio = rec->getValue<double>(s_ULOceaCrustRatioFieldName);
      }

      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Set Upper Lower Oceanic Crust Ratio value
   ErrorHandler::ReturnCode CtcManagerImpl::setUpperLowerOceanicCrustRatio(double & ULOceanicCrustRatio)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_ctcIoTbl) { throw Exception(NonexistingID) << s_ctcTableName << " table could not be found in project"; }
         database::Record * rec = m_ctcIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in CTC Io table: "; }
         rec->setValue<double>(s_ULOceaCrustRatioFieldName, ULOceanicCrustRatio);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
  

}

