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
   const char * CtcManagerImpl::s_endRiftingAgeGridFieldName = "TRIniGrid";
   const char * CtcManagerImpl::s_endRiftingAgeFieldName = "TRIni";
   const char * CtcManagerImpl::s_rdaScalarFieldName = "DeltaSL";
   const char * CtcManagerImpl::s_rdaMapFieldName = "DeltaSLGrid";
   const char * CtcManagerImpl::s_basaltThicknessScalarFieldName = "HBu";
   const char * CtcManagerImpl::s_basaltThicknessMapFieldName = "HBuGrid";

   const char * CtcManagerImpl::s_stratIoTableName = "StratIoTbl";
   const char * CtcManagerImpl::s_FormationDepoAgeFieldName = "DepoAge";

   const char * CtcManagerImpl::s_ctcRiftingHistoryIoTableName = "CTCRiftingHistoryIoTbl";
   const char * CtcManagerImpl::s_ctcRiftingHistoryAgeFieldName = "Age";
   const char * CtcManagerImpl::s_tectonicContextFieldName = "TectonicFlag";

   const char * CtcManagerImpl::s_gridMapIoTableName = "GridMapIoTbl";
   const char * CtcManagerImpl::s_gridMapReferredByFieldName = "ReferredBy";
   const char * CtcManagerImpl::s_gridMapIoTblMapNameFieldName = "MapName";
   
   // Constructor
   CtcManagerImpl::CtcManagerImpl()
   {
      m_db = nullptr;
      m_ctcIoTbl = nullptr;
      m_stratIoTbl = nullptr;
      m_ctcRifthingHistoryIoTbl = nullptr;
      m_gridMapIoTbl = nullptr;
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
      m_stratIoTbl= m_db->getTable(s_stratIoTableName);
      m_ctcRifthingHistoryIoTbl = m_db->getTable(s_ctcRiftingHistoryIoTableName);
      m_gridMapIoTbl = m_db->getTable(s_gridMapIoTableName);
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
   ErrorHandler::ReturnCode CtcManagerImpl::setFilterHalfWidthValue(const int & FiltrHalfWidth)
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
   ErrorHandler::ReturnCode CtcManagerImpl::setUpperLowerContinentalCrustRatio(const double & ULContCrustRatio)
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
   ErrorHandler::ReturnCode CtcManagerImpl::setUpperLowerOceanicCrustRatio(const double & ULOceanicCrustRatio)
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
   // Get list of time steps defined in the StratIo table
   // return array with IDs of different time steps defined in the StratIo table
   std::vector<CtcManager::StratigraphyTblLayerID> CtcManagerImpl::getStratigraphyTblLayerID() const
   {
      std::vector<StratigraphyTblLayerID> StratLayerIDs;
      if (m_stratIoTbl)
      {
         // fill IDs array with increasing indexes
         StratLayerIDs.resize(m_stratIoTbl->size(), 0);
         for (size_t i = 0; i < StratLayerIDs.size(); ++i) StratLayerIDs[i] = static_cast<StratigraphyTblLayerID>(i);
      }

      return StratLayerIDs;
   }

   // Get depositional age of a particular stratigraphic layer from StratIoTbl
   ErrorHandler::ReturnCode CtcManagerImpl::getDepoAge(const StratigraphyTblLayerID id, double & DepoAge)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_stratIoTbl) { throw Exception(NonexistingID) << s_stratIoTableName << " table could not be found in project"; }
         database::Record * rec = m_stratIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         DepoAge = rec->getValue<double>(s_FormationDepoAgeFieldName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   // Get list of time steps defined in CTCRiftingHistoryIoTbl
   // return array with IDs of time steps
   std::vector<CtcManager::TimeStepID> CtcManagerImpl::getTimeStepID() const
   {
      std::vector<TimeStepID> TimeStepIDs;
      if (m_ctcRifthingHistoryIoTbl)
      {
         // fill IDs array with increasing indexes
         TimeStepIDs.resize(m_ctcRifthingHistoryIoTbl->size(), 0);
         for (size_t i = 0; i < TimeStepIDs.size(); ++i) TimeStepIDs[i] = static_cast<TimeStepID>(i);
      }

      return TimeStepIDs;
   }

   // Get age for a time step ID defined in the CTCRiftingHistoryIoTbl
   ErrorHandler::ReturnCode CtcManagerImpl::getCTCRiftingHistoryTblAge(const TimeStepID id, double & Age)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_ctcRifthingHistoryIoTbl) { throw Exception(NonexistingID) << s_ctcRiftingHistoryIoTableName << " table could not be found in project"; }
         database::Record * rec = m_ctcRifthingHistoryIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         Age = rec->getValue<double>(s_ctcRiftingHistoryAgeFieldName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Set age of a particular time step ID defined in the CTCRiftingHistoryIoTbl
   ErrorHandler::ReturnCode CtcManagerImpl::setCTCRiftingHistoryTblAge(const TimeStepID id, const double Age)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_ctcRifthingHistoryIoTbl) { throw Exception(NonexistingID) << s_ctcRiftingHistoryIoTableName << " table could not be found in project"; }
         database::Record * rec = m_ctcRifthingHistoryIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         rec->setValue<double>(s_ctcRiftingHistoryAgeFieldName, Age);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
  
   // Get end rifting age map name
   ErrorHandler::ReturnCode CtcManagerImpl::getEndRiftingAgeMap(std::string & TRIniMapName)
   {
      if (errorCode() != NoError) resetError(); 
      try
      {
         if (!m_ctcIoTbl) { throw Exception(NonexistingID) << s_ctcTableName << " table could not be found in project"; }
         database::Record * rec = m_ctcIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in CTC Io table "; }
         TRIniMapName = rec->getValue<std::string>(s_endRiftingAgeGridFieldName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Set end rifting age map
   ErrorHandler::ReturnCode CtcManagerImpl::setEndRiftingAgeMap(const std::string & TRIniMapName)
   {
	   if (errorCode() != NoError) resetError();

	   try
	   {
		   if (!m_ctcIoTbl) { throw Exception(NonexistingID) << s_ctcTableName << " table could not be found in project"; }
		   database::Record * rec = m_ctcIoTbl->getRecord(static_cast<int>(0));
		   if (!rec) { throw Exception(NonexistingID) << "No data found in CTC Io table: "; }
		   rec->setValue<std::string>(s_endRiftingAgeGridFieldName, TRIniMapName);
	   }
	   catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
	   return NoError;
   }

   // Get end rifting age scalar value
   ErrorHandler::ReturnCode CtcManagerImpl::getEndRiftingAge(double & RiftingEndAge)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         if (!m_ctcIoTbl) { throw Exception(NonexistingID) << s_ctcTableName << " table could not be found in project"; }
         database::Record * rec = m_ctcIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in CTC Io table "; }
         RiftingEndAge = rec->getValue<double>(s_endRiftingAgeFieldName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Get tectonicFlag for a time step ID defined in the CTCRiftingHistoryIoTbl
   ErrorHandler::ReturnCode CtcManagerImpl::getTectonicFlag(const TimeStepID id, std::string & tectonicContext)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_ctcRifthingHistoryIoTbl) { throw Exception(NonexistingID) << s_ctcRiftingHistoryIoTableName << " table could not be found in project"; }
         database::Record * rec = m_ctcRifthingHistoryIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         tectonicContext = rec->getValue<std::string>(s_tectonicContextFieldName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   // Set tectonicFlag of a particular time step ID defined in the CTCRiftingHistoryIoTbl
   ErrorHandler::ReturnCode CtcManagerImpl::setTectonicFlag(const TimeStepID id, const std::string & tectonicContext)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_ctcRifthingHistoryIoTbl) { throw Exception(NonexistingID) << s_ctcRiftingHistoryIoTableName << " table could not be found in project"; }
         database::Record * rec = m_ctcRifthingHistoryIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         rec->setValue<std::string>(s_tectonicContextFieldName, tectonicContext);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Get Relative sealevel adjustment value
   ErrorHandler::ReturnCode CtcManagerImpl::getResidualDepthAnomalyScalar(double & RDAvalue)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         if (!m_ctcIoTbl) { throw Exception(NonexistingID) << s_ctcTableName << " table could not be found in project"; }
         database::Record * rec = m_ctcIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in CTC Io table: "; }
         RDAvalue = rec->getValue<double>(s_rdaScalarFieldName);
      }

      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Set Relative sealevel adjustment value
   ErrorHandler::ReturnCode CtcManagerImpl::setResidualDepthAnomalyScalar(const double & RDAvalue)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_ctcIoTbl) { throw Exception(NonexistingID) << s_ctcTableName << " table could not be found in project"; }
         database::Record * rec = m_ctcIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in CTC Io table: "; }
         rec->setValue<double>(s_rdaScalarFieldName, RDAvalue);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Get Relative sealevel adjustment map
   ErrorHandler::ReturnCode CtcManagerImpl::getResidualDepthAnomalyMap(std::string & RDAmap)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         if (!m_ctcIoTbl) { throw Exception(NonexistingID) << s_ctcTableName << " table could not be found in project"; }
         database::Record * rec = m_ctcIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in CTC Io table: "; }
         RDAmap = rec->getValue<std::string>(s_rdaMapFieldName);
      }

      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Set Relative sealevel adjustment map
   ErrorHandler::ReturnCode CtcManagerImpl::setResidualDepthAnomalyMap(const std::string & RDAmap)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_ctcIoTbl) { throw Exception(NonexistingID) << s_ctcTableName << " table could not be found in project"; }
         database::Record * rec = m_ctcIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in CTC Io table: "; }
         rec->setValue<std::string>(s_rdaMapFieldName, RDAmap);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Get Relative sealevel adjustment for a time step ID defined in the CTCRiftingHistoryIoTbl
   ErrorHandler::ReturnCode CtcManagerImpl::getRiftingTblResidualDepthAnomalyScalar(const TimeStepID id, double & depth)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_ctcRifthingHistoryIoTbl) { throw Exception(NonexistingID) << s_ctcRiftingHistoryIoTableName << " table could not be found in project"; }
         database::Record * rec = m_ctcRifthingHistoryIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         depth = rec->getValue<double>(s_rdaScalarFieldName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   // Set Relative sealevel adjustment of a particular time step ID defined in the CTCRiftingHistoryIoTbl
   ErrorHandler::ReturnCode CtcManagerImpl::setRiftingTblResidualDepthAnomalyScalar(const TimeStepID id, const double & depth)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_ctcRifthingHistoryIoTbl) { throw Exception(NonexistingID) << s_ctcRiftingHistoryIoTableName << " table could not be found in project"; }
         database::Record * rec = m_ctcRifthingHistoryIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         rec->setValue<double>(s_rdaScalarFieldName, depth);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   // Get Relative sealevel adjustment map for a time step ID defined in the CTCRiftingHistoryIoTbl
   ErrorHandler::ReturnCode CtcManagerImpl::getRiftingTblResidualDepthAnomalyMap(const TimeStepID id, std::string & depthMap)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_ctcRifthingHistoryIoTbl) { throw Exception(NonexistingID) << s_ctcRiftingHistoryIoTableName << " table could not be found in project"; }
         database::Record * rec = m_ctcRifthingHistoryIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         depthMap = rec->getValue<std::string>(s_rdaMapFieldName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   // Set Relative sealevel adjustment of a particular time step ID defined in the CTCRiftingHistoryIoTbl
   ErrorHandler::ReturnCode CtcManagerImpl::setRiftingTblResidualDepthAnomalyMap(const TimeStepID id, const std::string & depthMap)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_ctcRifthingHistoryIoTbl) { throw Exception(NonexistingID) << s_ctcRiftingHistoryIoTableName << " table could not be found in project"; }
         database::Record * rec = m_ctcRifthingHistoryIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         rec->setValue<std::string>(s_rdaMapFieldName, depthMap);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   // Get Maximum thickness of basalt melt value
   ErrorHandler::ReturnCode CtcManagerImpl::getBasaltMeltThicknessValue(double & basaltThicknessvalue)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         if (!m_ctcIoTbl) { throw Exception(NonexistingID) << s_ctcTableName << " table could not be found in project"; }
         database::Record * rec = m_ctcIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in CTC Io table: "; }
         basaltThicknessvalue = rec->getValue<double>(s_basaltThicknessScalarFieldName);
      }

      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Set Maximum thickness of basalt melt value
   ErrorHandler::ReturnCode CtcManagerImpl::setBasaltMeltThicknessValue(const double & basaltThicknessvalue)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_ctcIoTbl) { throw Exception(NonexistingID) << s_ctcTableName << " table could not be found in project"; }
         database::Record * rec = m_ctcIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in CTC Io table: "; }
         rec->setValue<double>(s_basaltThicknessScalarFieldName, basaltThicknessvalue);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   // Get Maximum thickness of basalt melt map
   ErrorHandler::ReturnCode CtcManagerImpl::getBasaltMeltThicknessMap(std::string & basaltThicknessMap)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         if (!m_ctcIoTbl) { throw Exception(NonexistingID) << s_ctcTableName << " table could not be found in project"; }
         database::Record * rec = m_ctcIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in CTC Io table: "; }
         basaltThicknessMap = rec->getValue<std::string>(s_basaltThicknessMapFieldName);
      }

      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Set Maximum thickness of basalt melt map
   ErrorHandler::ReturnCode CtcManagerImpl::setBasaltMeltThicknessMap(const std::string & basaltThicknessMap)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_ctcIoTbl) { throw Exception(NonexistingID) << s_ctcTableName << " table could not be found in project"; }
         database::Record * rec = m_ctcIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in CTC Io table: "; }
         rec->setValue<std::string>(s_basaltThicknessMapFieldName, basaltThicknessMap);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   // Get maximum thickness of basalt melt for a time step ID defined in the CTCRiftingHistoryIoTbl
   ErrorHandler::ReturnCode CtcManagerImpl::getRiftingTblBasaltMeltThicknessScalar(const TimeStepID id, double & DepthValue)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_ctcRifthingHistoryIoTbl) { throw Exception(NonexistingID) << s_ctcRiftingHistoryIoTableName << " table could not be found in project"; }
         database::Record * rec = m_ctcRifthingHistoryIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         DepthValue = rec->getValue<double>(s_basaltThicknessScalarFieldName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   // Set maximum thickness of basalt melt of a particular time step ID defined in the CTCRiftingHistoryIoTbl
   ErrorHandler::ReturnCode CtcManagerImpl::setRiftingTblBasaltMeltThicknessScalar(const TimeStepID id, const double & DepthValue)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_ctcRifthingHistoryIoTbl) { throw Exception(NonexistingID) << s_ctcRiftingHistoryIoTableName << " table could not be found in project"; }
         database::Record * rec = m_ctcRifthingHistoryIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         rec->setValue<double>(s_basaltThicknessScalarFieldName, DepthValue);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   // Get maximum thickness of basalt melt map for a time step ID defined in the CTCRiftingHistoryIoTbl
   ErrorHandler::ReturnCode CtcManagerImpl::getRiftingTblBasaltMeltThicknessMap(const TimeStepID id, std::string & DepthMap)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_ctcRifthingHistoryIoTbl) { throw Exception(NonexistingID) << s_ctcRiftingHistoryIoTableName << " table could not be found in project"; }
         database::Record * rec = m_ctcRifthingHistoryIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         DepthMap = rec->getValue<std::string>(s_basaltThicknessMapFieldName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   // Set maximum thickness of basalt melt map of a particular time step ID defined in the CTCRiftingHistoryIoTbl
   ErrorHandler::ReturnCode CtcManagerImpl::setRiftingTblBasaltMeltThicknessMap(const TimeStepID id, const std::string & DepthMap)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_ctcRifthingHistoryIoTbl) { throw Exception(NonexistingID) << s_ctcRiftingHistoryIoTableName << " table could not be found in project"; }
         database::Record * rec = m_ctcRifthingHistoryIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         rec->setValue<std::string>(s_basaltThicknessMapFieldName, DepthMap);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   // Get the table name referred in the GridMapIoTbl for an id
   ErrorHandler::ReturnCode CtcManagerImpl::getGridMapTablename(const GridMapID id, std::string & tableName)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_gridMapIoTbl) { throw Exception(NonexistingID) << s_gridMapIoTableName << " table could not be found in project"; }
         database::Record * rec = m_gridMapIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         tableName = rec->getValue<std::string>(s_gridMapReferredByFieldName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   // Set the table name referred in the GridMapIoTbl for an id
   ErrorHandler::ReturnCode CtcManagerImpl::setGridMapTablename(const GridMapID id, const std::string & tableName)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_gridMapIoTbl) { throw Exception(NonexistingID) << s_gridMapIoTableName << " table could not be found in project"; }
         database::Record * rec = m_gridMapIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         rec->setValue<std::string>(s_gridMapReferredByFieldName, tableName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Get list of rows defined in the GridMapIo table
   // return array with IDs of rows defined in the GridMapIo table
   std::vector<CtcManager::GridMapID> CtcManagerImpl::getGridMapID() const
   {
      std::vector<GridMapID> IDs;
      if (m_gridMapIoTbl)
      {
         // fill IDs array with increasing indexes
         IDs.resize(m_gridMapIoTbl->size(), 0);
         for (size_t i = 0; i < IDs.size(); ++i) IDs[i] = static_cast<GridMapID>(i);
      }

      return IDs;
   }
   // Get the map name referred in the GridMapIoTbl for a an id
   ErrorHandler::ReturnCode CtcManagerImpl::getGridMapIoTblMapName(const GridMapID id, std::string & tableName)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_gridMapIoTbl) { throw Exception(NonexistingID) << s_gridMapIoTableName << " table could not be found in project"; }
         database::Record * rec = m_gridMapIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         tableName = rec->getValue<std::string>(s_gridMapIoTblMapNameFieldName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   // Set the map name referred in the GridMapIoTbl for a an id
   ErrorHandler::ReturnCode CtcManagerImpl::setGridMapIoTblMapName(const GridMapID id, const std::string & tableName)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_gridMapIoTbl) { throw Exception(NonexistingID) << s_gridMapIoTableName << " table could not be found in project"; }
         database::Record * rec = m_gridMapIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         rec->setValue<std::string>(s_gridMapIoTblMapNameFieldName, tableName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

}

