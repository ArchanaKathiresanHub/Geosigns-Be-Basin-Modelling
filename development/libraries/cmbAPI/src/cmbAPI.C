//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file cmbAPI.C
/// @brief This file keeps API definition for creating Cauldron data model

#include "cmbAPI.h"

// CMB API
#include "LithologyManagerImpl.h"
#include "StratigraphyManagerImpl.h"
#include "FluidManagerImpl.h"
#include "SourceRockManagerImpl.h"
#include "SnapshotManagerImpl.h"
#include "PropertyManagerImpl.h"
#include "MapsManagerImpl.h"

// DataAccess library
#include "Interface/ProjectHandle.h"
#include "Interface/ProjectData.h"
#include "Interface/ObjectFactory.h"

#include "cauldronschemafuncs.h"

// FileSystem library
#include "FilePath.h"

// Utilities lib
#include <NumericFunctions.h>

// C Library
#include <cmath>

// STL
#include <string>
#include <set>
#include <algorithm>
#include <sstream>

namespace mbapi {

///////////////////////////////////////////////////////////////////////////////
// Class which hides all CMB API implementation
class Model::ModelImpl
{
public:

   // constructor / destructor
   ModelImpl();
   ~ModelImpl(); 

   ModelImpl & operator = ( const ModelImpl & otherModel );

   // methods

   // compare tables in project file
   std::string compareProject( Model::ModelImpl * mdl, const std::set<std::string> & procesList, const std::set<std::string> & ignoreList, double relTol );

   // Set of universal access interfaces. Project file level
   std::vector<std::string> tablesList();
   std::vector<std::string> tableColumnsList(   const std::string & tableName, std::vector<datatype::DataType> & colDataTypes );

   int                      tableSize(     const std::string & tableName );
   void                     clearTable(    const std::string & tableName );
   void                     addRowToTable( const std::string & tableName );

   long        tableValueAsInteger(  const std::string & tableName, size_t rowNumber, const std::string & propName );
   double      tableValueAsDouble(   const std::string & tableName, size_t rowNumber, const std::string & propName );
   std::string tableValueAsString(   const std::string & tableName, size_t rowNumber, const std::string & propName );
   
   void        setTableIntegerValue( const std::string & tableName, size_t rowNumber, const std::string & propName, long propValue );
   void        setTableDoubleValue(  const std::string & tableName, size_t rowNumber, const std::string & propName, double propValue );
   void        setTableStringValue(  const std::string & tableName, size_t rowNumber, const std::string & propName, const std::string & propValue );
   
   void        tableSort( const std::string & tblName, const std::vector<std::string> & colsName );

   // IO methods
   void loadModelFromProjectFile( const char * projectFileName );
   void saveModelToProjectFile(   const char * projectFileName );
   std::string projectFileName() { return m_projFileName; }

   // Lithology
   LithologyManager    & lithologyManager() { return m_lithMgr; }
   // Stratigraphy
   StratigraphyManager & stratigraphyManager() { return m_stratMgr; }
   // Fluid
   FluidManager        & fluidManager() { return m_fluidMgr; }
   // Source Rock
   SourceRockManager   & sourceRockManager() { return m_srkMgr; }
   // Snapshots manager
   SnapshotManager     & snapshotManager()  { return m_snpMgr;  }
   // Properties manager
   PropertyManager     & propertyManager()  { return m_prpMgr;  }
   // Maps manager
   MapsManager         & mapsManager()  { return m_mapMgr;  }
  
   // data members
   LithologyManagerImpl              m_lithMgr;
   StratigraphyManagerImpl           m_stratMgr;
   FluidManagerImpl                  m_fluidMgr;
   SourceRockManagerImpl             m_srkMgr;
   SnapshotManagerImpl               m_snpMgr;
   PropertyManagerImpl               m_prpMgr;
   MapsManagerImpl                   m_mapMgr;

   std::auto_ptr<DataAccess::Interface::ProjectHandle>  m_projHandle;  // project file database (set of tables)
   std::string                                          m_projFileName;  // project files name with path

   // model origin
   void origin( double & x, double & y );

   // model dimensions along X/Y
   void arealSize( double & dimX, double & dimY);

};


///////////////////////////////////////////////////////////////////////////////
// Set of Model wrapper functions to hide the actual implementation from .h
Model::Model() { m_pimpl.reset( new ModelImpl() ); }
Model::Model( const Model & otherModel ) {}
Model::~Model() { m_pimpl.reset( 0 ); }

Model & Model::operator = ( const Model & otherModel )
{
   *(m_pimpl.get()) = *(otherModel.m_pimpl.get());
   return *this;
}

///////////////////////////////////////////////////////////////////////////////
// Generic Table IO interface


std::vector<std::string> Model::tablesList()
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { return m_pimpl->tablesList(); }
   catch ( const ErrorHandler::Exception & ex ) { reportError( ex.errorCode(), ex.what() ); }
   catch ( ... ) { reportError( UnknownError, "Unknown error" ); }

