//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file PropertyManagerImpl.h
/// @brief This file keeps implementation declarations for API which deals with property requests

#ifndef CMB_PROPERTY_MANAGER_IMPL
#define CMB_PROPERTY_MANAGER_IMPL

#include "ProjectFileHandler.h"

#include "PropertyManager.h"

#include <string>

namespace database
{
   class Database;
   class Table;
}

namespace DataAccess
{
   namespace Mining
   {
      class ProjectHandle;
   }
}

namespace mbapi
{
   /// @brief Class which defines implementation of API to manage FilterTimeIoTable in project file
   class PropertyManagerImpl : public PropertyManager
   {
   public:
      // Constructor
      PropertyManagerImpl();

      /// @brief Destructor
      virtual ~PropertyManagerImpl() { ; }

      // Set project database. Reset all
      void setDatabase( database::ProjectFileHandlerPtr db, const std::string & projName );

      /// @brief Check if properties table has given property in active state. If given property
      ///        doesn't exist in table it adds corresponded record, if property isn't active - makes it active
      /// @param propName name of the requested property
      /// @param outputPropOption in which layers option should be requested, for example: None/SourceRockOnly/SedimentsOnly...
      /// @return ErrorHandler::NoError on success, error code otherwise
      virtual ErrorHandler::ReturnCode requestPropertyInSnapshots( const std::string & propName, const std::string & outputPropOption = "" );

      /// @brief Copy all .HDF results files from old project path to the new project path
      /// @param oldProject old project name with full path
      /// @param newProject new project name with full path
      /// @param copyFiles if copyFiles is false - results files will be linked where it is possible otherwise they will be copied
      /// @return ErrorHandler::NoError on success, error code otherwise
      ErrorHandler::ReturnCode copyResultsFiles( const std::string & oldProject, const std::string & newProject, bool copyFiles = false );

   private:
      database::ProjectFileHandlerPtr                   m_db;              // project database
      database::Table                                 * m_fltTimeTable;    // FilterTimeIo table
      database::Table                                 * m_snapshotIoTable; // SnapshotIoTbl
      database::Table                                 * m_timeIoTable;     // TimeIoTbl
      database::Table                                 * m_3dTimeIoTable;   // 3DTimeIoTbl


      // None, SourceRockOnly, SedimentsOnly, SedimentsPlusBasement depends on property name
      std::string outputOptionForProperty( const std::string & propName );

      PropertyManagerImpl( const PropertyManagerImpl & );
      PropertyManagerImpl & operator = ( const PropertyManagerImpl & );
   };
}

#endif // CMB_SNAPSHOT_MANAGER_IMPL
