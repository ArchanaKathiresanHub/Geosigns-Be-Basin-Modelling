//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file BottomBoundaryManager.h
/// @brief This file keeps API declaration for manipulating bottom boundary model in Cauldron model

#include "BottomBoundaryManagerImpl.h"
#include <stdexcept>
#include <string>

namespace mbapi
{
   const char * BottomBoundaryManagerImpl::s_bottomBoundaryModelTableName = "BasementIoTbl";
   const char * BottomBoundaryManagerImpl::s_bottomBoundaryModelFieldName = "BottomBoundaryModel";
   const char * BottomBoundaryManagerImpl::s_crustPropModelFieldName = "CrustPropertyModel";
   const char * BottomBoundaryManagerImpl::s_mantlePropModelFieldName = "MantlePropertyModel";
   const char * BottomBoundaryManagerImpl::s_initLithManThicknessFieldName = "InitialLithosphericMantleThickness";

   const char * BottomBoundaryManagerImpl::s_crustalThicknessTableName = "CrustIoTbl";
   const char * BottomBoundaryManagerImpl::s_CrustalThicknessAgeFieldName = "Age";
   const char * BottomBoundaryManagerImpl::s_CrustalThicknessThicknessFieldName = "Thickness";
   const char * BottomBoundaryManagerImpl::s_CrustalThicknessGridFieldName = "ThicknessGrid";

   const char * BottomBoundaryManagerImpl::s_contCrustalThicknessTableName = "ContCrustalThicknessIoTbl";
   const char * BottomBoundaryManagerImpl::s_ContCrustalThicknessAgeFieldName = "Age";
   const char * BottomBoundaryManagerImpl::s_ContCrustalThicknessFieldName = "Thickness";
   const char * BottomBoundaryManagerImpl::s_ContCrustalThicknessGridFieldName = "ThicknessGrid";

   const char * BottomBoundaryManagerImpl::s_oceaCrustalThicknessTableName = "OceaCrustalThicknessIoTbl";
   const char * BottomBoundaryManagerImpl::s_OceaCrustalThicknessAgeFieldName = "Age";
   const char * BottomBoundaryManagerImpl::s_OceaCrustalThicknessFieldName = "Thickness";

   const char * BottomBoundaryManagerImpl::s_gridMapTableName = "GridMapIoTbl";
   const char * BottomBoundaryManagerImpl::s_GridMapRefByFieldName = "ReferredBy";



   // Constructor
   BottomBoundaryManagerImpl::BottomBoundaryManagerImpl()
   {
      m_db = nullptr;
      m_basementIoTbl = nullptr;
      m_crustIoTbl = nullptr;
      m_contCrustIoTbl = nullptr;
      m_oceaCrustIoTbl = nullptr;
      m_gridMapIoTbl = nullptr;
   }

   // Copy operator
   BottomBoundaryManagerImpl & BottomBoundaryManagerImpl::operator = (const BottomBoundaryManagerImpl & /*otherFluidMgr*/)
   {
      throw std::runtime_error("Not implemented yet");
      return *this;
   }

