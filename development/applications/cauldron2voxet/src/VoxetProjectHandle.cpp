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

   database::Table* propTable = m_database->getTable ( "CauldronPropertyIoTbl" );

   if ( propTable == nullptr ) {
      LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << " Cannot load CauldronPropertyIoTbl.";
   }

   assert ( propTable != nullptr );

   database::Table::iterator propIter;

   for ( propIter = propTable->begin (); propIter != propTable->end (); ++propIter ) {
      m_cauldronProperties.push_back ( new CauldronProperty ( m_cauldronProjectHandle, *propIter ));
   }

}

//------------------------------------------------------------//

void VoxetProjectHandle::loadVoxetGrid ( const DataAccess::Interface::Grid* cauldronGrid ) {

   database::Table* voxetGridTable = m_database->getTable ( "VoxetGridIoTbl" );
   database::Table* cauldronGridTable = m_cauldronProjectHandle.getTable ( "ProjectIoTbl" );

   if ( voxetGridTable == nullptr ) {
      LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << " Cannot load VoxetGridIoTbl.";
   }

   assert ( voxetGridTable != nullptr );

   if ( cauldronGridTable == nullptr ) {
      LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << " Cannot load ProjectIoTbl.";
   }
   assert ( cauldronGridTable != nullptr );

   database::Record* cauldronRecord = cauldronGridTable->getRecord ( 0 );
   database::Record* voxetRecord = voxetGridTable->getRecord ( 0 );

   if ( voxetRecord == nullptr ) {
      LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << " No data found in VoxetGridIoTbl.";
   }

   if ( cauldronRecord == nullptr ) {
      LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << " No data found in ProjectIoTbl.";
   }

   assert ( cauldronRecord != nullptr );
   assert ( voxetRecord != nullptr );

   m_gridDescription = new GridDescription ( cauldronRecord, voxetRecord, cauldronGrid );
}

void VoxetProjectHandle::loadSnapshotTime () {

   database::Table* snapshotTimeTable = m_database->getTable ( "SnapshotTimeIoTbl" );

   if ( snapshotTimeTable == nullptr ) {
      LogHandler(LogHandler::ERROR_SEVERITY, LogHandler::DEFAULT) << " Cannot load SnapshotTimeIoTbl.";
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

//------------------------------------------------------------//

bool VoxetProjectHandle::isConsistent () const {
   // Perform checks on:
   //     o input properties, do all properties exist;
   //     o Formation names, are all formation mentioned and those that are, are they in the cauldron project file;
   //     o Function names, that formations to not access a function that does not exist;
   //     o anything else?
   return true;
}

//------------------------------------------------------------//


