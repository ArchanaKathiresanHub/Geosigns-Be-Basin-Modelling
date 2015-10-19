//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file SnapshotManagerImpl.C
/// @brief This file keeps implementation for API which provides access to simulation results

#include "SnapshotManagerImpl.h"
#include "cmbAPI.h"

#include "database.h"

#include "cauldronschemafuncs.h"

#include <cmath>

namespace mbapi
{

double SnapshotManagerImpl::s_snpTol = 1.e-4;

SnapshotManagerImpl::SnapshotManagerImpl()
{
   m_db       = NULL;
   m_snpTable = NULL;
}

// Set project database. Reset all
void SnapshotManagerImpl::setDatabase( database::Database * db, const std::string & projName )
{
   m_db = db;
   m_snpTable = m_db->getTable( "SnapshotIoTbl" );
   //m_projHndl.reset( DataAccess::Mining::ProjectHandle( m_db, projName, "r" ) );
}


// Get number of snapshots in project
size_t SnapshotManagerImpl::snapshotsNumber()
{
   if ( !m_snpTable ) return 0;
   return m_snpTable->end() - m_snpTable->begin();
}

// Get time for the i-th snapshot
double SnapshotManagerImpl::time( size_t i )
{
   return database::getTime( (*m_snpTable)[static_cast<int>(i)] );
}

// Ask, is i-th snapshot minor?
bool SnapshotManagerImpl::isMinor( size_t i )
{
   return database::getIsMinorSnapshot( (*m_snpTable)[static_cast<int>(i)] ) ? true : false;
}


// Get snapshot type
std::string SnapshotManagerImpl::type( size_t i )
{
   return database::getTypeOfSnapshot( (*m_snpTable)[static_cast<int>(i)] );
}


// Get data file name for i-th snapshot
std::string SnapshotManagerImpl::fileName( size_t i )
{
   return database::getSnapshotFileName( (*m_snpTable)[static_cast<int>(i)] );
}


static bool SnapshotIoTblSorter( database::Record * recordL, database::Record * recordR )
{
   if ( database::getTime( recordL ) < database::getTime( recordR ) ) return true;

   return false;
}

ErrorHandler::ReturnCode SnapshotManagerImpl::requestMajorSnapshot( double simTime )
{
   if ( simTime < 1e-5 ) return NoError; // 0.0 always be created

   // add major snapshot if it is not exists already
   if ( !m_db ) throw Exception(UndefinedValue) << "Project was not loaded in to the model";

   // if table does not exist - report error
   if ( !m_snpTable ) throw Exception( UndefinedValue ) << "SnapshotIoTbl table could not be found in project";

   database::Record * record = NULL;

   for ( database::Table::iterator it = m_snpTable->begin(); !record && it != m_snpTable->end(); ++it )
   {
      if ( std::abs(simTime - database::getTime( *it ) ) < s_snpTol )
      {
         record = *it;
      }
   }

   if ( record )
   {
      // check is it a major snapshot?
      if ( database::getIsMinorSnapshot( record ) )
      {
         database::setIsMinorSnapshot( record, 0 ); // change snapshot to major
      }
   }
   else // can't find
   {
      database::Record * record = m_snpTable->createRecord();
      database::setTime( record, simTime );
      database::setIsMinorSnapshot( record, 0 );
      database::setTypeOfSnapshot( record, "User Defined" );
      database::setSnapshotFileName( record, "" );
      m_snpTable->sort( SnapshotIoTblSorter );
   }
   return NoError;
}

}