   return std::vector<std::string>();
}

std::vector<std::string> Model::tableColumnsList( const std::string & tableName, std::vector<datatype::DataType> & colTypes )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { return m_pimpl->tableColumnsList( tableName, colTypes ); }
   catch ( const ErrorHandler::Exception & ex ) { reportError( ex.errorCode(), ex.what() ); }
   catch ( ... ) { reportError( UnknownError, "Unknown error" ); }

   colTypes.clear();
   return std::vector<std::string>();
}


// Get size of the given table
int Model::tableSize( const std::string & tableName )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { return m_pimpl->tableSize( tableName ); }
   catch ( const ErrorHandler::Exception & ex ) { return this->ErrorHandler::reportError( ex.errorCode(), ex.what( ) ); }
   catch ( ... ) { return this->ErrorHandler::reportError( UnknownError, "Unknown error" ); }

   return UndefinedIntegerValue;
}

// Get value from the table
long Model::tableValueAsInteger( const std::string & tableName, size_t rowNumber, const std::string & propName )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { return m_pimpl->tableValueAsInteger( tableName, rowNumber, propName ); }
   catch ( const ErrorHandler::Exception & ex ) { this->ErrorHandler::reportError( ex.errorCode(), ex.what( ) ); }
   catch ( ... ) { this->ErrorHandler::reportError( UnknownError, "Unknown error" ); }

   return UndefinedIntegerValue;
}

// Get value from the table
double Model::tableValueAsDouble( const std::string & tableName, size_t rowNumber, const std::string & propName )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { return m_pimpl->tableValueAsDouble( tableName, rowNumber, propName ); }
   catch ( const ErrorHandler::Exception & ex ) { this->ErrorHandler::reportError( ex.errorCode(), ex.what( ) ); }
   catch ( ... ) { this->ErrorHandler::reportError( UnknownError, "Unknown error" ); }

   return UndefinedDoubleValue;
}

// Get value from the table
std::string Model::tableValueAsString( const std::string & tableName, size_t rowNumber, const std::string & propName )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { return m_pimpl->tableValueAsString( tableName, rowNumber, propName ); }
   catch ( const ErrorHandler::Exception & ex ) { this->ErrorHandler::reportError( ex.errorCode(), ex.what( ) ); }
   catch ( ... ) { this->ErrorHandler::reportError( UnknownError, "Unknown error" ); }

   return UndefinedStringValue;
}

// Set value in the table
ErrorHandler::ReturnCode Model::setTableValue( const std::string & tableName, size_t rowNumber, const std::string & propName, long propValue )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { m_pimpl->setTableIntegerValue( tableName, rowNumber, propName, propValue ); }
   catch ( const ErrorHandler::Exception & ex ) { return this->ErrorHandler::reportError( ex.errorCode(), ex.what( ) ); }
   catch ( ... ) { return this->ErrorHandler::reportError( UnknownError, "Unknown error" ); }

   return NoError;
}

// Set value in the table
ErrorHandler::ReturnCode Model::setTableValue( const std::string & tableName, size_t rowNumber, const std::string & propName, double propValue )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { m_pimpl->setTableDoubleValue( tableName, rowNumber, propName, propValue ); }
   catch ( const ErrorHandler::Exception & ex ) { return this->ErrorHandler::reportError( ex.errorCode(), ex.what( ) ); }
   catch ( ... ) { return this->ErrorHandler::reportError( UnknownError, "Unknown error" ); }

   return NoError;
}

// Set value in the table
ErrorHandler::ReturnCode Model::setTableValue( const std::string & tableName, size_t rowNumber, const std::string & propName, const std::string & propValue )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { m_pimpl->setTableStringValue( tableName, rowNumber, propName, propValue ); }
   catch ( const ErrorHandler::Exception & ex ) { return this->ErrorHandler::reportError( ex.errorCode(), ex.what() ); }
   catch ( ... ) { return this->ErrorHandler::reportError( UnknownError, "Unknown error" ); }

   return NoError;
}

std::string Model::compareProject( Model & mdl1, const std::set<std::string> & compareTblsList, const std::set<std::string> & ignoreTblsList, double relTol )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { return m_pimpl->compareProject( mdl1.m_pimpl.get(), compareTblsList, ignoreTblsList, relTol ); }
   catch ( const ErrorHandler::Exception & ex ) { return std::string( "Exception during project comparison. Error code: " + 
                                                         ibs::to_string( ex.errorCode() ) + ", error message " + ex.what() ); }
   catch ( ... ) { return "Unknown error"; }
  
   return "Can not perform comparison for unknown reason";
}


