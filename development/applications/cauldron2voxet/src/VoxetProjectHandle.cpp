#include "VoxetProjectHandle.h"
#include "voxetschemafuncs.h"
#include "voxetschema.h"
#include "database.h"

#include "CauldronProperty.h"

VoxetProjectHandle::VoxetProjectHandle ( const std::string& voxetProjectFileName,
                                         const ProjectHandle& projectHandle ) :
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

   if ( propTable == 0 ) {
      cout << " Cannot load CauldronPropertyIoTbl." << endl;
   }

   assert ( propTable != 0 );

   database::Table::iterator propIter;
   database::Record* propRecord;

   for ( propIter = propTable->begin (); propIter != propTable->end (); ++propIter ) {
      m_cauldronProperties.push_back ( new CauldronProperty ( m_cauldronProjectHandle, this, *propIter ));
   }

}

//------------------------------------------------------------//

void VoxetProjectHandle::loadVoxetGrid ( const Interface::Grid* cauldronGrid ) {

   database::Table* voxetGridTable = m_database->getTable ( "VoxetGridIoTbl" );
   database::Table* cauldronGridTable = m_cauldronProjectHandle.getTable ( "ProjectIoTbl" );

   if ( voxetGridTable == 0 ) {
      cout << " Cannot load VoxetGridIoTbl." << endl;
   }

   assert ( voxetGridTable != 0 );

   if ( cauldronGridTable == 0 ) {
      cout << " Cannot load ProjectIoTbl." << endl;
   }

   assert ( cauldronGridTable != 0 );

   database::Record* cauldronRecord = cauldronGridTable->getRecord ( 0 );
   database::Record* voxetRecord = voxetGridTable->getRecord ( 0 );

   if ( voxetRecord == 0 ) {
      cout << " No data found in VoxetGridIoTbl." << endl;
   }

   if ( cauldronRecord == 0 ) {
      cout << " No data found in ProjectIoTbl." << endl;
   }

   assert ( cauldronRecord != 0 );
   assert ( voxetRecord != 0 );

   m_gridDescription = new GridDescription ( cauldronRecord, voxetRecord, cauldronGrid );
}

void VoxetProjectHandle::loadSnapshotTime () {

   database::Table* snapshotTimeTable = m_database->getTable ( "SnapshotTimeIoTbl" );

   if ( snapshotTimeTable == 0 ) {
      cout << " Cannot load SnapshotTimeIoTbl." << endl;
   }

   assert ( snapshotTimeTable != 0 );

   database::Record* snapshotTimeRecord = snapshotTimeTable->getRecord ( 0 );

   if ( snapshotTimeRecord == 0 )
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


