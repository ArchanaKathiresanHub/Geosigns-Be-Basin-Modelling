//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file PropertyManagerImpl.C
/// @brief This file keeps implementation for API to

// CMB API
#include "PropertyManagerImpl.h"
#include "cmbAPI.h"

// FileSystem library
#include "FilePath.h"
#include "FolderPath.h"

#include "database.h"

#include "cauldronschemafuncs.h"

// STL
#include <set>

namespace mbapi
{

PropertyManagerImpl::PropertyManagerImpl()
{
   m_db           = NULL;
   m_fltTimeTable = NULL;
}

// Set project database. Reset all
void PropertyManagerImpl::setDatabase( database::ProjectFileHandlerPtr db, const std::string & /*projName*/ )
{
   m_db = db;
   m_fltTimeTable    = m_db->getTable( "FilterTimeIoTbl" );
   m_snapshotIoTable = m_db->getTable( "SnapshotIoTbl" );
   m_timeIoTable     = m_db->getTable( "TimeIoTbl" );
   m_3dTimeIoTable   = m_db->getTable( "3DTimeIoTbl" );
}

ErrorHandler::ReturnCode PropertyManagerImpl::requestPropertyInSnapshots( const std::string & propName, const std::string & outputPropOption )
{
   // add major snapshot if it is not exists already
   if ( !m_db ) throw Exception(UndefinedValue) << "Project was not loaded in to the model";

   // if table does not exist - report error
   if ( !m_fltTimeTable ) throw Exception( UndefinedValue ) << "FilterTimeIoTbl table could not be found in project";

   database::Record * record3d = NULL;
   database::Record * record1d = NULL;

   for ( database::Table::iterator it = m_fltTimeTable->begin(); it != m_fltTimeTable->end(); ++it )
   {
      if ( database::getPropertyName( *it ) == propName )
      {
         if ( database::getModellingMode( *it ) == "3d" )
         {
            record3d = *it;
         }
         else if ( database::getModellingMode( *it ) == "1d" )
         {
            record1d = *it;
         }
      }
      if ( record1d && record3d ) break; // do not to scan all records
   }

   const std::string & oo = outputPropOption.empty() ? outputOptionForProperty( propName ) : outputPropOption; // get output option for the property

   if ( record3d )
   {
      if ( database::getOutputOption( record3d ) == "None" ) // if save was not requested
      {
         if ( record1d && database::getOutputOption( record1d ) != "None" ) // special case if 1d is set and 3d isn't
         {
            database::setOutputOption( record3d, database::getOutputOption( record1d ) ); // copy from 1d
         }
         else
         {
            database::setOutputOption( record3d, oo ); // for implemented properties provide outputOption
            if ( record1d ) database::setOutputOption( record1d, oo );
         }
      }
   }
   else // can't find
   {
      record3d = m_fltTimeTable->createRecord();
      record1d = m_fltTimeTable->createRecord();

      database::setPropertyName( record3d, propName );
      database::setPropertyName( record1d, propName );

      database::setModellingMode( record3d, "3d" );
      database::setModellingMode( record1d, "1d" );


      database::setOutputOption( record3d, oo );
      database::setOutputOption( record1d, oo );

      database::setResultOption( record3d, "Simple" );
      database::setResultOption( record1d, "Simple" );
   }
   return NoError;
}


ErrorHandler::ReturnCode PropertyManagerImpl::copyResultsFiles( const std::string & oldProject, const std::string & newProject, bool copyFiles )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      std::set<std::string> fileList;

      // collect .HDF files in a set
      for ( size_t t = 0; t < 3; ++t )
      {
         database::Table * tbl = NULL;
         std::string       colName;
         switch ( t )
         {
            case 0: tbl = m_snapshotIoTable; colName = "SnapshotFileName"; break;
            case 1: tbl = m_timeIoTable;     colName = "MapFileName";      break;
            case 2: tbl = m_3dTimeIoTable;   colName = "MapFileName";      break;
         }
         if ( !tbl ) continue;

         size_t tblSize = m_snapshotIoTable->size();
         for ( size_t i = 0; i < tblSize; ++i )
         {
            database::Record * rec = tbl->getRecord( static_cast<int>( i ) );
            if ( !rec ) continue;

            const std::string & fileName = rec->getValue<std::string>( colName );
            if ( fileName.empty() ) continue;
            if ( !fileList.count( fileName ) ) fileList.insert( fileName );
         }
      }