   // Set project database. Reset all
   void BottomBoundaryManagerImpl::setDatabase(database::ProjectFileHandlerPtr pfh)
   {
      m_db = pfh;

      m_basementIoTbl = m_db->getTable(s_bottomBoundaryModelTableName);
      m_crustIoTbl = m_db->getTable(s_crustalThicknessTableName);
      m_contCrustIoTbl = m_db->getTable(s_contCrustalThicknessTableName);
      m_oceaCrustIoTbl = m_db->getTable(s_oceaCrustalThicknessTableName);
      m_gridMapIoTbl = m_db->getTable(s_gridMapTableName);
   }
   // Get bottom boundary model
   ErrorHandler::ReturnCode BottomBoundaryManagerImpl::getBottomBoundaryModel(BottomBoundaryModel & BBCModel)
   {

      if (errorCode() != NoError) resetError();
      try
      {
         // if table does not exist - report error
         if (!m_basementIoTbl) { throw Exception(NonexistingID) << s_bottomBoundaryModelTableName << " table could not be found in project"; }

         database::Record * rec = m_basementIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in Basement table: "; }

         const std::string bottomBoundModelName = rec->getValue<std::string>(s_bottomBoundaryModelFieldName);
         if (bottomBoundModelName == "Fixed Temperature") BBCModel = BottomBoundaryModel::BasicCrustThinning;
         else if (bottomBoundModelName == "Advanced Lithosphere Calculator") BBCModel = BottomBoundaryModel::AdvancedCrustThinning;
         else if (bottomBoundModelName == "Improved Lithosphere Calculator Linear Element Mode") BBCModel = BottomBoundaryModel::ImprovedCrustThinningLinear;
         else if (bottomBoundModelName == "Fixed HeatFlow") BBCModel = BottomBoundaryModel::BaseSedimentHeatFlow;
         else { throw Exception(UndefinedValue) << "Unknown bottom boundary model:" << bottomBoundModelName; }

      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

      return NoError;
   }
   // Set bottom boundary model
   ErrorHandler::ReturnCode BottomBoundaryManagerImpl::setBottomBoundaryModel(const BottomBoundaryModel & BBCModel)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         // if table does not exist - report error
         if (!m_basementIoTbl) { throw Exception(NonexistingID) << s_bottomBoundaryModelTableName << " table could not be found in project"; }

         database::Record * rec = m_basementIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in Basement table: "; }

         switch (BBCModel)
         {
         case BottomBoundaryModel::BasicCrustThinning:
            rec->setValue<std::string>(s_bottomBoundaryModelFieldName, "Fixed Temperature");
            break;

         case BottomBoundaryModel::AdvancedCrustThinning:
            rec->setValue<std::string>(s_bottomBoundaryModelFieldName, "Advanced Lithosphere Calculator");
            break;

         case BottomBoundaryModel::ImprovedCrustThinningLinear:
            rec->setValue<std::string>(s_bottomBoundaryModelFieldName, "Improved Lithosphere Calculator Linear Element Mode");
            break;

         case BottomBoundaryModel::BaseSedimentHeatFlow:
            rec->setValue<std::string>(s_bottomBoundaryModelFieldName, "Fixed HeatFlow");
            break;

         default: throw Exception(UndefinedValue) << "Unknown bottom boundary model:" << as_integer(BBCModel /*BottomBoundaryModel::UnknownBottomBoundaryModel*/);//BBCModel
         }
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

      return NoError;
   }

   // Get crust property model
   ErrorHandler::ReturnCode BottomBoundaryManagerImpl::getCrustPropertyModel(CrustPropertyModel & CrustPropModel)
   {

      if (errorCode() != NoError) resetError();
      try
      {
         // if table does not exist - report error
         if (!m_basementIoTbl) { throw Exception(NonexistingID) << s_bottomBoundaryModelTableName << " table could not be found in project"; }

         database::Record * rec = m_basementIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in Basement table: "; }

         const std::string crustPropModelName = rec->getValue<std::string>(s_crustPropModelFieldName);
         if (crustPropModelName == "Legacy Crust") CrustPropModel = CrustPropertyModel::LegacyCrust;
         else if (crustPropModelName == "Low Conductivity Crust") CrustPropModel = CrustPropertyModel::LowCondModel;
         else if (crustPropModelName == "Standard Conductivity Crust") CrustPropModel = CrustPropertyModel::StandardCondModel;
         else { throw Exception(UndefinedValue) << "Unknown crust property model:" << crustPropModelName; }

      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

      return NoError;
   }

   // Set crust property model
   ErrorHandler::ReturnCode BottomBoundaryManagerImpl::setCrustPropertyModel(const CrustPropertyModel & CrustPropModel)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         // if table does not exist - report error
         if (!m_basementIoTbl) { throw Exception(NonexistingID) << s_bottomBoundaryModelTableName << " table could not be found in project"; }

         database::Record * rec = m_basementIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in Basement table: "; }

         switch (CrustPropModel)
         {
         case CrustPropertyModel::LegacyCrust:
            rec->setValue<std::string>(s_crustPropModelFieldName, "Legacy Crust");
            break;

         case CrustPropertyModel::LowCondModel:
            rec->setValue<std::string>(s_crustPropModelFieldName, "Low Conductivity Crust");
            break;

         case CrustPropertyModel::StandardCondModel:
            rec->setValue<std::string>(s_crustPropModelFieldName, "Standard Conductivity Crust");
            break;

         default: throw Exception(UndefinedValue) << "Unknown bottom boundary model:" <<as_integer(CrustPropModel);
         }
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

      return NoError;
   }

