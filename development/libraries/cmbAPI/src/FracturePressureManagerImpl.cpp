//                                                                      
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by CGI.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file FracturePressureManager.h
/// @brief This file keeps API declaration which deal with fracture pressure related inputs

#include "FracturePressureManagerImpl.h"
#include <stdexcept>
#include <string>


namespace mbapi
{
   const char * FracturePressureManagerImpl::s_runOptionsTableName = "RunOptionsIoTbl";
   const char * FracturePressureManagerImpl::s_FracturePressureFunctionFieldName = "FracturePressureFunction";
   const char * FracturePressureManagerImpl::s_FracturePressureTypeFieldName = "FractureType";
   const char * FracturePressureManagerImpl::s_FracturePressureConstraintFieldName = "FractureModel";

   const char * FracturePressureManagerImpl::s_pressFuncTableName = "PressureFuncIoTbl";
   const char * FracturePressureManagerImpl::s_PressureFunctionFieldName = "PresFuncName";
   const char * FracturePressureManagerImpl::s_UserDefinedFlagFieldName = "UserDefined";
   
  
   // Constructor
   FracturePressureManagerImpl::FracturePressureManagerImpl()
   {
      m_db = nullptr;
      m_runOptionsIoTbl = nullptr;
      m_pressFuncIoTbl = nullptr;
   }

   // Copy operator
   FracturePressureManagerImpl & FracturePressureManagerImpl::operator = (const FracturePressureManagerImpl &)
   {
      throw std::runtime_error("Not implemented yet");
      return *this;
   }

   // Set project database. Reset all
   void FracturePressureManagerImpl::setDatabase(database::ProjectFileHandlerPtr pfh)
   {
      m_db = pfh;
      m_runOptionsIoTbl = m_db->getTable(s_runOptionsTableName);
      m_pressFuncIoTbl = m_db->getTable(s_pressFuncTableName);
   }

   // Get Fracture Pressure model name specified in the RunOptionsIoTbl
   ErrorHandler::ReturnCode FracturePressureManagerImpl::getFracturePressureFunctionName(std::string & FPModelName)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         if (!m_runOptionsIoTbl) { throw Exception(NonexistingID) << s_runOptionsTableName << " table could not be found in project"; }
         database::Record * rec = m_runOptionsIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in RunOptionsIo table: "; }
         FPModelName = rec->getValue<std::string>(s_FracturePressureFunctionFieldName);
      }

      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Set Fracture Pressure model name in the RunOptionsIoTbl
   ErrorHandler::ReturnCode FracturePressureManagerImpl::setFracturePressureFunctionName(const std::string & FPModelName)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         if (!m_runOptionsIoTbl) { throw Exception(NonexistingID) << s_runOptionsTableName << " table could not be found in project"; }
         database::Record * rec = m_runOptionsIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in RunOptionsIo table: "; }
         rec->setValue<std::string>(s_FracturePressureFunctionFieldName, FPModelName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Get list of models defined in the PressureFuncIo table
   // return array with IDs of models defined in the PressureFuncIo table
   std::vector<FracturePressureManager::pressFuncTblID> FracturePressureManagerImpl::getpressFuncTblID() const
   {
      std::vector<pressFuncTblID> IDs;
      if (m_pressFuncIoTbl)
      {
         // fill IDs array with increasing indexes
         IDs.resize(m_pressFuncIoTbl->size(), 0);
         for (size_t i = 0; i < IDs.size(); ++i) IDs[i] = static_cast<pressFuncTblID>(i);
      }

      return IDs;
   }

   // Get model name of a particular ID
   ErrorHandler::ReturnCode FracturePressureManagerImpl::getPresFuncName(const pressFuncTblID id, std::string & Name)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         if (!m_pressFuncIoTbl) { throw Exception(NonexistingID) << s_pressFuncTableName << " table could not be found in project"; }
         database::Record * rec = m_pressFuncIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         Name = rec->getValue<std::string>(s_PressureFunctionFieldName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Get Fracture Pressure type specified in the RunOptionsIoTbl
   ErrorHandler::ReturnCode FracturePressureManagerImpl::getFracturePressureType(std::string & FPType)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         if (!m_runOptionsIoTbl) { throw Exception(NonexistingID) << s_runOptionsTableName << " table could not be found in project"; }
         database::Record * rec = m_runOptionsIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in RunOptionsIo table: "; }
         FPType = rec->getValue<std::string>(s_FracturePressureTypeFieldName);
      }

      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   // Set Fracture Pressure type in the RunOptionsIoTbl
   ErrorHandler::ReturnCode FracturePressureManagerImpl::setFracturePressureType(const std::string & FPType)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         if (!m_runOptionsIoTbl) { throw Exception(NonexistingID) << s_runOptionsTableName << " table could not be found in project"; }
         database::Record * rec = m_runOptionsIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in RunOptionsIo table: "; }
         rec->setValue<std::string>(s_FracturePressureTypeFieldName, FPType);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   // Get Fracture Pressure constraint method specified in the RunOptionsIoTbl
   ErrorHandler::ReturnCode FracturePressureManagerImpl::getFractureConstraintMethod(int & FPMethod)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         if (!m_runOptionsIoTbl) { throw Exception(NonexistingID) << s_runOptionsTableName << " table could not be found in project"; }
         database::Record * rec = m_runOptionsIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in RunOptionsIo table: "; }
         FPMethod = rec->getValue<int>(s_FracturePressureConstraintFieldName);
      }

      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   // Set Fracture Pressure constraint method in the RunOptionsIoTbl
   ErrorHandler::ReturnCode FracturePressureManagerImpl::setFractureConstraintMethod(const int & FPMethod)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         if (!m_runOptionsIoTbl) { throw Exception(NonexistingID) << s_runOptionsTableName << " table could not be found in project"; }
         database::Record * rec = m_runOptionsIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in RunOptionsIo table: "; }
         rec->setValue<int>(s_FracturePressureConstraintFieldName, FPMethod);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Get user defined flag of a particular ID of PressureFuncIoTbl
   ErrorHandler::ReturnCode FracturePressureManagerImpl::getUserDefinedFlag(const pressFuncTblID id, int & UserDefinedFlag)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         if (!m_pressFuncIoTbl) { throw Exception(NonexistingID) << s_pressFuncTableName << " table could not be found in project"; }
         database::Record * rec = m_pressFuncIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         UserDefinedFlag = rec->getValue<int>(s_UserDefinedFlagFieldName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Set user defined flag of a particular ID of PressureFuncIoTbl
   ErrorHandler::ReturnCode FracturePressureManagerImpl::setUserDefinedFlag(const pressFuncTblID id, const int & UserDefinedFlag)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         if (!m_pressFuncIoTbl) { throw Exception(NonexistingID) << s_pressFuncTableName << " table could not be found in project"; }
         database::Record * rec = m_pressFuncIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         rec->setValue<int>(s_UserDefinedFlagFieldName, UserDefinedFlag);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

}

