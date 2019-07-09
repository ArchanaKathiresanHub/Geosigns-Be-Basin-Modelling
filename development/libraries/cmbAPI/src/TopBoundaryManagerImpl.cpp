//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file TopBoundaryManager.h
/// @brief This file keeps API declaration for manipulating top boundary model in Cauldron model

#include "TopBoundaryManagerImpl.h"
#include <stdexcept>
#include <string>

namespace mbapi
{

   const char * TopBoundaryManagerImpl::s_surfaceDepthIoTableName = "SurfaceDepthIoTbl";
   const char * TopBoundaryManagerImpl::s_surfceDepthAgeFieldName = "Age";

   const char * TopBoundaryManagerImpl::s_surfaceTempIoTableName = "SurfaceTempIoTbl";
   const char * TopBoundaryManagerImpl::s_surfceTempAgeFieldName = "Age";


   // Constructor
   TopBoundaryManagerImpl::TopBoundaryManagerImpl()
   {
      m_db = nullptr;
	  m_surfaceDepthIoTbl = nullptr;
	  m_surfaceTempIoTbl = nullptr;
   }

   // Copy operator
   TopBoundaryManagerImpl & TopBoundaryManagerImpl::operator = (const TopBoundaryManagerImpl & /*otherFluidMgr*/)
   {
      throw std::runtime_error("Not implemented yet");
      return *this;
   }

   // Set project database. Reset all
   void TopBoundaryManagerImpl::setDatabase(database::ProjectFileHandlerPtr pfh)
   {
      m_db = pfh;

	  m_surfaceDepthIoTbl = m_db->getTable(s_surfaceDepthIoTableName);
	  m_surfaceTempIoTbl = m_db->getTable(s_surfaceTempIoTableName);
  
   }

   // Get list of surfaces in the model
   // return array with IDs of different surfaces defined in the model
   std::vector<TopBoundaryManager::SurfaceID>  TopBoundaryManagerImpl::getSurfaceDepthIDs() const
   {
	   std::vector<SurfaceID> surfIDs;
	   if (m_surfaceDepthIoTbl)
	   {
		   surfIDs.resize(m_surfaceDepthIoTbl->size(), 0);
		   for (size_t i = 0; i < surfIDs.size(); ++i) 
		   {
			   surfIDs[i] = static_cast<SurfaceID>(i);
		   }
	   }
	   return surfIDs;
   }

   // Get list of surfaces in the model
   // return array with IDs of different surfaces defined in the model
   std::vector<TopBoundaryManager::SurfaceID>  TopBoundaryManagerImpl::getSurfaceTempIDs() const
   {
	   std::vector<SurfaceID> surfIDs;
	   if (m_surfaceTempIoTbl)
	   {
		   surfIDs.resize(m_surfaceTempIoTbl->size(), 0);
		   for (size_t i = 0; i < surfIDs.size(); ++i)
		   {
			   surfIDs[i] = static_cast<SurfaceID>(i);
		   }
	   }
	   return surfIDs;
   }

   ErrorHandler::ReturnCode TopBoundaryManagerImpl::getSurfaceDepthAge(const SurfaceID id, double& surfAge) 
   {
	   if (errorCode() != NoError) resetError();

	   try
	   {
		   if (!m_surfaceDepthIoTbl) { throw Exception(NonexistingID) << s_surfaceDepthIoTableName << " table could not be found in project"; }
		   database::Record * rec = m_surfaceDepthIoTbl->getRecord(static_cast<int>(id));
		   if (!rec) { throw Exception(NonexistingID) << "No surface type with such ID: " << id; }
		   surfAge = rec->getValue<double>(s_surfceDepthAgeFieldName);
	   }
	   catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
	   return NoError;
   }

   

   ErrorHandler::ReturnCode TopBoundaryManagerImpl::setSurfaceDepthAge(const SurfaceID id, double& surfAge)
   {
	   if (errorCode() != NoError) resetError();
	   try
	   {
		   // if table does not exist - report error
		   if (!m_surfaceDepthIoTbl) { throw Exception(NonexistingID) << s_surfaceDepthIoTableName << " table could not be found in project"; }

		   database::Record * rec = m_surfaceDepthIoTbl->getRecord(static_cast<int>(id));
		   if (!rec) { throw Exception(NonexistingID) << "No surface type with such ID: " << id; }

		   rec->setValue<double>(s_surfceDepthAgeFieldName, surfAge);
	   }
	   catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

	   return NoError;
   }

   ErrorHandler::ReturnCode TopBoundaryManagerImpl::getSurfaceTempAge(const SurfaceID id, double& surfAge)
   {
	   if (errorCode() != NoError) resetError();

	   try
	   {
		   if (!m_surfaceTempIoTbl) { throw Exception(NonexistingID) << s_surfaceTempIoTableName << " table could not be found in project"; }
		   database::Record * rec = m_surfaceTempIoTbl->getRecord(static_cast<int>(id));
		   if (!rec) { throw Exception(NonexistingID) << "No surface type with such ID: " << id; }
		   surfAge = rec->getValue<double>(s_surfceTempAgeFieldName);
	   }
	   catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
	   return NoError;
   }

   ErrorHandler::ReturnCode TopBoundaryManagerImpl::setSurfaceTempAge(const SurfaceID id, double& surfAge)
   {
	   if (errorCode() != NoError) resetError();
	   try
	   {
		   // if table does not exist - report error
		   if (!m_surfaceTempIoTbl) { throw Exception(NonexistingID) << s_surfaceTempIoTableName << " table could not be found in project"; }

		   database::Record * rec = m_surfaceTempIoTbl->getRecord(static_cast<int>(id));
		   if (!rec) { throw Exception(NonexistingID) << "No surface type with such ID: " << id; }

		   rec->setValue<double>(s_surfceTempAgeFieldName, surfAge);
	   }
	   catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

	   return NoError;
   }

}

