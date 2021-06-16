#include "VoxetProjectHandle.h"
#include "voxetschemafuncs.h"
#include "voxetschema.h"
#include "database.h"
#include "LogHandler.h"

#include "CauldronProperty.h"

VoxetProjectHandle::VoxetProjectHandle ( const std::string& voxetProjectFileName,
                                         const DataAccess::Interface::ProjectHandle& projectHandle ) :
   m_voxetProjectFileName ( voxetProjectFileName ),
   m_cauldronProjectHandle ( projectHandle ) {

   m_voxetSchema = database::createVoxetSchema ();
   m_database = database::Database::CreateFromFile ( voxetProjectFileName, *m_voxetSchema );

   loadSnapshotTime ();
   loadVoxetGrid ( m_cauldronProjectHandle.getLowResolutionOutputGrid ());
   loadCauldronProperties ();
}

//------------------------------------------------------------//

void VoxetProjectHandle::loadCauldronProperties () {

   database::Table* const propTable = m_database->getTable ( "CauldronPropertyIoTbl" );

   assert ( propTable != nullptr );
   if ( propTable == nullptr ) {
      LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << " Cannot load CauldronPropertyIoTbl.";
      return;
   }

   for ( database::Record* record : *propTable ) {
      m_cauldronProperties.push_back ( new CauldronProperty ( m_cauldronProjectHandle, record ));
   }
}

//------------------------------------------------------------//

void VoxetProjectHandle::loadVoxetGrid ( const DataAccess::Interface::Grid* cauldronGrid ) {

   database::Table* voxetGridTable = m_database->getTable ( "VoxetGridIoTbl" );
   database::Table* cauldronGridTable = m_cauldronProjectHandle.getTable ( "ProjectIoTbl" );

   assert ( voxetGridTable != nullptr );
   if ( voxetGridTable == nullptr ) {
      LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << " Cannot load VoxetGridIoTbl.";
      return;
   }

   assert ( cauldronGridTable != nullptr );
   if ( cauldronGridTable == nullptr ) {
      LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << " Cannot load ProjectIoTbl.";
      return;
   }

   database::Record* cauldronRecord = cauldronGridTable->getRecord ( 0 );
   database::Record* voxetRecord = voxetGridTable->getRecord ( 0 );

   assert ( voxetRecord != nullptr );
   if ( voxetRecord == nullptr ) {
      LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << " No data found in VoxetGridIoTbl.";
      return;
   }

   assert ( cauldronRecord != nullptr );
   if ( cauldronRecord == nullptr ) {
      LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << " No data found in ProjectIoTbl.";
      return;
   }

   m_gridDescription = new GridDescription ( cauldronRecord, voxetRecord, cauldronGrid );
}

void VoxetProjectHandle::loadSnapshotTime () {

   database::Table* snapshotTimeTable = m_database->getTable ( "SnapshotTimeIoTbl" );

   if ( snapshotTimeTable == nullptr ) {
      LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << " Cannot load SnapshotTimeIoTbl.";
      return;
   }

   assert ( snapshotTimeTable != nullptr );

   database::Record* snapshotTimeRecord = snapshotTimeTable->getRecord ( 0 );

   if ( snapshotTimeRecord == nullptr )
   {
      m_snapshotTime = 0;
   }
   else
   {
      m_snapshotTime = database::getSnapshotTime (snapshotTimeRecord);
   }
}

//------------------------------------------------------------//

const GridDescription& VoxetProjectHandle::getGridDescription () const {
   return *m_gridDescription;
}

//------------------------------------------------------------//

CauldronPropertyList::iterator VoxetProjectHandle::cauldronPropertyBegin () {
   return m_cauldronProperties.begin ();
}

//------------------------------------------------------------//

CauldronPropertyList::iterator VoxetProjectHandle::cauldronPropertyEnd () {
   return m_cauldronProperties.end ();
}

double VoxetProjectHandle::getSnapshotTime () const {
   return m_snapshotTime;
}

