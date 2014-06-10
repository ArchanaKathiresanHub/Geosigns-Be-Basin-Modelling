//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file SourceRockManagerImpl.C
/// @brief This file keeps API implementation for manipulating source rocks in Cauldron model

#include "cmbAPI.h"
#include "SourceRockManagerImpl.h"

#include "database.h"

#include <stdexcept>
#include <string>


namespace mbapi
{

const char * SourceRockManagerImpl::m_sourceRockTableName     = "SourceRockLithoIoTbl";
const char * SourceRockManagerImpl::m_layerNameFieldName      = "LayerName";      
const char * SourceRockManagerImpl::m_sourceRockTypeFieldName = "SourceRockType";
const char * SourceRockManagerImpl::m_tocIni                  = "TocIni";
const char * SourceRockManagerImpl::m_tocIniMap               = "TocIniGrid";
// Constructor
SourceRockManagerImpl::SourceRockManagerImpl()
{
   //throw std::runtime_error( "Not implemented yet" );
   m_db = NULL;
}

// Copy operator
SourceRockManagerImpl & SourceRockManagerImpl::operator = ( const SourceRockManagerImpl & otherSrRockMgr )
{
   throw std::runtime_error( "Not implemented yet" );
   return *this;
}

// Set project database. Reset all
void SourceRockManagerImpl::setDatabase( database::Database * db )
{
   m_db = db;
}

// Get list of source rocks in the model
// return array with IDs of different source rock lithologies defined in the model
std::vector<SourceRockManager::SourceRockID> SourceRockManagerImpl::sourceRockIDs( ) const
{
   std::vector<SourceRockID> srIDs;
   if ( !m_db ) return srIDs;

   // get pointer to the table
   database::Table * table = m_db->getTable( m_sourceRockTableName );

   // if table does not exist - return empty array
   if ( !table ) return srIDs;

   // fill IDs array with increasing indexes
   srIDs.resize( table->size(), 0 );
 
   for ( size_t i = 0; i < srIDs.size(); ++i ) srIDs[ i ] = static_cast<SourceRockID>( i );

   return srIDs;
}

// Create new source rock
// return ID of the new SourceRock
SourceRockManager::SourceRockID SourceRockManagerImpl::createNewSourceRockLithology()
{
   throw std::runtime_error( "Not implemented yet" );
}

// Get layer name for the source rock lithology
std::string SourceRockManagerImpl::layerName( SourceRockID id )
{
   if ( errorCode() != NoError ) resetError();

   std::string layName;

   // get pointer to the table
   database::Table * table = m_db->getTable( m_sourceRockTableName );

   // if table does not exist - report error
   if ( !table )
   {
      reportError( NonexistingID, std::string( m_sourceRockTableName ) + " table could not be found in project" );
      return layName;
   }

   database::Record * rec = table->getRecord( static_cast<int>( id ) );
   if ( !rec )
   {
      reportError( NonexistingID, "No source rock lithology with such ID" );
      return layName;
   }
   layName = rec->getValue<std::string>( m_layerNameFieldName );

   return layName;
}

// Get source rock type name for source rock lithology
std::string SourceRockManagerImpl::sourceRockType( SourceRockID id )
{
   if ( errorCode() != NoError ) resetError();

   std::string tpName;

   // get pointer to the table
   database::Table * table = m_db->getTable( m_sourceRockTableName );

   // if table does not exist - report error
   if ( !table )
   {
      reportError( NonexistingID, std::string( m_sourceRockTableName ) + " table could not be found in project" );
      return tpName;
   }

   database::Record * rec = table->getRecord( static_cast<int>( id ) );
   if ( !rec )
   {
      reportError( NonexistingID, "No source rock lithology with such ID" );
      return tpName;
   }
   tpName = rec->getValue<std::string>( m_sourceRockTypeFieldName );

   return tpName;
}


double SourceRockManagerImpl::tocIni( SourceRockID id )
{
   if ( errorCode() != NoError ) resetError();

   // get pointer to the table
   database::Table * table = m_db->getTable( m_sourceRockTableName );

   // if table does not exist - report error
   if ( !table )
   {
      reportError( NonexistingID, std::string( m_sourceRockTableName ) + " table could not be found in project" );
      return UndefinedDoubleValue;
   }

   // if record does not exist report error
   database::Record * rec = table->getRecord( static_cast<int>( id ) );
   if ( !rec )
   {
      reportError( NonexistingID, "No source rock lithology with such ID" );
      return UndefinedDoubleValue;
   }

   return rec->getValue<double>( m_tocIni );
}

// get layer TOC map name
std::string SourceRockManagerImpl::tocInitMapName( SourceRockID id )
{
   // get pointer to the table
   database::Table * table = m_db->getTable( m_sourceRockTableName );

   // if table does not exist - report error
   if ( !table )
   {
      reportError( NonexistingID, std::string( m_sourceRockTableName ) + " table could not be found in project" );
      return UndefinedStringValue;
   }

   // if record does not exist report error
   database::Record * rec = table->getRecord( static_cast<int>( id ) );
   if ( !rec )
   {
      reportError( NonexistingID, "No source rock lithology with such ID" );
      return UndefinedStringValue;
   }
   return rec->getValue<std::string>( m_tocIniMap );
}

ErrorHandler::ReturnCode SourceRockManagerImpl::setTOCIni( const std::string & layerName, double newTOC )
{
   if ( newTOC < 0.0 || newTOC > 100.0 ) return reportError( OutOfRangeValue, "TOC value must be in range [0:100]" );
   
   // get pointer to the table
   database::Table * table = m_db->getTable( m_sourceRockTableName );

   // if table does not exist - report error
   if ( !table ) return reportError( NonexistingID, std::string( m_sourceRockTableName ) + " table could not be found in project" );
   
   size_t recNum = table->size();
   for ( size_t i = 0; i < recNum; ++i )
   {
      database::Record * rec = table->getRecord(  static_cast<int>( i ) );
      if ( rec )
      {
         const std::string & ln = rec->getValue<std::string>( m_layerNameFieldName );
         if ( ln == layerName )
         {
            rec->setValue( m_tocIni, newTOC );
         }
      }
   }
   return NoError;
}


}