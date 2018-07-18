//                                                                      
// Copyright (C) 2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell .
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file ReservoirManagerImpl.C
/// @brief This file keeps API implementation for manipulating Reservoirs in Cauldron model

#include "ReservoirManagerImpl.h"

#include <stdexcept>
#include <string>

namespace mbapi
{
   const char * ReservoirManagerImpl::s_reservoirTableName = "ReservoirIoTbl";
   const char * ReservoirManagerImpl::s_reservoirOptionsTableName = "ReservoirOptionsIoTbl";
   const char * ReservoirManagerImpl::s_trapCapacityFieldName = "TrapCapacity";
   const char * ReservoirManagerImpl::s_bioDegradIndFieldName = "BioDegradInd";
   const char * ReservoirManagerImpl::s_oilToGasCrackingIndFieldName = "OilToGasCrackingInd";
   const char * ReservoirManagerImpl::s_diffusionIndFieldName = "DiffusionInd";
   const char * ReservoirManagerImpl::s_minOilColumnHeightFieldName = "MinOilColumnHeight";
   const char * ReservoirManagerImpl::s_minGasColumnHeightFieldName = "MinGasColumnHeight";
   const char * ReservoirManagerImpl::s_blockingIndFieldName = "BlockingInd";
   const char * ReservoirManagerImpl::s_blockingPermeabilityFieldName = "BlockingPermeability";
   const char * ReservoirManagerImpl::s_blockingPorosityFieldName = "BlockingPorosity";
   const char * ReservoirManagerImpl::s_reservoirTypeFieldName = "ReservoirName";

   // Constructor
   ReservoirManagerImpl::ReservoirManagerImpl()
   {
      m_db = NULL;
   }

   // Copy operator
   ReservoirManagerImpl & ReservoirManagerImpl::operator = (const ReservoirManagerImpl & /*otherReservoirMgr*/)
   {
      throw std::runtime_error("Not implemented yet");
      return *this;
   }

   // Set project database. Reset all
   void ReservoirManagerImpl::setDatabase(database::ProjectFileHandlerPtr pfh)
   {
      m_db = pfh;
      m_reservoirIoTbl = m_db->getTable(s_reservoirTableName);
      m_reservoirOptionsIoTbl = m_db->getTable(s_reservoirOptionsTableName);
   }