ErrorHandler::ReturnCode Model::tableSort( const std::string & tblName, const std::vector<std::string> & colsName )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { m_pimpl->tableSort( tblName, colsName ); }
   catch ( const ErrorHandler::Exception & ex ) { return this->ErrorHandler::reportError( ex.errorCode(), ex.what() ); }
   catch ( ... ) { return this->ErrorHandler::reportError( UnknownError, "Unknown error" ); }

   return NoError;
}

// Delete all rows in given table
ErrorHandler::ReturnCode Model::clearTable( const std::string & tableName )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { m_pimpl->clearTable( tableName ); }
   catch ( const ErrorHandler::Exception & ex ) { return this->ErrorHandler::reportError( ex.errorCode(), ex.what() ); }
   catch ( ... ) { return this->ErrorHandler::reportError( UnknownError, "Unknown error" ); }

   return NoError;
}

// Add new row to the table
ErrorHandler::ReturnCode Model::addRowToTable( const std::string & tableName )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { m_pimpl->addRowToTable( tableName ); }
   catch ( const ErrorHandler::Exception & ex ) { return this->ErrorHandler::reportError( ex.errorCode(), ex.what() ); }
   catch ( ... ) { return this->ErrorHandler::reportError( UnknownError, "Unknown error" ); }

   return NoError;
}


Model::ReturnCode Model::loadModelFromProjectFile( const char * projectFileName )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { m_pimpl->loadModelFromProjectFile( projectFileName ); }
   catch ( const ErrorHandler::Exception & ex ) { return this->ErrorHandler::reportError( ex.errorCode(), ex.what() ); }
   catch ( ... ) { return this->ErrorHandler::reportError( UnknownError, "Unknown error" ); }

   return NoError;
}

Model::ReturnCode Model::saveModelToProjectFile( const char * projectFileName )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { m_pimpl->saveModelToProjectFile( projectFileName ); }
   catch ( const ErrorHandler::Exception & ex ) { return this->ErrorHandler::reportError( ex.errorCode(), ex.what() ); }
   catch ( ... ) { return this->ErrorHandler::reportError( UnknownError, "Unknown error" ); }

   return NoError;
}


std::string           Model::projectFileName(    ) { return m_pimpl->projectFileName(    ); }
LithologyManager    & Model::lithologyManager(   ) { return m_pimpl->lithologyManager(   ); }
StratigraphyManager & Model::stratigraphyManager() { return m_pimpl->stratigraphyManager(); }
FluidManager        & Model::fluidManager(       ) { return m_pimpl->fluidManager(       ); }
SourceRockManager   & Model::sourceRockManager(  ) { return m_pimpl->sourceRockManager(  ); }
SnapshotManager     & Model::snapshotManager(    ) { return m_pimpl->snapshotManager(    ); }
PropertyManager     & Model::propertyManager(    ) { return m_pimpl->propertyManager(    ); }
MapsManager         & Model::mapsManager(        ) { return m_pimpl->mapsManager(        ); }

Model::ReturnCode Model::origin( double & x, double & y )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { m_pimpl->origin( x, y ); }
   catch ( const ErrorHandler::Exception & ex ) { return this->ErrorHandler::reportError( ex.errorCode(), ex.what() ); }
   catch ( ... ) { return this->ErrorHandler::reportError( UnknownError, "Unknown error" ); }

   return NoError;
}

Model::ReturnCode Model::arealSize( double & dimX, double & dimY )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { m_pimpl->arealSize( dimX, dimY ); }
   catch ( const ErrorHandler::Exception & ex ) { return this->ErrorHandler::reportError( ex.errorCode(), ex.what() ); }
   catch ( ... ) { return this->ErrorHandler::reportError( UnknownError, "Unknown error" ); }

   return NoError;
}


///////////////////////////////////////////////////////////////////////////////
// Actual implementation of CMB API

Model::ModelImpl::ModelImpl() {}

Model::ModelImpl::~ModelImpl( ) {}

struct RecordSorter
{
   RecordSorter( database::Table * tbl, double tol ) 
   {
      const database::TableDefinition & tblDef = tbl->getTableDefinition();
      m_eps = tol;

      // cache fields index and data type 
      for ( int i = 0; i < tblDef.size(); ++i )
      {
         if ( tblDef.getFieldDefinition( i )->dataType() == datatype::String )
         {
            m_fldIDs.push_back( i );
            m_fldTypes.push_back( tblDef.getFieldDefinition( i )->dataType() );
         }         
      }
      for ( int i = 0; i < tblDef.size(); ++i )
      {
         if ( tblDef.getFieldDefinition( i )->dataType() != datatype::String )
         {
            m_fldIDs.push_back( i );
            m_fldTypes.push_back( tblDef.getFieldDefinition( i )->dataType() );
         }         
      }
   }

