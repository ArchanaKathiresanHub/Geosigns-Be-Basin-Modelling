//                                                                      
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by CGI.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file ProjectDataManager.h
/// @brief This file keeps API declaration which deal with project related inputs

#include "ProjectDataManagerImpl.h"
#include <stdexcept>
#include <string>


namespace mbapi
{
   const char* ProjectDataManagerImpl::s_projectOptionsTableName = "ProjectIoTbl";
   const char* ProjectDataManagerImpl::s_xNodesFieldName = "NumberX";
   const char* ProjectDataManagerImpl::s_yNodesFieldName = "NumberY";
   const char* ProjectDataManagerImpl::s_xGridSpecingFieldName = "DeltaX";
   const char* ProjectDataManagerImpl::s_yGridSpecingFieldName = "DeltaY";
   const char* ProjectDataManagerImpl::s_projectDescriptionFieldName = "Description";
   const char* ProjectDataManagerImpl::s_windowXMaxFieldName = "WindowXMax"; 
   const char* ProjectDataManagerImpl::s_windowXMinFieldName = "WindowXMin"; 
   const char* ProjectDataManagerImpl::s_stepXFieldName = "ScaleX";
   const char* ProjectDataManagerImpl::s_windowYMaxFieldName = "WindowYMax";
   const char* ProjectDataManagerImpl::s_windowYMinFieldName = "WindowYMin";
   const char* ProjectDataManagerImpl::s_stepYFieldName = "ScaleY";

   // Constructor
   ProjectDataManagerImpl::ProjectDataManagerImpl()
   {
      m_db = nullptr;
      m_projectIoTbl = nullptr;
   }

   // Copy operator
   ProjectDataManagerImpl& ProjectDataManagerImpl::operator = (const ProjectDataManagerImpl&)
   {
      throw std::runtime_error("Not implemented yet");
      return *this;
   }

   // Set project database. Reset all
   void ProjectDataManagerImpl::setDatabase(database::ProjectFileHandlerPtr pfh)
   {
      m_db = pfh;
      m_projectIoTbl = m_db->getTable(s_projectOptionsTableName);
   }

