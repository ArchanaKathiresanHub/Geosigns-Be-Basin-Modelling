#include "VoxetProjectHandle.h"
#include "voxetschemafuncs.h"
#include "voxetschema.h"
#include "database.h"

#include "DerivedPropertyFunction.h"
#include "CauldronProperty.h"
#include "DerivedProperty.h"
#include "DerivedPropertyFormationFunction.h"

VoxetProjectHandle::VoxetProjectHandle ( const std::string& voxetProjectFileName,
                                         Interface::ProjectHandle*     projectHandle ) :
   m_voxetProjectFileName ( voxetProjectFileName ),
   m_cauldronProjectHandle ( projectHandle ) {

   m_voxetSchema = database::createVoxetSchema ();
   m_database = database::Database::CreateFromFile ( voxetProjectFileName, *m_voxetSchema );

   loadSnapshotTime ();
   loadVoxetGrid ( m_cauldronProjectHandle->getLowResolutionOutputGrid ());
   loadCauldronProperties ();
   loadDerivedProperties ();
   loadDerivedPropertyFunctions ();
   loadDerivedPropertyFormationFunctions ();

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
   database::Table* cauldronGridTable = m_cauldronProjectHandle->getTable ( "ProjectIoTbl" );

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

void VoxetProjectHandle::loadDerivedProperties () {

   database::Table* propTable = m_database->getTable ( "DerivedPropertyIoTbl" );

   if ( propTable == 0 ) {
      cout << " Cannot load DerivedPropertyIoTbl." << endl;
   } 

   assert ( propTable != 0 );

   database::Table::iterator propIter;
   database::Record* propRecord;

   for ( propIter = propTable->begin (); propIter != propTable->end (); ++propIter ) {
      m_derivedProperties.push_back ( new DerivedProperty ( m_cauldronProjectHandle, this, *propIter ));
   }

   if ( m_derivedProperties.size () == 0 ) {
      cout << " No data found in DerivedPropertyIoTbl." << endl;
   } 

   assert ( m_derivedProperties.size () >= 0 );
}

//------------------------------------------------------------//

void VoxetProjectHandle::loadDerivedPropertyFunctions () {

   database::Table* funcTable = m_database->getTable ( "DerivedPropertyFunctionIoTbl" );
   database::Table::iterator funcIter;
   database::Record* funcRecord;

   if ( funcTable == 0 ) {
      cout << " Cannot load DerivedPropertyFunctionIoTbl." << endl;
   } 

   assert ( funcTable != 0 );

   for ( funcIter = funcTable->begin (); funcIter != funcTable->end (); ++funcIter ) {
      m_derivedPropertyFunctions.push_back ( new DerivedPropertyFunction ( *funcIter, m_derivedPropertyFunctions ));
   }
}

//------------------------------------------------------------//

void VoxetProjectHandle::loadDerivedPropertyFormationFunctions () {

   database::Table* funcTable = m_database->getTable ( "DerivedPropertyFormationFunctionIoTbl" );
   database::Table::iterator funcIter;
   database::Record* funcRecord;

   if ( funcTable == 0 ) {
      cout << " Cannot load DerivedPropertyFormationFunctionIoTbl." << endl;
   } 

   assert ( funcTable != 0 );

   for ( funcIter = funcTable->begin (); funcIter != funcTable->end (); ++funcIter ) {
      m_derivedPropertyFormationFunctions.push_back ( new DerivedPropertyFormationFunction ( this, *funcIter ));
   }

}

//------------------------------------------------------------//

DerivedProperty* VoxetProjectHandle::getDerivedProperty ( const std::string& propertyName ) const {

   DerivedPropertyList::const_iterator propIter;

   for ( propIter = m_derivedProperties.begin (); propIter != m_derivedProperties.end (); ++propIter ) {

      if ((*propIter)->getName () == propertyName ) {
         return *propIter;
      }

   }

   return 0;
}

//------------------------------------------------------------//

DerivedPropertyFunctionList* VoxetProjectHandle::getDerivedPropertyFunctions ( const DerivedProperty* property ) const {

   DerivedPropertyFormationFunctionList::const_iterator formFuncIter;
   DerivedPropertyFunctionList* functions = new DerivedPropertyFunctionList;

   for ( formFuncIter = m_derivedPropertyFormationFunctions.begin (); formFuncIter != m_derivedPropertyFormationFunctions.end (); ++formFuncIter ) {

      if ((*formFuncIter)->getDerivedPropertyName () == property->getName ()) {
         DerivedPropertyFunction* func = getDerivedPropertyFunction ((*formFuncIter)->getFunctionName ());

         if ( func != 0 ) {
            functions->push_back ( func );
         }

      }

   }

   return functions;
}

//------------------------------------------------------------//

DerivedPropertyFormationFunction* VoxetProjectHandle::getWaterColumnDerivedPropertyFormationFunction ( const DerivedProperty* property ) const {

   DerivedPropertyFormationFunctionList::const_iterator formFuncIter;

   for ( formFuncIter = m_derivedPropertyFormationFunctions.begin (); formFuncIter != m_derivedPropertyFormationFunctions.end (); ++formFuncIter ) {

      if ((*formFuncIter)->getDerivedPropertyName () == property->getName () and (*formFuncIter)->getFormationName () == WaterColumnFormationName ) {
         return *formFuncIter;
      }

   }

   return 0;
}

//------------------------------------------------------------//

DerivedPropertyFormationFunctionList* VoxetProjectHandle::getDerivedPropertyFormationFunctions ( const DerivedProperty* property ) const {

   DerivedPropertyFormationFunctionList::const_iterator formFuncIter;
   DerivedPropertyFormationFunctionList* formations = new DerivedPropertyFormationFunctionList;

   for ( formFuncIter = m_derivedPropertyFormationFunctions.begin (); formFuncIter != m_derivedPropertyFormationFunctions.end (); ++formFuncIter ) {

      if ((*formFuncIter)->getDerivedPropertyName () == property->getName ()) {
         formations->push_back ( *formFuncIter );
      }

   }

   return formations;
}

//------------------------------------------------------------//

DerivedPropertyFunction* VoxetProjectHandle::getDerivedPropertyFunction ( const std::string& functionName ) const {

   DerivedPropertyFunctionList::const_iterator funcIter;

   for ( funcIter = m_derivedPropertyFunctions.begin (); funcIter != m_derivedPropertyFunctions.end (); ++funcIter ) {

      if (( *funcIter)->getName () == functionName ) {
         return *funcIter;
      }

   }

   return 0;
}

//------------------------------------------------------------//

const GridDescription& VoxetProjectHandle::getGridDescription () const {
   return *m_gridDescription;
}

//------------------------------------------------------------//

const DerivedPropertyList& VoxetProjectHandle::getDerivedProperties () const {
   return m_derivedProperties;
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

DerivedPropertyList::iterator VoxetProjectHandle::getDerivedPropertyBegin () {
   return m_derivedProperties.begin ();
}

//------------------------------------------------------------//

DerivedPropertyList::iterator VoxetProjectHandle::getDerivedPropertyEnd () {
   return m_derivedProperties.end ();
}

//------------------------------------------------------------//

DerivedPropertyFunctionList::iterator VoxetProjectHandle::getDerivedPropertyFunctionBegin () {
   return m_derivedPropertyFunctions.begin ();
}

//------------------------------------------------------------//

DerivedPropertyFunctionList::iterator VoxetProjectHandle::getDerivedPropertyFunctionEnd () {
   return m_derivedPropertyFunctions.end ();
}

//------------------------------------------------------------//

DerivedPropertyFormationFunctionList::iterator VoxetProjectHandle::derivedPropertyFormationFunctionBegin () {
   return m_derivedPropertyFormationFunctions.begin ();
}

//------------------------------------------------------------//

DerivedPropertyFormationFunctionList::iterator VoxetProjectHandle::derivedPropertyFormationFunctionEnd () {
   return m_derivedPropertyFormationFunctions.end ();
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