   //  this function is used as less operator for the strict weak ordering
   bool operator() ( const database::Record * r1, const database::Record * r2 ) const
   {
      assert( r1 != NULL && r2 != NULL );

      for ( size_t i = 0; i < m_fldIDs.size(); ++ i )
      {  int id = m_fldIDs[i];
         switch ( m_fldTypes[i] )
         {
            case datatype::Bool:   { bool v = r1->getValue<bool>( id ); bool w = r2->getValue<bool>( id ); if ( v != w ) return v < w; } break;
            case datatype::Int:    { int  v = r1->getValue<int >( id ); int  w = r2->getValue<int >( id ); if ( v != w ) return v < w; } break;
            case datatype::Long:   { long v = r1->getValue<long>( id ); int  w = r2->getValue<long>( id ); if ( v != w ) return v < w; } break;
            case datatype::Float:
               { double v = r1->getValue<float>( id ); double w = r2->getValue<float>( id ); if ( !NumericFunctions::isEqual( v, w, m_eps ) ) return v < w; }
               break;
          case datatype::Double: 
               { double v = r1->getValue<double>( id ); double w = r2->getValue<double>( id ); if ( !NumericFunctions::isEqual( v, w, m_eps ) ) return v < w; }
               break;
            case datatype::String: { string v = r1->getValue<string>( id ); string w = r2->getValue<string>( id ); if ( v != w ) return v < w; } break;
         }
      }
      return false;
   }

   std::vector<int>                 m_fldIDs;
   std::vector<datatype::DataType>  m_fldTypes;
   double                           m_eps;
};