   // Get list of Reservoirs in the model
   // return array with IDs of different Reservoirs defined in the model
   std::vector<ReservoirManager::ReservoirID> ReservoirManagerImpl::getReservoirsID() const
   {
      std::vector<ReservoirID> resIDs; // if m_lithIoTbl does not exist - return empty array
      if (m_reservoirIoTbl)
      {
         // fill IDs array with increasing indexes
         resIDs.resize(m_reservoirIoTbl->size(), 0);
         for (size_t i = 0; i < resIDs.size(); ++i) resIDs[i] = static_cast<ReservoirID>(i);
      }
      return resIDs;
   }
   // Get reservoir name for
   // [in] id reservoir ID
   // [out] resName on succes has a reservoir name, or empty string otherwise
   // return NoError on success or NonexistingID on error
   ErrorHandler::ReturnCode ReservoirManagerImpl::getResName(const ReservoirID id, std::string & resName) 
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_reservoirIoTbl) { throw Exception(NonexistingID) << s_reservoirTableName << " table could not be found in project"; }
         database::Record * rec = m_reservoirIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No reservoir with such ID: " << id; }
         resName = rec->getValue<std::string>(s_reservoirTypeFieldName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   //
   // Get Reservoir capacity field value 
   ErrorHandler::ReturnCode ReservoirManagerImpl::getResCapacity(const ReservoirID id, double & resCap)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         // if table does not exist - report error
         if (!m_reservoirIoTbl) { throw Exception(NonexistingID) << s_reservoirTableName << " table could not be found in project"; }

         database::Record * rec = m_reservoirIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No Reservoir with such ID: " << id; }

         resCap = rec->getValue<double>(s_trapCapacityFieldName);         
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

      return NoError;
   }

   ErrorHandler::ReturnCode ReservoirManagerImpl::getResBioDegradInd(const ReservoirID id, int & bioDegradInd)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         // if table does not exist - report error
         if (!m_reservoirIoTbl) { throw Exception(NonexistingID) << s_reservoirTableName << " table could not be found in project"; }

         database::Record * rec = m_reservoirIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No Reservoir with such ID: " << id; }

         bioDegradInd = rec->getValue<int>(s_bioDegradIndFieldName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

      return NoError;
   }


   ErrorHandler::ReturnCode ReservoirManagerImpl::getResOilToGasCrackingInd(const ReservoirID id, int & oilToGasCrackingInd)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         // if table does not exist - report error
         if (!m_reservoirIoTbl) { throw Exception(NonexistingID) << s_reservoirTableName << " table could not be found in project"; }

         database::Record * rec = m_reservoirIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No Reservoir with such ID: " << id; }

         oilToGasCrackingInd = rec->getValue<int>(s_oilToGasCrackingIndFieldName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

      return NoError;
   }

   ErrorHandler::ReturnCode ReservoirManagerImpl::getResBlockingInd(const ReservoirID id, int & blockingInd)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         // if table does not exist - report error
         if (!m_reservoirIoTbl) { throw Exception(NonexistingID) << s_reservoirTableName << " table could not be found in project"; }

         database::Record * rec = m_reservoirIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No Reservoir with such ID: " << id; }

         blockingInd = rec->getValue<int>(s_blockingIndFieldName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

      return NoError;
   }

   ErrorHandler::ReturnCode ReservoirManagerImpl::getResBlockingPermeability(const ReservoirID id, double & blockingPermeability)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         // if table does not exist - report error
         if (!m_reservoirIoTbl) { throw Exception(NonexistingID) << s_reservoirTableName << " table could not be found in project"; }

         database::Record * rec = m_reservoirIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No Reservoir with such ID: " << id; }

         blockingPermeability = rec->getValue<double>(s_blockingPermeabilityFieldName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

      return NoError;
   } 
 
   // Set Reservoir capacity field
   ErrorHandler::ReturnCode ReservoirManagerImpl::setResCapacity(const ReservoirID id, const double & resCap)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         // if table does not exist - report error
         if (!m_reservoirIoTbl) { throw Exception(NonexistingID) << s_reservoirTableName << " table could not be found in project"; }

         database::Record * rec = m_reservoirIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No Reservoir type with such ID: " << id; }
         rec->setValue<double>(s_trapCapacityFieldName, resCap);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   //
   ErrorHandler::ReturnCode ReservoirManagerImpl::setResBioDegradInd(const ReservoirID id, const int & bioDegradInd)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         // if table does not exist - report error
         if (!m_reservoirIoTbl) { throw Exception(NonexistingID) << s_reservoirTableName << " table could not be found in project"; }

         database::Record * rec = m_reservoirIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No Reservoir type with such ID: " << id; }
         rec->setValue<int>(s_bioDegradIndFieldName, bioDegradInd);         
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   //
   ErrorHandler::ReturnCode ReservoirManagerImpl::setResOilToGasCrackingInd(const ReservoirID id, const int & oilToGasCrackingInd)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         // if table does not exist - report error
         if (!m_reservoirIoTbl) { throw Exception(NonexistingID) << s_reservoirTableName << " table could not be found in project"; }

         database::Record * rec = m_reservoirIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No Reservoir type with such ID: " << id; }
         rec->setValue<int>(s_oilToGasCrackingIndFieldName, oilToGasCrackingInd);         
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   //
   ErrorHandler::ReturnCode ReservoirManagerImpl::setResBlockingInd(const ReservoirID id, const int & blockingInd)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         // if table does not exist - report error
         if (!m_reservoirIoTbl) { throw Exception(NonexistingID) << s_reservoirTableName << " table could not be found in project"; }

         database::Record * rec = m_reservoirIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No Reservoir type with such ID: " << id; }
         rec->setValue<int>(s_blockingIndFieldName, blockingInd);         
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   //
   ErrorHandler::ReturnCode ReservoirManagerImpl::setResBlockingPermeability(const ReservoirID id, const double & blockingPermeability)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         // if table does not exist - report error
         if (!m_reservoirIoTbl) { throw Exception(NonexistingID) << s_reservoirTableName << " table could not be found in project"; }

         database::Record * rec = m_reservoirIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No Reservoir type with such ID: " << id; }
         rec->setValue<double>(s_blockingPermeabilityFieldName, blockingPermeability);         
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   //
   ErrorHandler::ReturnCode ReservoirManagerImpl::setResDiffusionInd(const ReservoirID id, const int & diffusionInd)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         // if table does not exist - report error
         if (!m_reservoirIoTbl) { throw Exception(NonexistingID) << s_reservoirTableName << " table could not be found in project"; }

         database::Record * rec = m_reservoirIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No Reservoir type with such ID: " << id; }
         rec->setValue<int>(s_diffusionIndFieldName, diffusionInd);         
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   //
   ErrorHandler::ReturnCode ReservoirManagerImpl::setResMinOilColumnHeight(const ReservoirID id, const double & minOilColumnHeight)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         // if table does not exist - report error
         if (!m_reservoirIoTbl) { throw Exception(NonexistingID) << s_reservoirTableName << " table could not be found in project"; }

         database::Record * rec = m_reservoirIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No Reservoir type with such ID: " << id; }
         rec->setValue<double>(s_minOilColumnHeightFieldName, minOilColumnHeight);         
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   //
   ErrorHandler::ReturnCode ReservoirManagerImpl::setResMinGasColumnHeight(const ReservoirID id, const double & minGasColumnHeight)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         // if table does not exist - report error
         if (!m_reservoirIoTbl) { throw Exception(NonexistingID) << s_reservoirTableName << " table could not be found in project"; }

         database::Record * rec = m_reservoirIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No Reservoir type with such ID: " << id; }
         rec->setValue<double>(s_minGasColumnHeightFieldName, minGasColumnHeight);         
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   //
   ErrorHandler::ReturnCode ReservoirManagerImpl::setResBlockingPorosity(const ReservoirID id, const double & blockingPorosity)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         // if table does not exist - report error
         if (!m_reservoirIoTbl) { throw Exception(NonexistingID) << s_reservoirTableName << " table could not be found in project"; }

         database::Record * rec = m_reservoirIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No Reservoir type with such ID: " << id; }
         rec->setValue<double>(s_blockingPorosityFieldName, blockingPorosity);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   //



   // Set Reservoir capacity field
   ErrorHandler::ReturnCode ReservoirManagerImpl::setResOptionsCapacity(const ReservoirID id, const double & resCap)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         // if table does not exist - report error
         if (!m_reservoirOptionsIoTbl) { throw Exception(NonexistingID) << s_reservoirOptionsTableName << " table could not be found in project"; }

         database::Record * rec = m_reservoirOptionsIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No Reservoir type with such ID: " << id; }
         rec->setValue<double>(s_trapCapacityFieldName, resCap);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   //
   ErrorHandler::ReturnCode ReservoirManagerImpl::setResOptionsBioDegradInd(const ReservoirID id, const int & bioDegradInd)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         // if table does not exist - report error
         if (!m_reservoirOptionsIoTbl) { throw Exception(NonexistingID) << s_reservoirOptionsTableName << " table could not be found in project"; }

         database::Record * rec = m_reservoirOptionsIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No Reservoir type with such ID: " << id; }
         rec->setValue<int>(s_bioDegradIndFieldName, bioDegradInd);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   //
   ErrorHandler::ReturnCode ReservoirManagerImpl::setResOptionsOilToGasCrackingInd(const ReservoirID id, const int & oilToGasCrackingInd)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         // if table does not exist - report error
         if (!m_reservoirOptionsIoTbl) { throw Exception(NonexistingID) << s_reservoirOptionsTableName << " table could not be found in project"; }

         database::Record * rec = m_reservoirOptionsIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No Reservoir type with such ID: " << id; }
         rec->setValue<int>(s_oilToGasCrackingIndFieldName, oilToGasCrackingInd);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   //
   ErrorHandler::ReturnCode ReservoirManagerImpl::setResOptionsBlockingInd(const ReservoirID id, const int & blockingInd)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         // if table does not exist - report error
         if (!m_reservoirOptionsIoTbl) { throw Exception(NonexistingID) << s_reservoirOptionsTableName << " table could not be found in project"; }

         database::Record * rec = m_reservoirOptionsIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No Reservoir type with such ID: " << id; }
         rec->setValue<int>(s_blockingIndFieldName, blockingInd);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   //
   ErrorHandler::ReturnCode ReservoirManagerImpl::setResOptionsBlockingPermeability(const ReservoirID id, const double & blockingPermeability)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         // if table does not exist - report error
         if (!m_reservoirOptionsIoTbl) { throw Exception(NonexistingID) << s_reservoirOptionsTableName << " table could not be found in project"; }

         database::Record * rec = m_reservoirOptionsIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No Reservoir type with such ID: " << id; }
         rec->setValue<double>(s_blockingPermeabilityFieldName, blockingPermeability);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   //
   ErrorHandler::ReturnCode ReservoirManagerImpl::setResOptionsDiffusionInd(const ReservoirID id, const int & diffusionInd)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         // if table does not exist - report error
         if (!m_reservoirOptionsIoTbl) { throw Exception(NonexistingID) << s_reservoirOptionsTableName << " table could not be found in project"; }

         database::Record * rec = m_reservoirOptionsIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No Reservoir type with such ID: " << id; }
         rec->setValue<int>(s_diffusionIndFieldName, diffusionInd);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   //
   ErrorHandler::ReturnCode ReservoirManagerImpl::setResOptionsMinOilColumnHeight(const ReservoirID id, const double & minOilColumnHeight)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         // if table does not exist - report error
         if (!m_reservoirOptionsIoTbl) { throw Exception(NonexistingID) << s_reservoirOptionsTableName << " table could not be found in project"; }

         database::Record * rec = m_reservoirOptionsIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No Reservoir type with such ID: " << id; }
         rec->setValue<double>(s_minOilColumnHeightFieldName, minOilColumnHeight);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   //
   ErrorHandler::ReturnCode ReservoirManagerImpl::setResOptionsMinGasColumnHeight(const ReservoirID id, const double & minGasColumnHeight)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         // if table does not exist - report error
         if (!m_reservoirOptionsIoTbl) { throw Exception(NonexistingID) << s_reservoirOptionsTableName << " table could not be found in project"; }

         database::Record * rec = m_reservoirOptionsIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No Reservoir type with such ID: " << id; }
         rec->setValue<double>(s_minGasColumnHeightFieldName, minGasColumnHeight);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   //
   ErrorHandler::ReturnCode ReservoirManagerImpl::setResOptionsBlockingPorosity(const ReservoirID id, const double & blockingPorosity)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         // if table does not exist - report error
         if (!m_reservoirOptionsIoTbl) { throw Exception(NonexistingID) << s_reservoirOptionsTableName << " table could not be found in project"; }

         database::Record * rec = m_reservoirOptionsIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No Reservoir type with such ID: " << id; }
         rec->setValue<double>(s_blockingPorosityFieldName, blockingPorosity);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }
   //

}

