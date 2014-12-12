//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PropertyManagerImpl.C
/// @brief This file keeps implementation for API to 

#include "PropertyManagerImpl.h"
#include "cmbAPI.h"

#include "database.h"

#include "cauldronschemafuncs.h"

namespace mbapi
{

PropertyManagerImpl::PropertyManagerImpl()
{
   m_db           = NULL;
   m_fltTimeTable = NULL;
}

// Set project database. Reset all
void PropertyManagerImpl::setDatabase( database::Database * db, const std::string & projName )
{
   m_db = db;
   m_fltTimeTable = m_db->getTable( "FilterTimeIoTbl" );
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