// compare tables in project file
std::string Model::ModelImpl::compareProject( Model::ModelImpl * mdl
                                            , const std::set<std::string> & procesList
                                            , const std::set<std::string> & ignoreList
                                            , double relTol
                                            )
{
   if ( !m_projHandle.get()      || !m_projHandle->getDataBase() )
   {
      throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Project " << m_projFileName      << " not loaded";
   }

   if ( !mdl->m_projHandle.get() || !m_projHandle->getDataBase() )
   {
      throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Project " << mdl->m_projFileName << " not loaded";
   }

   std::ostringstream oss;

   // compare tables list
   std::vector<std::string> lst1;
   std::vector<std::string> lst2;

   for ( database::Database::iterator it = m_projHandle->getDataBase()->begin(); it != m_projHandle->getDataBase()->end(); ++it )
   {
      if ( ignoreList.size() > 0 && ignoreList.count( (*it)->name() ) > 0 ||
           procesList.size() > 0 && procesList.count( (*it)->name() ) == 0 )
      { continue; }

      if ( (*it)->size() > 0 ) { lst1.push_back( (*it)->name() ); }
   }
   std::stable_sort( lst1.begin(), lst1.end()  );

   for ( database::Database::iterator it = mdl->m_projHandle->getDataBase()->begin(); it != mdl->m_projHandle->getDataBase()->end(); ++it )
   {
      if ( ignoreList.size() > 0 && ignoreList.count( (*it)->name() ) > 0 ||
           procesList.size() > 0 && procesList.count( (*it)->name() ) == 0 )
      { continue; }

      if ( (*it)->size() > 0 ) { lst2.push_back( (*it)->name() ); }
   }
   std::stable_sort( lst2.begin(), lst2.end() );

   // do comparison
   std::vector<std::string> tblLst( lst1.size() + lst2.size() );
   std::vector<std::string>::iterator tit = std::set_symmetric_difference( lst1.begin(), lst1.end(), lst2.begin(), lst2.end(), tblLst.begin() );
   tblLst.resize( tit - tblLst.begin() );

   for ( size_t i = 0; i < tblLst.size(); ++i )
   {  
      const std::string & pname = m_projHandle->getDataBase()->getTable( tblLst[i] )->size() > 0 ? m_projFileName : mdl->m_projFileName;
      oss << "Only in " << pname << " table " << tblLst[i] << "\n";
   }

   // get tables list which exist in both projects
   tblLst.resize( lst1.size() + lst2.size() );
   std::vector<std::string>::iterator intit = std::set_intersection( lst1.begin(), lst1.end(), lst2.begin(), lst2.end(), tblLst.begin() );
   tblLst.resize( intit - tblLst.begin() );

   // go over all selected tables
   for ( size_t i = 0; i < tblLst.size(); ++i )
   {
      const std::string & tblName = tblLst[i];
      database::Table * tbl1 = m_projHandle->getDataBase()->getTable( tblName );
      database::Table * tbl2 = mdl->m_projHandle->getDataBase()->getTable( tblName );

      if ( tbl1->size() != tbl2->size() )
      {
         oss << "Table " << tblName << " has " << tbl1->size() << " records in project " << m_projFileName <<
         " but " << tbl2->size() << " records in project " << mdl->m_projFileName << "\n";
         continue; // can't compare !!!
      }

      if ( tbl1->size() > std::set<database::Record*>().max_size() )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Can't compare too big tables: " << tblName << "(" << tbl1->size() << ")";
      }

      RecordSorter recCmp( tbl1, relTol );

      std::vector<database::Record*> tbl1Recs( tbl1->begin(), tbl1->end() );
      std::vector<database::Record*> tbl2Recs( tbl2->begin(), tbl2->end() );

      std::stable_sort( tbl1Recs.begin(), tbl1Recs.end(), recCmp );
      std::stable_sort( tbl2Recs.begin(), tbl2Recs.end(), recCmp );

      std::vector<database::Record *> diffRecs( tbl1Recs.size() + tbl2Recs.size() );
      std::vector<database::Record *>::iterator dit = std::set_symmetric_difference( tbl1Recs.begin(), tbl1Recs.end(), tbl2Recs.begin(), tbl2Recs.end(), diffRecs.begin(), recCmp );
      diffRecs.resize( dit - diffRecs.begin() );
      
      tbl1Recs.clear();
      tbl2Recs.clear();
      // sort different records back to sets
      for ( size_t j = 0; j < diffRecs.size(); ++j )
      {
         if ( diffRecs[j]->getTable() == tbl1 ) { tbl1Recs.push_back( diffRecs[j] ); }
         else                                   { tbl2Recs.push_back( diffRecs[j] ); }
      }    

      assert( tbl1Recs.size() == tbl2Recs.size() );

      const database::TableDefinition & tblDef = tbl1->getTableDefinition();

      // compare field in records
      for ( std::vector<database::Record*>::iterator it1  = tbl1Recs.begin(), it2  = tbl2Recs.begin();
                                                     it1 != tbl1Recs.end() && it2 != tbl2Recs.end(); 
                                                     ++it1, ++it2
          )
      {
         database::Record * r1 = *it1;
         database::Record * r2 = *it2;

         size_t pos1 = tbl1->findRecordPosition( r1 ) - tbl1->begin();
         size_t pos2 = tbl2->findRecordPosition( r2 ) - tbl2->begin();

         for ( int k = 0; k < tblDef.size(); ++k )
         {
            datatype::DataType dt = tblDef.getFieldDefinition( k )->dataType();
            const std::string & colName = tblDef.getFieldDefinition( k )->name();
            
            switch ( dt )
            {
               case datatype::Bool:
                  {
                     bool v1 = r1->getValue<bool>( k ); 
                     bool v2 = r2->getValue<bool>( k );
                     if ( v1 != v2 ) { oss << tblName << "("<< pos1 << "," << pos2 << ")." << colName << ": " << v1 << " != " << v2 << "\n"; }
                  }
                  break;

               case datatype::Int:
                  {
                     int v1 = r1->getValue<int>( k );
                     int v2 = r2->getValue<int>( k );
                     if ( v1 != v2 ) { oss << tblName << "("<< pos1 << "," << pos2 << ")." << colName << ": " << v1 << " != " << v2 << "\n"; }
                  }
                  break;

               case datatype::Long:
                  {
                     long v1 = r1->getValue<long>( k );
                     long v2 = r2->getValue<long>( k );
                     if ( v1 != v2 ) { oss << tblName << "("<< pos1 << "," << pos2 << ")." << colName << ": " << v1 << " != " << v2 << "\n"; }
                  }
                  break;

               case datatype::Float:
               case datatype::Double: 
                  {
                     double v1 = r1->getValue<double>( k );
                     double v2 = r2->getValue<double>( k );
                     if ( !NumericFunctions::isEqual( v1, v2, relTol ) )
                     {
                        oss << tblName << "("<< pos1 << "," << pos2 << ")." << colName << ": " << v1 << " != " << v2 << "\n";
                     }
                  }
                  break;

               case datatype::String:
                  {
                     string v1 = r1->getValue<string>( k );
                     string v2 = r2->getValue<string>( k );
                     if ( v1 != v2 ) { oss << tblName << "("<< pos1 << "," << pos2 << ")." << colName << ": " << v1 << " != " << v2 << "\n"; }
                  }
                  break;
            }
         }
      }
   }
   return oss.str();
}