   // Get crust property model
   ErrorHandler::ReturnCode BottomBoundaryManagerImpl::getMantlePropertyModel(MantlePropertyModel & MantlePropModel)
   {

      if (errorCode() != NoError) resetError();
      try
      {
         // if table does not exist - report error
         if (!m_basementIoTbl) { throw Exception(NonexistingID) << s_bottomBoundaryModelTableName << " table could not be found in project"; }

         database::Record * rec = m_basementIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in Basement table: "; }

         const std::string mantlePropModelName = rec->getValue<std::string>(s_mantlePropModelFieldName);
         if (mantlePropModelName == "Legacy Mantle") MantlePropModel = MantlePropertyModel::LegacyMantle;
         else if (mantlePropModelName == "Low Conductivity Mantle") MantlePropModel = MantlePropertyModel::LowCondMnModel;
         else if (mantlePropModelName == "Standard Conductivity Mantle") MantlePropModel = MantlePropertyModel::StandardCondMnModel;
         else if (mantlePropModelName == "High Conductivity Mantle") MantlePropModel = MantlePropertyModel::HighCondMnModel;
         else { throw Exception(UndefinedValue) << "Unknown crust property model:" << mantlePropModelName; }

      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

      return NoError;
   }

   // Set crust property model
   ErrorHandler::ReturnCode BottomBoundaryManagerImpl::setMantlePropertyModel(const MantlePropertyModel & MantlePropModel)
   {
      if (errorCode() != NoError) resetError();
      try
      {
         // if table does not exist - report error
         if (!m_basementIoTbl) { throw Exception(NonexistingID) << s_bottomBoundaryModelTableName << " table could not be found in project"; }

         database::Record * rec = m_basementIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in Basement table: "; }

         switch (MantlePropModel)
         {
         case MantlePropertyModel::LegacyMantle:
            rec->setValue<std::string>(s_mantlePropModelFieldName, "Legacy Mantle");
            break;

         case MantlePropertyModel::LowCondMnModel:
            rec->setValue<std::string>(s_mantlePropModelFieldName, "Low Conductivity Mantle");
            break;

         case MantlePropertyModel::StandardCondMnModel:
            rec->setValue<std::string>(s_mantlePropModelFieldName, "Standard Conductivity Mantle");
            break;

         case MantlePropertyModel::HighCondMnModel:
            rec->setValue<std::string>(s_mantlePropModelFieldName, "High Conductivity Mantle");
            break;

         default: throw Exception(UndefinedValue) << "Unknown bottom boundary model:" << as_integer(MantlePropModel);
         }
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }

      return NoError;
   }

   // Get initial lithospheric mantle thickness value
   ErrorHandler::ReturnCode BottomBoundaryManagerImpl::getInitialLithoMantleThicknessValue(double & IniLthMantThic)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_basementIoTbl) { throw Exception(NonexistingID) << s_bottomBoundaryModelTableName << " table could not be found in project"; }
         database::Record * rec = m_basementIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in Basement table: "; }
         IniLthMantThic = rec->getValue<double>(s_initLithManThicknessFieldName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Set initial lithospheric mantle thickness value
   ErrorHandler::ReturnCode BottomBoundaryManagerImpl::setInitialLithoMantleThicknessValue(double & IniLthMantThic)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_basementIoTbl) { throw Exception(NonexistingID) << s_bottomBoundaryModelTableName << " table could not be found in project"; }
         database::Record * rec = m_basementIoTbl->getRecord(static_cast<int>(0));
         if (!rec) { throw Exception(NonexistingID) << "No data found in Basement table: "; }
         rec->setValue<double>(s_initLithManThicknessFieldName, IniLthMantThic);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Get list of time steps defined in the CrustIoTbl
   // return array with IDs of different time steps defined in the model
   std::vector<BottomBoundaryManager::TimeStepID> BottomBoundaryManagerImpl::getTimeStepsID() const
   {
      std::vector<TimeStepID> TimeStpIDs;
      if (m_crustIoTbl)
      {
         // fill IDs array with increasing indexes
         TimeStpIDs.resize(m_crustIoTbl->size(), 0);
         for (size_t i = 0; i < TimeStpIDs.size(); ++i) TimeStpIDs[i] = static_cast<TimeStepID>(i);
      }

      return TimeStpIDs;
   }