   // Get number of the x-nodes specified in the ProjectIoTbl
   ErrorHandler::ReturnCode ProjectDataManagerImpl::getNumberOfNodesX(int& NodesX)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         if (!m_projectIoTbl) { throw Exception(NonexistingID) << s_projectOptionsTableName << " table could not be found in project"; }
         database::Record* rec = m_projectIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in ProjectIo table: "; }
         NodesX = rec->getValue<int>(s_xNodesFieldName);
      }

      catch (const Exception& e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Set number of the x-nodes in the ProjectIoTbl
   ErrorHandler::ReturnCode ProjectDataManagerImpl::setNumberOfNodesX(const int& NodesX)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         if (!m_projectIoTbl) { throw Exception(NonexistingID) << s_projectOptionsTableName << " table could not be found in project"; }
         database::Record* rec = m_projectIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in ProjectIo table: "; }
         rec->setValue<int>(s_xNodesFieldName, NodesX);
      }
      catch (const Exception& e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Get number of the y-nodes specified in the ProjectIoTbl
   ErrorHandler::ReturnCode ProjectDataManagerImpl::getNumberOfNodesY(int& NodesY)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         if (!m_projectIoTbl) { throw Exception(NonexistingID) << s_projectOptionsTableName << " table could not be found in project"; }
         database::Record* rec = m_projectIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in ProjectIo table: "; }
         NodesY = rec->getValue<int>(s_yNodesFieldName);
      }

      catch (const Exception& e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Set number of the y-nodes in the ProjectIoTbl
   ErrorHandler::ReturnCode ProjectDataManagerImpl::setNumberOfNodesY(const int& NodesY)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         if (!m_projectIoTbl) { throw Exception(NonexistingID) << s_projectOptionsTableName << " table could not be found in project"; }
         database::Record* rec = m_projectIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in ProjectIo table: "; }
         rec->setValue<int>(s_yNodesFieldName, NodesY);
      }
      catch (const Exception& e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   // Get grid specing in x-direction specified in the ProjectIoTbl
   ErrorHandler::ReturnCode ProjectDataManagerImpl::getDeltaX(double& deltaX)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         if (!m_projectIoTbl) { throw Exception(NonexistingID) << s_projectOptionsTableName << " table could not be found in project"; }
         database::Record* rec = m_projectIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in ProjectIo table: "; }
         deltaX = rec->getValue<double>(s_xGridSpecingFieldName);
      }

      catch (const Exception& e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Set grid specing in x-direction in the ProjectIoTbl
   ErrorHandler::ReturnCode ProjectDataManagerImpl::setDeltaX(const double& deltaX)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         if (!m_projectIoTbl) { throw Exception(NonexistingID) << s_projectOptionsTableName << " table could not be found in project"; }
         database::Record* rec = m_projectIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in ProjectIo table: "; }
         rec->setValue<double>(s_xGridSpecingFieldName, deltaX);
      }
      catch (const Exception& e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Get grid specing in y-direction specified in the ProjectIoTbl
   ErrorHandler::ReturnCode ProjectDataManagerImpl::getDeltaY(double& deltaY)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         if (!m_projectIoTbl) { throw Exception(NonexistingID) << s_projectOptionsTableName << " table could not be found in project"; }
         database::Record* rec = m_projectIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in ProjectIo table: "; }
         deltaY = rec->getValue<double>(s_yGridSpecingFieldName);
      }

      catch (const Exception& e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Set grid specing in y-direction in the ProjectIoTbl
   ErrorHandler::ReturnCode ProjectDataManagerImpl::setDeltaY(const double& deltaY)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         if (!m_projectIoTbl) { throw Exception(NonexistingID) << s_projectOptionsTableName << " table could not be found in project"; }
         database::Record* rec = m_projectIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in ProjectIo table: "; }
         rec->setValue<double>(s_yGridSpecingFieldName, deltaY);
      }
      catch (const Exception& e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Get Get project description specified in the ProjectIoTbl
   ErrorHandler::ReturnCode ProjectDataManagerImpl::getProjectDescription(std::string& description)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         if (!m_projectIoTbl) { throw Exception(NonexistingID) << s_projectOptionsTableName << " table could not be found in project"; }
         database::Record* rec = m_projectIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in ProjectIo table: "; }
         description = rec->getValue<std::string>(s_projectDescriptionFieldName);
      }

      catch (const Exception& e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Set Get project description in the ProjectIoTbl
   ErrorHandler::ReturnCode ProjectDataManagerImpl::setProjectDescription(const std::string& description)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         if (!m_projectIoTbl) { throw Exception(NonexistingID) << s_projectOptionsTableName << " table could not be found in project"; }
         database::Record* rec = m_projectIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in ProjectIo table: "; }
         rec->setValue<std::string>(s_projectDescriptionFieldName, description);
      }
      catch (const Exception& e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   /// Get the values of WindowXMin, WindowsXMax, StepX, WindowYMin, WindowsYMax and StepY specified in the ProjectIoTbl
   ErrorHandler::ReturnCode ProjectDataManagerImpl::getSimulationWindowDetails(int& WindowXMin, int& WindowXMax, int& stepX, int& WindowYMin, int& WindowYMax, int& stepY)
   {
	   if (errorCode() != NoError) resetError();
	   try
	   {
		   if (!m_projectIoTbl) { throw Exception(NonexistingID) << s_projectOptionsTableName << " table could not be found in project"; }
		   database::Record* rec = m_projectIoTbl->getRecord(static_cast<int>(0));
		   if (!rec) { throw Exception(NonexistingID) << "No data found in ProjectIo table: "; }
		   WindowXMax = rec->getValue<int>(s_windowXMaxFieldName);
           WindowXMin = rec->getValue<int>(s_windowXMinFieldName);
           stepX = rec->getValue<int>(s_stepXFieldName);
		   WindowYMax = rec->getValue<int>(s_windowYMaxFieldName);
           WindowYMin = rec->getValue<int>(s_windowYMinFieldName);
           stepY = rec->getValue<int>(s_stepYFieldName);
	   }

	   catch (const Exception& e) { return reportError(e.errorCode(), e.what()); }
	   return NoError;
   }

   // Set the count of WindowXMin, WindowsXMax and StepX in the ProjectIoTbl
   ErrorHandler::ReturnCode ProjectDataManagerImpl::setSimulationWindowX(const int& WindowXMin,const int& WindowXMax, const int& StepX)
   {
	   if (errorCode() != NoError) resetError();
	   try
	   {
		   if (!m_projectIoTbl) { throw Exception(NonexistingID) << s_projectOptionsTableName << " table could not be found in project"; }
		   database::Record* rec = m_projectIoTbl->getRecord(static_cast<int>(0));
		   if (!rec) { throw Exception(NonexistingID) << "No data found in ProjectIo table: "; }
		   rec->setValue<int>(s_windowXMaxFieldName, WindowXMax);
		   rec->setValue<int>(s_windowXMinFieldName, WindowXMin);
           rec->setValue<int>(s_stepXFieldName, StepX);
	   }
	   catch (const Exception& e) { return reportError(e.errorCode(), e.what()); }
	   return NoError;
   }

   // Set the count of WindowYMin, WindowsYMax and StepY in the ProjectIoTbl
   ErrorHandler::ReturnCode ProjectDataManagerImpl::setSimulationWindowY(const int& WindowYMin, const int& WindowYMax, const int& StepY)
   {
       if (errorCode() != NoError) resetError();
       try
       {
           if (!m_projectIoTbl) { throw Exception(NonexistingID) << s_projectOptionsTableName << " table could not be found in project"; }
           database::Record* rec = m_projectIoTbl->getRecord(static_cast<int>(0));
           if (!rec) { throw Exception(NonexistingID) << "No data found in ProjectIo table: "; }
           rec->setValue<int>(s_windowYMaxFieldName, WindowYMax);
           rec->setValue<int>(s_windowYMinFieldName, WindowYMin);
           rec->setValue<int>(s_stepYFieldName, StepY);
       }
       catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
       return NoError;
   }

}