std::vector<std::string> Model::ModelImpl::tablesList()
{
   std::vector<std::string> ret;

   if ( !m_projHandle.get() || !m_projHandle->getDataBase() ) throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Project " << m_projFileName << " not loaded";
   for ( database::Database::iterator it = m_projHandle->getDataBase()->begin(); it != m_projHandle->getDataBase()->end(); ++it )
   {
      ret.push_back( (*it)->name() );
   }
   return ret;
}

std::vector<std::string> Model::ModelImpl::tableColumnsList( const std::string & tableName, std::vector<datatype::DataType> & colTypes )
{
   std::vector<std::string> ret;
   colTypes.clear();

   // get pointer to the table
   database::Table * table = m_projHandle->getDataBase()->getTable( tableName.c_str() );

   // if table does not exist - report error
   if ( !table ) throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << tableName << " table could not be found in project";

   const database::TableDefinition & tblDef = table->getTableDefinition();

   for ( size_t i = 0; i < tblDef.size(); ++i )
   {
      const database::FieldDefinition * fldDef = tblDef.getFieldDefinition( i );
      if ( !fldDef ) continue;
      ret.push_back( fldDef->name() );
      colTypes.push_back( fldDef->dataType() );
   }

   return ret;
}

int Model::ModelImpl::tableSize( const std::string & tableName )
{
   // get pointer to the table
   database::Table * table = m_projHandle->getDataBase()->getTable( tableName.c_str() );

   // if table does not exist - report error
   if ( !table ) throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << tableName << " table could not be found in project";

   // return table size
   return static_cast<int>( table->size() );
}

void Model::ModelImpl::clearTable( const std::string & tableName )
{
   // get pointer to the table
   database::Table * table = m_projHandle->getDataBase()->getTable( tableName.c_str() );

   // if table does not exist - report error
   if ( !table ) throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << tableName << " table could not be found in project";

   table->clear();
}

// add empty record to the end of the table
void Model::ModelImpl::addRowToTable( const std::string & tableName )
{
   // get pointer to the table
   database::Table * table = m_projHandle->getDataBase()->getTable( tableName.c_str() );

   // if table does not exist - report error
   if ( !table ) throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << tableName << " table could not be found in project";

   // add empty record to the end of the table
   table->createRecord();
}

long Model::ModelImpl::tableValueAsInteger( const std::string & tableName, size_t rowNumber, const std::string & propName )
{
   // get pointer to the table
   database::Table * table = m_projHandle->getDataBase()->getTable( tableName );

   // if table does not exist - report error
   if (                     !table ) throw ErrorHandler::Exception( UndefinedValue ) << tableName << " table could not be found in project";
   if ( table->size( ) < rowNumber ) throw ErrorHandler::Exception( UndefinedValue ) << tableName << " size is less then requested row number";

   database::Record * record = table->getRecord( static_cast<int>( rowNumber ) );
   if ( !record ) throw ErrorHandler::Exception( UndefinedValue ) << tableName << " does not have any records";

   const database::TableDefinition & tblDef  = record->getTableDefinition();
   int ind = tblDef.getIndex( propName );

   if ( ind < 0 ) throw ErrorHandler::Exception( UndefinedValue ) << propName << " - unknown column name in the table " + tableName;

   datatype::DataType dt = tblDef.getFieldDefinition( ind )->dataType();
   switch ( dt )
   {
   case datatype::Bool: return record->getValue<bool>( ind ) ? 1 : 0;
   case datatype::Int:  return record->getValue<int>( ind );
   case datatype::Long: return record->getValue<long>( ind );
   default:
      throw ErrorHandler::Exception( UndefinedValue ) << tableName << "(" << propName << ") - data type can't be cast to float point value";
   }
   return UndefinedIntegerValue;
}

double Model::ModelImpl::tableValueAsDouble( const std::string & tableName, size_t rowNumber, const std::string & propName )
{
   // get pointer to the table
   database::Table * table = m_projHandle->getDataBase()->getTable( tableName );

   // if table does not exist - report error
   if (                     !table ) throw ErrorHandler::Exception( UndefinedValue ) << tableName << " table could not be found in project";
   if ( table->size( ) < rowNumber ) throw ErrorHandler::Exception( UndefinedValue ) << tableName << " size is less then requested row number";

   database::Record * record = table->getRecord( static_cast<int>( rowNumber ) );
   if ( !record ) throw ErrorHandler::Exception( UndefinedValue ) << tableName << " does not have any records";

   const database::TableDefinition & tblDef  = record->getTableDefinition();
   int ind = tblDef.getIndex( propName );
   
   if ( ind < 0 ) throw ErrorHandler::Exception( UndefinedValue ) << propName << " - unknown column name in the table " + tableName;
   
   datatype::DataType dt = tblDef.getFieldDefinition( ind )->dataType();
   switch ( dt )
   {
   case datatype::Double: return record->getValue<double>( ind );
   case datatype::Float:  return record->getValue<float>( ind );
   default:
      throw ErrorHandler::Exception( UndefinedValue ) << tableName << "(" << propName << ") - data type can't be cast to float point value";
   }
   return UndefinedDoubleValue;
}

