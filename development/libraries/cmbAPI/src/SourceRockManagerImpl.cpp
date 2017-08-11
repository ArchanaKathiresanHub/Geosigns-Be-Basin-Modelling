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

// CMB API
#include "cmbAPI.h"
#include "SourceRockManagerImpl.h"

// TableIO lib
#include "database.h"

// Genex6_kernel lib
#include "SourceRock.h"

// STL
#include <stdexcept>
#include <string>

namespace mbapi
{
const char * SourceRockManagerImpl::s_sourceRockTableName     = "SourceRockLithoIoTbl";
const char * SourceRockManagerImpl::s_layerNameFieldName      = "LayerName";
const char * SourceRockManagerImpl::s_sourceRockTypeFieldName = "SourceRockType";
const char * SourceRockManagerImpl::s_tocIni                  = "TocIni";
const char * SourceRockManagerImpl::s_tocIniMap               = "TocIniGrid";
const char * SourceRockManagerImpl::s_hcIni                   = "HcVRe05";
const char * SourceRockManagerImpl::s_PreAsphaltStartAct      = "PreAsphaltStartAct";

// Constructor
SourceRockManagerImpl::SourceRockManagerImpl()
{
   //throw std::runtime_error( "Not implemented yet" );
   m_db = NULL;
}

// Copy operator
SourceRockManagerImpl & SourceRockManagerImpl::operator = ( const SourceRockManagerImpl & /*otherSrRockMgr*/ )
{
   throw ErrorHandler::Exception( ErrorHandler::NotImplementedAPI ) << "SourceRockManagerImpl::operator = () not implemented yet";
   return *this;
}

// Set project database. Reset all
void SourceRockManagerImpl::setDatabase( database::ProjectFileHandlerPtr pfh )
{
   m_db = pfh;
}

// Get list of source rocks in the model
// return array with IDs of different source rock lithologies defined in the model
std::vector<SourceRockManager::SourceRockID> SourceRockManagerImpl::sourceRockIDs( ) const
{
   std::vector<SourceRockID> srIDs;
   if ( !m_db ) return srIDs;

   // get pointer to the table
   database::Table * table = m_db->getTable( s_sourceRockTableName );

   // if table does not exist - return empty array
   if ( !table ) return srIDs;

   // fill IDs array with increasing indexes
   srIDs.resize( table->size(), 0 );

   for ( size_t i = 0; i < srIDs.size(); ++i ) srIDs[ i ] = static_cast<SourceRockID>( i );

   return srIDs;
}

// Search for source rock lithology record which has given layer name and source rock type name
// return ID of found source rock lithology on success or Utilities::Numerical::NoDataIDValue otherwise
SourceRockManager::SourceRockID SourceRockManagerImpl::findID( const std::string & lName
                                                             , const std::string & srTypeName
                                                             )
{
   if ( errorCode() != NoError ) resetError();
   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( s_sourceRockTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << s_sourceRockTableName << " table could not be found in project";
      }

      size_t tblSize = table->size();
      for ( size_t i = 0; i < tblSize; ++i )
      {
         database::Record * rec = table->getRecord( static_cast<int>( i ) );
         if ( !rec )
         {
            throw Exception( NonexistingID ) << "No source rock lithology with such ID: " << i;
         }

         if ( lName      == rec->getValue<std::string>( s_layerNameFieldName ) &&
              srTypeName == rec->getValue<std::string>( s_sourceRockTypeFieldName )
            )
         {
            return static_cast<SourceRockManager::SourceRockID>(i);
         }
      }
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return Utilities::Numerical::NoDataIDValue;
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

   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( s_sourceRockTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << s_sourceRockTableName << " table could not be found in project";
      }

      database::Record * rec = table->getRecord( static_cast<int>(id) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No source rock lithology with such ID: " << id;
      }
      layName = rec->getValue<std::string>( s_layerNameFieldName );
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return layName;
}

// Get source rock type name for source rock lithology
std::string SourceRockManagerImpl::sourceRockType( SourceRockID id )
{
   if ( errorCode() != NoError ) resetError();
   std::string tpName;

   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( s_sourceRockTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << s_sourceRockTableName << " table could not be found in project";
      }

      database::Record * rec = table->getRecord( static_cast<int>(id) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No source rock lithology with such ID: " << id;
      }
      tpName = rec->getValue<std::string>( s_sourceRockTypeFieldName );
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return tpName;
}


double SourceRockManagerImpl::tocIni( SourceRockID id )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( s_sourceRockTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << s_sourceRockTableName << " table could not be found in project";
      }

      // if record does not exist report error
      database::Record * rec = table->getRecord( static_cast<int>(id) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No source rock lithology with such ID: " << id;
      }

      return rec->getValue<double>( s_tocIni );
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }
   return Utilities::Numerical::IbsNoDataValue;
}

// get layer TOC map name
std::string SourceRockManagerImpl::tocInitMapName( SourceRockID id )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( s_sourceRockTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << s_sourceRockTableName << " table could not be found in project";
      }

      // if record does not exist report error
      database::Record * rec = table->getRecord( static_cast<int>(id) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No source rock lithology with such ID: " << id;
      }
      return rec->getValue<std::string>( s_tocIniMap );
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return Utilities::Numerical::NoDataStringValue;
}