   // Get age of a particular time step ID from CrustIoTbl
   ErrorHandler::ReturnCode BottomBoundaryManagerImpl::getAge(const TimeStepID id, double & Age)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_crustIoTbl) { throw Exception(NonexistingID) << s_crustalThicknessTableName << " table could not be found in project"; }
         database::Record * rec = m_crustIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         Age = rec->getValue<double>(s_CrustalThicknessAgeFieldName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Set age of a particular time step ID for CrustIoTbl
   ErrorHandler::ReturnCode BottomBoundaryManagerImpl::setAge(const TimeStepID id, const double Age)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_crustIoTbl) { throw Exception(NonexistingID) << s_crustalThicknessTableName << " table could not be found in project"; }
         database::Record * rec = m_crustIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         rec->setValue<double>(s_CrustalThicknessAgeFieldName, Age);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Get crustal thickness of a particular time step ID for CrustIoTbl
   ErrorHandler::ReturnCode BottomBoundaryManagerImpl::getThickness(const TimeStepID id, double & Thickness)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_crustIoTbl) { throw Exception(NonexistingID) << s_crustalThicknessTableName << " table could not be found in project"; }
         database::Record * rec = m_crustIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         Thickness = rec->getValue<double>(s_CrustalThicknessThicknessFieldName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Set crustal thickness of a particular time step ID for CrustIoTbl
   ErrorHandler::ReturnCode BottomBoundaryManagerImpl::setThickness(const TimeStepID id, const double Thickness)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_crustIoTbl) { throw Exception(NonexistingID) << s_crustalThicknessTableName << " table could not be found in project"; }
         database::Record * rec = m_crustIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         rec->setValue<double>(s_CrustalThicknessThicknessFieldName, Thickness);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Get crustal thickness grid maps of a particular time step ID for CrustIoTbl
   ErrorHandler::ReturnCode BottomBoundaryManagerImpl::getCrustThicknessGrid(const TimeStepID id, std::string & mapName)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_crustIoTbl) { throw Exception(NonexistingID) << s_crustalThicknessTableName << " table could not be found in project"; }
         database::Record * rec = m_crustIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         mapName = rec->getValue<std::string>(s_CrustalThicknessGridFieldName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Set crustal thickness grid map of a particular time step ID for CrustIoTbl
   ErrorHandler::ReturnCode BottomBoundaryManagerImpl::setCrustThicknessGrid(const TimeStepID id, const std::string & mapName)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_crustIoTbl) { throw Exception(NonexistingID) << s_crustalThicknessTableName << " table could not be found in project"; }
         database::Record * rec = m_crustIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         rec->setValue<std::string>(s_CrustalThicknessGridFieldName, mapName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Get list of time steps defined in the continental crustal thickness table
   // return array with IDs of different time steps defined in the model
   std::vector<BottomBoundaryManager::ContCrustTimeStepID> BottomBoundaryManagerImpl::getContCrustTimeStepsID() const
   {
      std::vector<ContCrustTimeStepID> CCTimeStpIDs;
      if (m_contCrustIoTbl)
      {
         // fill IDs array with increasing indexes
         CCTimeStpIDs.resize(m_contCrustIoTbl->size(), 0);
         for (size_t i = 0; i < CCTimeStpIDs.size(); ++i) CCTimeStpIDs[i] = static_cast<ContCrustTimeStepID>(i);
      }

      return CCTimeStpIDs;
   }

   // Get age of a particular time step ID defined in the continental crustal thickness table
   ErrorHandler::ReturnCode BottomBoundaryManagerImpl::getContCrustAge(const ContCrustTimeStepID id, double & Age)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_contCrustIoTbl) { throw Exception(NonexistingID) << s_contCrustalThicknessTableName << " table could not be found in project"; }
         database::Record * rec = m_contCrustIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         Age = rec->getValue<double>(s_ContCrustalThicknessAgeFieldName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Set age of a particular time step ID defined in the continental crustal thickness table
   ErrorHandler::ReturnCode BottomBoundaryManagerImpl::setContCrustAge(const ContCrustTimeStepID id, const double Age)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_contCrustIoTbl) { throw Exception(NonexistingID) << s_contCrustalThicknessTableName << " table could not be found in project"; }
         database::Record * rec = m_contCrustIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         rec->setValue<double>(s_ContCrustalThicknessAgeFieldName, Age);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Get continental crustal thickness value from the continental crustal thickness table table for a particular time step ID
   ErrorHandler::ReturnCode BottomBoundaryManagerImpl::getContCrustThickness(const ContCrustTimeStepID id, double & Thikness)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_contCrustIoTbl) { throw Exception(NonexistingID) << s_contCrustalThicknessTableName << " table could not be found in project"; }
         database::Record * rec = m_contCrustIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         Thikness = rec->getValue<double>(s_ContCrustalThicknessFieldName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Set continental crust thickness value from the continental crustal thickness table table for a particular time step ID
   ErrorHandler::ReturnCode BottomBoundaryManagerImpl::setContCrustThickness(const ContCrustTimeStepID id, const double Thikness)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_contCrustIoTbl) { throw Exception(NonexistingID) << s_contCrustalThicknessTableName << " table could not be found in project"; }
         database::Record * rec = m_contCrustIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         rec->setValue<double>(s_ContCrustalThicknessFieldName, Thikness);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Get continental crustal thickness grid maps of a particular time step ID from the continental crustal thickness table
   ErrorHandler::ReturnCode BottomBoundaryManagerImpl::getContCrustThicknessGrid(const ContCrustTimeStepID id, std::string & mapName)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_contCrustIoTbl) { throw Exception(NonexistingID) << s_contCrustalThicknessTableName << " table could not be found in project"; }
         database::Record * rec = m_contCrustIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         mapName = rec->getValue<std::string>(s_ContCrustalThicknessGridFieldName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Set continental crust thickness grid maps in the continental crustal thickness table for a particular time step ID
   ErrorHandler::ReturnCode BottomBoundaryManagerImpl::setContCrustThicknessGrid(const ContCrustTimeStepID id, const std::string & mapName)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_contCrustIoTbl) { throw Exception(NonexistingID) << s_contCrustalThicknessTableName << " table could not be found in project"; }
         database::Record * rec = m_contCrustIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         rec->setValue<std::string>(s_ContCrustalThicknessGridFieldName, mapName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Get list of time steps defined in the oceanic crustal thickness table
   // return array with IDs of different time steps defined in the model
   std::vector<BottomBoundaryManager::OceaCrustTimeStepID> BottomBoundaryManagerImpl::getOceaCrustTimeStepsID() const
   {
      std::vector<OceaCrustTimeStepID> OCTimeStpIDs;
      if (m_oceaCrustIoTbl)
      {
         // fill IDs array with increasing indexes
         OCTimeStpIDs.resize(m_oceaCrustIoTbl->size(), 0);
         for (size_t i = 0; i < OCTimeStpIDs.size(); ++i) OCTimeStpIDs[i] = static_cast<OceaCrustTimeStepID>(i);
      }

      return OCTimeStpIDs;
   }

   // Get age corresponding to a particular time step ID defined in the oceanic crustal thickness table
   ErrorHandler::ReturnCode BottomBoundaryManagerImpl::getOceaCrustAge(const OceaCrustTimeStepID id, double & Age)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_oceaCrustIoTbl) { throw Exception(NonexistingID) << s_oceaCrustalThicknessTableName << " table could not be found in project"; }
         database::Record * rec = m_oceaCrustIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         Age = rec->getValue<double>(s_OceaCrustalThicknessAgeFieldName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Set age corresponding to a particular time step ID defined in the oceanic crustal thickness table
   ErrorHandler::ReturnCode BottomBoundaryManagerImpl::setOceaCrustAge(const OceaCrustTimeStepID id, const double Age)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_oceaCrustIoTbl) { throw Exception(NonexistingID) << s_oceaCrustalThicknessTableName << " table could not be found in project"; }
         database::Record * rec = m_oceaCrustIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         rec->setValue<double>(s_OceaCrustalThicknessAgeFieldName, Age);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Get oceanic crustal thickness value corresponding to a particular time step ID defined in the oceanic crustal thickness table
   ErrorHandler::ReturnCode BottomBoundaryManagerImpl::getOceaCrustThickness(const OceaCrustTimeStepID id, double & Thickness)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_oceaCrustIoTbl) { throw Exception(NonexistingID) << s_oceaCrustalThicknessTableName << " table could not be found in project"; }
         database::Record * rec = m_oceaCrustIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         Thickness = rec->getValue<double>(s_OceaCrustalThicknessFieldName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Set oceanic crustal thickness value corresponding to a particular time step ID defined in the oceanic crustal thickness table
   ErrorHandler::ReturnCode BottomBoundaryManagerImpl::setOceaCrustThickness(const OceaCrustTimeStepID id, const double Thickness)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_oceaCrustIoTbl) { throw Exception(NonexistingID) << s_oceaCrustalThicknessTableName << " table could not be found in project"; }
         database::Record * rec = m_oceaCrustIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         rec->setValue<double>(s_OceaCrustalThicknessFieldName, Thickness);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Get list of time steps defined in the grid map Io table
   // return array with IDs of different time steps defined in the model
   std::vector<BottomBoundaryManager::GridMapTimeStepID> BottomBoundaryManagerImpl::getGridMapTimeStepsID() const
   {
      std::vector<GridMapTimeStepID> GMTimeStpIDs;
      if (m_gridMapIoTbl)
      {
         // fill IDs array with increasing indexes
         GMTimeStpIDs.resize(m_gridMapIoTbl->size(), 0);
         for (size_t i = 0; i < GMTimeStpIDs.size(); ++i) GMTimeStpIDs[i] = static_cast<GridMapTimeStepID>(i);
      }

      return GMTimeStpIDs;
   }

   // Get table name referred in the grid map Io table for a particular time step ID
   ErrorHandler::ReturnCode BottomBoundaryManagerImpl::getReferredBy(const GridMapTimeStepID id, std::string & mapName)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_gridMapIoTbl) { throw Exception(NonexistingID) << s_gridMapTableName << " table could not be found in project"; }
         database::Record * rec = m_gridMapIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         mapName = rec->getValue<std::string>(s_GridMapRefByFieldName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

   // Set table name referred in the grid map Io table for a particular time step ID
   ErrorHandler::ReturnCode BottomBoundaryManagerImpl::setReferredBy(const GridMapTimeStepID id, const std::string & mapName)
   {
      if (errorCode() != NoError) resetError();

      try
      {
         if (!m_gridMapIoTbl) { throw Exception(NonexistingID) << s_gridMapTableName << " table could not be found in project"; }
         database::Record * rec = m_gridMapIoTbl->getRecord(static_cast<int>(id));
         if (!rec) { throw Exception(NonexistingID) << "No data found with such ID: " << id; }
         rec->setValue<std::string>(s_GridMapRefByFieldName, mapName);
      }
      catch (const Exception & e) { return reportError(e.errorCode(), e.what()); }
      return NoError;
   }

}