std::string Model::ModelImpl::tableValueAsString( const std::string & tableName, size_t rowNumber, const std::string & propName )
{
   // get pointer to the table
   database::Table * table = m_projHandle->getDataBase()->getTable( tableName );

   // if table does not exist - report error
   if (                    !table ) throw ErrorHandler::Exception( UndefinedValue ) << tableName << " table could not be found in project";
   if ( table->size() < rowNumber ) throw ErrorHandler::Exception( UndefinedValue ) << tableName << " size is less then requested row number";

   database::Record * record = table->getRecord( static_cast<int>( rowNumber ) );
   if ( !record ) throw ErrorHandler::Exception( UndefinedValue ) << tableName << " does not have any records";

   const database::TableDefinition & tblDef = record->getTableDefinition();
   int ind = tblDef.getIndex( propName );

   if ( ind < 0 ) throw ErrorHandler::Exception( UndefinedValue ) << propName << " - unknown column name in the table " + tableName;

   datatype::DataType dt = tblDef.getFieldDefinition( ind )->dataType();
   switch ( dt )
   {
   case datatype::String: return record->getValue<std::string>( ind );
   default:
      throw ErrorHandler::Exception( UndefinedValue ) << tableName << "(" << propName << ") - data type can't be cast to string";
   }
   return UndefinedStringValue;
}

void Model::ModelImpl::setTableIntegerValue( const std::string & tableName, size_t rowNumber, const std::string & propName, long propValue )
{
   // get pointer to the table
   database::Table * table = m_projHandle->getDataBase()->getTable( tableName );

   // if table does not exist - report error
   if (                     !table ) throw ErrorHandler::Exception( UndefinedValue ) << tableName << " table could not be found in project";
   if ( table->size( ) < rowNumber ) throw ErrorHandler::Exception( UndefinedValue ) << tableName << " size is less then requested row number";

   database::Record * record = table->getRecord( static_cast<int>( rowNumber ) );
   if ( !record ) throw ErrorHandler::Exception( UndefinedValue ) << tableName << " does not have any records";

   const database::TableDefinition & tblDef = record->getTableDefinition();
   int ind = tblDef.getIndex( propName );

   if ( ind < 0 ) throw ErrorHandler::Exception( UndefinedValue ) << propName << " - unknown column name in the table " << tableName;

   datatype::DataType dt = tblDef.getFieldDefinition( ind )->dataType();
   switch ( dt )
   {
   case datatype::Bool: record->setValue<bool>( ind, (propValue ? true : false)  ); break;
   case datatype::Int:  record->setValue<int>(  ind, static_cast<int>(propValue) ); break;
   case datatype::Long: record->setValue<long>( ind,                  propValue  ); break;
   default:
      throw ErrorHandler::Exception( UndefinedValue ) << tableName << "(" << propName << ") - data type can't be cast to integer value";
   }
}


void Model::ModelImpl::setTableDoubleValue( const std::string & tableName, size_t rowNumber, const std::string & propName, double propValue )
{
   // get pointer to the table
   database::Table * table = m_projHandle->getDataBase()->getTable( tableName );

   // if table does not exist - report error
   if (                     !table ) throw ErrorHandler::Exception( UndefinedValue ) << tableName << " table could not be found in project";
   if ( table->size( ) < rowNumber ) throw ErrorHandler::Exception( UndefinedValue ) << tableName << " size is less then requested row number";

   database::Record * record = table->getRecord( static_cast<int>( rowNumber ) );
   if ( !record ) throw ErrorHandler::Exception( UndefinedValue ) << tableName << " does not have any records";

   const database::TableDefinition & tblDef = record->getTableDefinition();
   int ind = tblDef.getIndex( propName );

   if ( ind < 0 ) throw ErrorHandler::Exception( UndefinedValue ) << propName << " - unknown column name in the table " << tableName;

   datatype::DataType dt = tblDef.getFieldDefinition( ind )->dataType();
   switch ( dt )
   {
   case datatype::Double: record->setValue<double>( ind, propValue );                        break;
   case datatype::Float:  record->setValue<float>( ind, static_cast<float>( propValue ) );   break;
   default:
      throw ErrorHandler::Exception( UndefinedValue ) << tableName << "(" << propName << ") - data type can't be cast to float point value";
   }
}