ErrorHandler::ReturnCode SourceRockManagerImpl::setTOCIni( SourceRockID id, double newTOC )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      if ( (newTOC < 0.0 || newTOC > 100.0 ) && !IsValueUndefined( newTOC ) )
      {
         throw Exception( OutOfRangeValue ) << "TOC value must be in range [0:100] but given is: " << newTOC;
      }
      // get pointer to the table
      database::Table * table = m_db->getTable( s_sourceRockTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << s_sourceRockTableName << " table could not be found in project";
      }

      database::Record * rec = table->getRecord(  static_cast<int>( id ) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No source rock lithology with such ID: " << id;
      }
      rec->setValue( s_tocIni, newTOC );
   }
   catch ( const Exception & e ) { return reportError( e.errorCode(), e.what() ); }

   return NoError;
}

ErrorHandler::ReturnCode SourceRockManagerImpl::setTOCInitMapName( SourceRockID id, const std::string & mapName )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( s_sourceRockTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << s_sourceRockTableName << " table could not be found in project";
      }

      database::Record * rec = table->getRecord(  static_cast<int>( id ) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No source rock lithology with such ID: " << id;
      }
      rec->setValue( s_tocIniMap, mapName );
      rec->setValue( s_tocIni,    Utilities::Numerical::IbsNoDataValue ); // TOC must be -9999
   }
   catch ( const Exception & e ) { return reportError( e.errorCode(), e.what() ); }

   return NoError;
}

double SourceRockManagerImpl::hiIni( SourceRockID id )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( s_sourceRockTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << s_sourceRockTableName << " table could not be found in project";
      }

      // if record does not exist report error
      database::Record * rec = table->getRecord( static_cast<int>(id) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No source rock lithology with such ID: " << id;
      }

      double hcIni = rec->getValue<double>( s_hcIni );
      return Genex6::SourceRock::convertHCtoHI( hcIni );
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return Utilities::Numerical::IbsNoDataValue;
}

ErrorHandler::ReturnCode SourceRockManagerImpl::setHIIni( SourceRockID id, double newHI )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      if ( newHI < 0.0 || newHI > 1000.0 )
      {
         throw Exception( OutOfRangeValue ) << "HI value must be in range [0:1000], but given is: " << newHI;
      }
      // get pointer to the table
      database::Table * table = m_db->getTable( s_sourceRockTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << s_sourceRockTableName << " table could not be found in project";
      }

      database::Record * rec = table->getRecord( static_cast<int>( id ) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No source rock lithology with such ID: " << id;
      }

      double hcIni = Genex6::SourceRock::convertHItoHC( newHI );
      rec->setValue( s_hcIni, hcIni );
   }
   catch ( const Exception & e ) { return reportError( e.errorCode(), e.what() ); }

   return NoError;
}

double SourceRockManagerImpl::hcIni( SourceRockID id )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( s_sourceRockTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << s_sourceRockTableName << " table could not be found in project";
      }

      // if record does not exist report error
      database::Record * rec = table->getRecord( static_cast<int>(id) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No source rock lithology with such ID: " << id;
      }

      return rec->getValue<double>( s_hcIni );
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return Utilities::Numerical::IbsNoDataValue;
}

ErrorHandler::ReturnCode SourceRockManagerImpl::setHCIni( SourceRockID id, double newHC )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      if ( newHC < 0.0 || newHC > 2.0 )
      {
         throw Exception( OutOfRangeValue ) << "H/C value must be in range [0:2] but given is: " << newHC;
      }

      // get pointer to the table
      database::Table * table = m_db->getTable( s_sourceRockTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << s_sourceRockTableName << " table could not be found in project";
      }

      database::Record * rec = table->getRecord( static_cast<int>(id) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No source rock lithology with such ID: " << id;
      }
      rec->setValue( s_hcIni, newHC );
   }
   catch ( const Exception & e ) { return reportError( e.errorCode(), e.what() ); }

   return NoError;
}

// Get pre-asphaltene activation energy [kJ/mol]
double SourceRockManagerImpl::preAsphActEnergy( SourceRockID id )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( s_sourceRockTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << s_sourceRockTableName << " table could not be found in project";
      }

      // if record does not exist report error
      database::Record * rec = table->getRecord( static_cast<int>(id) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No source rock lithology with such ID: " << id;
      }

      return rec->getValue<double>( s_PreAsphaltStartAct );
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return Utilities::Numerical::IbsNoDataValue;
}

// Set pre-asphaltene activation energy (must be in range 200-220 kJ/mol)
ErrorHandler::ReturnCode SourceRockManagerImpl::setPreAsphActEnergy( SourceRockID id, double newVal )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      if ( newVal < 100.0 || newVal > 300.0 )
      {
         throw Exception( OutOfRangeValue ) << "pre-asphaltene activation energy  value must be in range [100:300]" <<
                                               " but given is: " << newVal;
      }

      // get pointer to the table
      database::Table * table = m_db->getTable( s_sourceRockTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << s_sourceRockTableName << " table could not be found in project";
      }

      database::Record * rec = table->getRecord( static_cast<int>(id) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No source rock lithology with such ID: " << id;
      }
      rec->setValue( s_PreAsphaltStartAct, newVal );
   }
   catch ( const Exception & e ) { return reportError( e.errorCode(), e.what() ); }

   return NoError;
}


}