      // construct full paths to the results files
      ibs::FilePath oldProjectPath( oldProject );
      const std::string & oldProjectName = oldProjectPath.fileNameNoExtension();
      oldProjectPath.cutLast(); // cut project file name
      oldProjectPath << oldProjectName + Model::s_ResultsFolderSuffix;

      ibs::FilePath newProjectPath( newProject );
      const std::string & newProjectName  = newProjectPath.fileNameNoExtension();
      newProjectPath.cutLast(); // cut project file name
      newProjectPath << newProjectName + Model::s_ResultsFolderSuffix;

      // create new folder for results files if it doesn't exist
      if ( !newProjectPath.exists() ) ibs::FolderPath( newProjectPath.path() ).create();

      if ( !oldProjectPath.exists() && !fileList.empty() )
      {
         throw Exception( IoError ) << "Copy results files failed: no such folder: " << oldProjectPath.path();
      }

      for ( std::set<std::string>::const_iterator it = fileList.begin(); it != fileList.end(); ++it )
      {
         ibs::FilePath oldResFile( oldProjectPath.path() );
         oldResFile << *it;
         if ( !oldResFile.exists() ) throw Exception( IoError ) << "Copy results files failed: no such file: " << oldResFile.path();
         ibs::FilePath newResFile( newProjectPath.path() );
         newResFile << *it;
         if ( newResFile.exists() ) throw Exception( IoError ) << "Copy results files failed: file already exists: " << newResFile.path();

         bool copied = true;
         // for Time_*.h5/*_Results files make only links
         if (      !(*it).compare( 0, 5,  "Time_", 0, 5 ) )
         {
            copied = copyFiles ? oldResFile.copyFile( newResFile.fullPath() ) : oldResFile.linkFile( newResFile.fullPath() );
         }
         else if ( !(*it).compare( 0, 25, "HydrostaticTemperature_Results", 0, 25 ) )
         {
            copied = copyFiles ? oldResFile.copyFile( newResFile.fullPath() ) : oldResFile.linkFile( newResFile.fullPath() );
         }
         else if ( !(*it).compare( 0, 25, "PressureAndTemperature_Results", 0, 25 ) )
         {
            copied = copyFiles ? oldResFile.copyFile( newResFile.fullPath() ) : oldResFile.linkFile( newResFile.fullPath() );
         }
         else
         {
            copied = oldResFile.copyFile( newResFile.fullPath() );
         }

         if ( !copied ) throw Exception( IoError )  << "Can't copy file: " << oldResFile.path() << " to " << newResFile.path();
      }
   }
   catch( const Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }

   return NoError;
}

std::string PropertyManagerImpl::outputOptionForProperty( const std::string & propName )
{
   if ( propName == "Vr" ||
        propName == "VrVec"               ) return "SedimentsOnly";
   if ( propName == "Temperature"         ) return "SedimentsPlusBasement";
   if ( propName.find( "Pressure"     ) != std::string::npos ) return "SedimentsOnly"; // all pressures like LithoStaticPressure, HydroStaticPressure, OverPressure
   if ( propName.find( "Permeability" ) != std::string::npos ) return "SedimentsOnly";
   if ( propName.find( "Permeability" ) != std::string::npos ) return "SedimentsOnly";
   if ( propName.find( "Sonic"        ) != std::string::npos ) return "SedimentsOnly";
   if ( propName.find( "Velocity"     ) != std::string::npos ) return "SedimentsOnly";

   // TODO add all properties
   return "SedimentsPlusBasement"; // just in case for unimplemented properties
}

}