void Model::ModelImpl::setTableStringValue( const std::string & tableName, size_t rowNumber, const std::string & propName, const std::string & propValue )
{
   // get pointer to the table
   database::Table * table = m_projHandle->getDataBase()->getTable( tableName );

   // if table does not exist - report error
   if (                     !table ) throw ErrorHandler::Exception( UndefinedValue ) << tableName << " table could not be found in project";
   if ( table->size( ) < rowNumber ) throw ErrorHandler::Exception( UndefinedValue ) << tableName << " size is less then requested row number";

   database::Record * record = table->getRecord( static_cast<int>( rowNumber ) );
   if ( !record ) throw ErrorHandler::Exception( UndefinedValue ) << tableName << " does not have any records";

   const database::TableDefinition & tblDef = record->getTableDefinition();
   int ind = tblDef.getIndex( propName );

   if ( ind < 0 ) throw ErrorHandler::Exception( UndefinedValue ) << propName << " - unknown column name in the table " << tableName;

   datatype::DataType dt = tblDef.getFieldDefinition( ind )->dataType();
   switch ( dt )
   {
   case datatype::String: record->setValue<std::string>( ind, propValue ); break;
   default:
      throw ErrorHandler::Exception( UndefinedValue ) << tableName << "(" << propName << ") - data type can't be cast to string";
   }
}

// sort table according to given columns list
void Model::ModelImpl::tableSort( const std::string & tblName, const std::vector<std::string> & colsName )
{
   // get pointer to the table
   database::Table * table = m_projHandle->getDataBase()->getTable( tblName );

   // if table does not exist - report error
   if ( !table ) throw ErrorHandler::Exception( UndefinedValue ) << tblName << " table could not be found in project";

   table->stable_sort( colsName );
}


// Load model from the project file
Model::ModelImpl & Model::ModelImpl::operator = ( const Model::ModelImpl & otherModel )
{
   throw ErrorHandler::Exception( ErrorHandler::NotImplementedAPI ) << "Not implemented yet";

   return *this;
}


void Model::ModelImpl::loadModelFromProjectFile( const char * projectFileName )
{
   m_projHandle.reset( DataAccess::Interface::OpenCauldronProject( projectFileName, "rw" ) );

   if ( !m_projHandle.get() )
   {
      throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Model::loadModelFromProjectFile() failed to load " << projectFileName;
   }

   m_projFileName = projectFileName;
   
   m_srkMgr.setDatabase(   m_projHandle->getDataBase() );                 // set database in source rock manager
   m_lithMgr.setDatabase(  m_projHandle->getDataBase() );                 // set database in lithologies type manager
   m_snpMgr.setDatabase(   m_projHandle->getDataBase(), m_projFileName ); // set database in snapshot manager
   m_prpMgr.setDatabase(   m_projHandle->getDataBase(), m_projFileName ); // set database in property manager
   m_stratMgr.setDatabase( m_projHandle->getDataBase() );                 // set database in stratigraphy manager

   m_mapMgr.setProject(    m_projHandle.get(),          m_projFileName ); // set project handle in maps manager
}

// Save model to the project file
void Model::ModelImpl::saveModelToProjectFile( const char * projectFileName )
{
   if ( !m_projHandle.get() )
   {
      throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Model::saveModelToProjectFile(): no project to save";
   }

   if ( !m_projHandle->getDataBase()->saveToFile( projectFileName ) )
   {
      throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Model::saveModelToProjectFile() failed to save to " << 
                                                                   projectFileName << " project file";
   }
    
   // copying maps
   ibs::FilePath projectFilePath( projectFileName );
   projectFilePath.cutLast();  // cut filename

   m_mapMgr.copyMapFiles( projectFilePath.path() );
   m_prpMgr.copyResultsFiles( m_projFileName, std::string( projectFileName ) );
}

// model origin
void Model::ModelImpl::origin( double & x, double & y )
{
   if ( !m_projHandle.get() )
   {
      throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Model::origin(): no project was loaded";
   }

   const DataAccess::Interface::ProjectData * pd = m_projHandle->getProjectData();

   x = pd->getXOrigin() + pd->getDeltaX() * pd->getWindowXMin();
   y = pd->getYOrigin() + pd->getDeltaY() * pd->getWindowYMin();
}

// model dimensions along X/Y
void Model::ModelImpl::arealSize( double & dimX, double & dimY )
{
   if ( !m_projHandle.get() ) { throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Model::origin(): no project was loaded"; }

   const DataAccess::Interface::ProjectData * pd = m_projHandle->getProjectData();

   dimX = ( pd->getWindowXMax() - pd->getWindowXMin() ) * pd->getDeltaX();
   dimY = ( pd->getWindowYMax() - pd->getWindowYMin() ) * pd->getDeltaY();
}

}

