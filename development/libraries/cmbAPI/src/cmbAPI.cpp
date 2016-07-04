//                                                                      
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file cmbAPI.C
/// @brief This file keeps API definition for creating Cauldron data model

// CMB API
#include "cmbAPI.h"
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

// TableIO library
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

// generates pseudo random string from alphanumeric characters with given length
static std::string GetRandomString( size_t len )
{
   static unsigned long next = 1970;
   static const char alphanum[] = "0123456789"
                                  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                  "abcdefghijklmnopqrstuvwxyz";
   std::ostringstream oss;
   for ( size_t i = 0; i < len; ++i )
   {
      /* RAND_MAX assumed to be 32767 */
      next = next * 1103515245 + 12345;
      int rnd = ( (unsigned)( next / 65536 ) % 32768 ) / ( 32767 / sizeof( alphanum ) + 1 );

      oss << alphanum[rnd];
   }
   return oss.str();
} 


namespace mbapi {

const char * Model::s_ResultsFolderSuffix = "_CauldronOutputDir"; // defines Cauldron results folder name suffix

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
   std::string compareProject( Model::ModelImpl            * mdl
                             , const std::set<std::string> & procesList
                             , const std::set<std::string> & ignoreList
                             , double                        relTol
                             );

   // Copy matched given filter records from the given project to the current, all similar records in 
   // the currenct projects will be deleted and replaced
   std::string mergeProject( Model::ModelImpl                             * mdl
                           , const std::set<std::string>                  & tblsList
                           , const std::vector<std::vector<std::string> > & fltList
                           , size_t                                       & dlRecNum
                           , size_t                                       & cpRecNum
                           );

   // Set of universal access interfaces. Project file level
   std::vector<std::string> tablesList();
   std::vector<std::string> tableColumnsList(   const std::string & tableName, std::vector<datatype::DataType> & colDataTypes );

   int                      tableSize(     const std::string & tableName );
   void                     clearTable(    const std::string & tableName );
   void                     addRowToTable( const std::string & tableName );
   void                     removeRecordFromTable( const std::string & tableName, int ind );

   long        tableValueAsInteger(  const std::string & tableName, size_t rowNumber, const std::string & propName );
   double      tableValueAsDouble(   const std::string & tableName, size_t rowNumber, const std::string & propName );
   std::string tableValueAsString(   const std::string & tableName, size_t rowNumber, const std::string & propName );
   
   void        setTableIntegerValue( const std::string & tableName, size_t rowNumber, const std::string & propName, long propValue );
   void        setTableDoubleValue(  const std::string & tableName, size_t rowNumber, const std::string & propName, double propValue );
   void        setTableStringValue(  const std::string & tableName, size_t rowNumber, const std::string & propName, const std::string & propValue );
   
   void        tableSort( const std::string & tblName, const std::vector<std::string> & colsName );

   // IO methods
   void loadModelFromProjectFile( const char * projectFileName );
   void saveModelToProjectFile(   const char * projectFileName, bool copyFiles );
   std::string projectFileName() { return m_projFileName; }

   // Create the unique copies of lithology for each given layer, alochtonous lithology and fault cut from the given lists
   // returns array of newly created lithologies name in order of layers->alochtonous lithologies->fault cuts
   std::vector<std::string> copyLithology( const std::string                                       & litName      
                                         , const std::vector< std::pair<std::string, size_t> >     & layersName   
                                         , const std::vector<std::string>                          & alochtLitName
                                         , const std::vector<std::pair<std::string, std::string> > & faultsName
                                         );
  
   LithologyManager    & lithologyManager()    { return m_lithMgr;  } // Lithology
   StratigraphyManager & stratigraphyManager() { return m_stratMgr; } // Stratigraphy
   FluidManager        & fluidManager()        { return m_fluidMgr; } // Fluid
   SourceRockManager   & sourceRockManager()   { return m_srkMgr;   } // Source Rock
   SnapshotManager     & snapshotManager()     { return m_snpMgr;   } // Snapshots manager
   PropertyManager     & propertyManager()     { return m_prpMgr;   } // Properties manager
   MapsManager         & mapsManager()         { return m_mapMgr;   } // Maps manager
  
   // data members
   LithologyManagerImpl     m_lithMgr;
   StratigraphyManagerImpl  m_stratMgr;
   FluidManagerImpl         m_fluidMgr;
   SourceRockManagerImpl    m_srkMgr;
   SnapshotManagerImpl      m_snpMgr;
   PropertyManagerImpl      m_prpMgr;
   MapsManagerImpl          m_mapMgr;

   std::unique_ptr<DataAccess::Interface::ProjectHandle> m_projHandle;   // project file database (set of tables)
   std::unique_ptr<DataAccess::Interface::ObjectFactory> m_factory;  
   std::string                                           m_projFileName; // project files name with path

   // model origin
   void origin( double & x, double & y );

   // model dimensions along X/Y
   void arealSize( double & dimX, double & dimY );
   
   // window size
   void windowSize( double x, double y, int & xMin, int & xMax, int & yMin, int & yMax, double & xc, double & yc );

   void interpolateLithoFractions(
      const std::vector<double>& xin,
      const std::vector<double>& yin,
      const std::vector<double>& lf1,
      const std::vector<double>& lf2,
      const std::vector<double>& lf3,
      std::vector<double>& xInt,
      std::vector<double>& yInt,
      std::vector<double>& rpInt,
      std::vector<double>& r13Int );

   void backTransformLithoFractions(
      const std::vector<double>& rpInt,
      const std::vector<double>& r13Int,
      std::vector<double>& lf1CorrInt,
      std::vector<double>& lf2CorrInt,
      std::vector<double>& lf3CorrInt );
};


///////////////////////////////////////////////////////////////////////////////
// Set of Model wrapper functions to hide the actual implementation from .h
Model::Model() { m_pimpl.reset( new ModelImpl() ); }
Model::Model( const Model & /*otherModel*/ ) { assert(0); }
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
   catch ( ... )                                { reportError( UnknownError, "Unknown error" ); }

   return std::vector<std::string>();
}

std::vector<std::string> Model::tableColumnsList( const std::string & tableName, std::vector<datatype::DataType> & colTypes )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { return m_pimpl->tableColumnsList( tableName, colTypes ); }
   catch ( const Exception & ex ) { reportError( ex.errorCode(), ex.what() ); }
   catch ( ... )                  { reportError( UnknownError, "Unknown error" ); }

   colTypes.clear();
   return std::vector<std::string>();
}


// Get size of the given table
int Model::tableSize( const std::string & tableName )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { return m_pimpl->tableSize( tableName ); }
   catch ( const Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }
   catch ( ... )                  { return reportError( UnknownError, "Unknown error" ); }

   return UndefinedIntegerValue;
}

// Get value from the table
long Model::tableValueAsInteger( const std::string & tableName, size_t rowNumber, const std::string & propName )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { return m_pimpl->tableValueAsInteger( tableName, rowNumber, propName ); }
   catch ( const Exception & ex ) { reportError( ex.errorCode(), ex.what() ); }
   catch ( ... )                  { reportError( UnknownError, "Unknown error" ); }

   return UndefinedIntegerValue;
}

// Get value from the table
double Model::tableValueAsDouble( const std::string & tableName, size_t rowNumber, const std::string & propName )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { return m_pimpl->tableValueAsDouble( tableName, rowNumber, propName ); }
   catch ( const Exception & ex ) { reportError( ex.errorCode(), ex.what() ); }
   catch ( ... )                  { reportError( UnknownError, "Unknown error" ); }

   return UndefinedDoubleValue;
}

// Get value from the table
std::string Model::tableValueAsString( const std::string & tableName, size_t rowNumber, const std::string & propName )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { return m_pimpl->tableValueAsString( tableName, rowNumber, propName ); }
   catch ( const Exception & ex ) { reportError( ex.errorCode(), ex.what() ); }
   catch ( ... )                  { reportError( UnknownError, "Unknown error" ); }

   return UndefinedStringValue;
}

// Set value in the table
ErrorHandler::ReturnCode Model::setTableValue( const std::string & tableName, size_t rowNumber, const std::string & propName, long propValue )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { m_pimpl->setTableIntegerValue( tableName, rowNumber, propName, propValue ); }
   catch ( const Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }
   catch ( ... )                  { return reportError( UnknownError, "Unknown error" ); }

   return NoError;
}

// Set value in the table
ErrorHandler::ReturnCode Model::setTableValue( const std::string & tableName, size_t rowNumber, const std::string & propName, double propValue )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { m_pimpl->setTableDoubleValue( tableName, rowNumber, propName, propValue ); }
   catch ( const Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }
   catch ( ... )                  { return reportError( UnknownError, "Unknown error" ); }

   return NoError;
}

// Set value in the table
ErrorHandler::ReturnCode Model::setTableValue( const std::string & tableName
                                             , size_t              rowNumber
                                             , const std::string & propName
                                             , const std::string & propValue
                                             )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { m_pimpl->setTableStringValue( tableName, rowNumber, propName, propValue ); }
   catch ( const Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }
   catch ( ... )                  { return reportError( UnknownError, "Unknown error" ); }

   return NoError;
}

std::string Model::compareProject( Model                       & mdl1
                                 , const std::set<std::string> & compareTblsList
                                 , const std::set<std::string> & ignoreTblsList
                                 , double                        relTol
                                 )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { return m_pimpl->compareProject( mdl1.m_pimpl.get(), compareTblsList, ignoreTblsList, relTol ); }
   catch ( const Exception & ex ) { return std::string( "Exception during project comparison. Error code: " + 
                                                         ibs::to_string( ex.errorCode() ) + ", error message " + ex.what() ); }
   catch ( ... ) { return "Unknown error"; }
  
   return "Can not perform comparison for unknown reason";
}


std::string Model::mergeProject( Model                                   & mdl1
                          , const std::set<std::string>                  & tblsList
                          , const std::vector<std::vector<std::string> > & fltList
                          , size_t                                       & dlRecNum
                          , size_t                                       & cpRecNum
                       )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { return m_pimpl->mergeProject( mdl1.m_pimpl.get(), tblsList, fltList, dlRecNum, cpRecNum ); }
   catch ( const Exception & ex ) { return std::string( "Exception during project merge. Error code: " + 
                                                        ibs::to_string( ex.errorCode() ) + ", error message " + ex.what() ); }
   catch ( ... ) { return "Unknown error"; }

   return "Can not perform merge for unknown reason";
}


ErrorHandler::ReturnCode Model::tableSort( const std::string & tblName, const std::vector<std::string> & colsName )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { m_pimpl->tableSort( tblName, colsName ); }
   catch ( const Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }
   catch ( ... )                  { return reportError( UnknownError, "Unknown error" ); }

   return NoError;
}

// Delete all rows in given table
ErrorHandler::ReturnCode Model::clearTable( const std::string & tableName )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { m_pimpl->clearTable( tableName ); }
   catch ( const Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }
   catch ( ... )                  { return reportError( UnknownError, "Unknown error" ); }

   return NoError;
}

ErrorHandler::ReturnCode Model::removeRecordFromTable( const std::string & tableName, int ind )
{
   if ( errorCode( ) != NoError ) resetError( ); // clean any previous error

   try { m_pimpl->removeRecordFromTable( tableName, ind ); }
   catch ( const Exception & ex ) { return reportError( ex.errorCode( ), ex.what( ) ); }
   catch ( ... )                  { return reportError( UnknownError, "Unknown error" ); }

   return NoError;
}

// Add new row to the table
ErrorHandler::ReturnCode Model::addRowToTable( const std::string & tableName )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { m_pimpl->addRowToTable( tableName ); }
   catch ( const Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }
   catch ( ... )                  { return reportError( UnknownError, "Unknown error" ); }

   return NoError;
}


Model::ReturnCode Model::loadModelFromProjectFile( const char * projectFileName )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { m_pimpl->loadModelFromProjectFile( projectFileName ); }
   catch ( const Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }
   catch ( ... )                  { return reportError( UnknownError, "Unknown error" ); }

   return NoError;
}

Model::ReturnCode Model::saveModelToProjectFile( const char * projectFileName, bool copyFiles )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { m_pimpl->saveModelToProjectFile( projectFileName, copyFiles ); }
   catch ( const Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }
   catch ( ... )                  { return reportError( UnknownError, "Unknown error" ); }

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
   catch ( const Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }
   catch ( ... )                  { return reportError( UnknownError, "Unknown error" ); }

   return NoError;
}


Model::ReturnCode Model::arealSize( double & dimX, double & dimY )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { m_pimpl->arealSize( dimX, dimY ); }
   catch ( const Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }
   catch ( ... )                  { return reportError( UnknownError, "Unknown error" ); }

   return NoError;
}

Model::ReturnCode Model::windowSize( double x, double y, int & xMin, int & xMax, int & yMin, int & yMax, double & xc, double & yc )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { m_pimpl->windowSize( x, y, xMin, xMax, yMin, yMax, xc, yc); }
   catch ( const Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }
   catch ( ...                  ) { return reportError( UnknownError, "Unknown error" ); }

   return NoError;
}

Model::ReturnCode Model::interpolateLithoFractions( const std::vector<double> & xin
                                                  , const std::vector<double> & yin
                                                  , const std::vector<double> & lf1
                                                  , const std::vector<double> & lf2
                                                  , const std::vector<double> & lf3
                                                  ,       std::vector<double> & xInt
                                                  ,       std::vector<double> & yInt
                                                  ,       std::vector<double> & rpInt
                                                  ,       std::vector<double> & r13Int
                                                  )
{
   if ( errorCode() != NoError ) resetError();

   try { m_pimpl->interpolateLithoFractions( xin, yin, lf1, lf2, lf3, xInt, yInt, rpInt, r13Int ); }
   catch ( const Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }
   catch ( ... ) { return reportError( UnknownError, "Unknown error" ); }

   return NoError;
}

Model::ReturnCode Model::backTransformLithoFractions( const std::vector<double> & rpInt
                                                    , const std::vector<double> & r13Int
                                                    ,       std::vector<double> & lf1CorrInt
                                                    ,       std::vector<double> & lf2CorrInt
                                                    ,       std::vector<double> & lf3CorrInt
                                                    )
{
   if ( errorCode() != NoError ) resetError();

   try { m_pimpl->backTransformLithoFractions( rpInt, r13Int, lf1CorrInt, lf2CorrInt, lf3CorrInt ); }
   catch ( const Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }
   catch ( ... ) { return reportError( UnknownError, "Unknown error" ); }

   return NoError;
}

std::vector<std::string> Model::copyLithology( const std::string                                       & litName      
                                             , const std::vector< std::pair<std::string, size_t> >     & layersName   
                                             , const std::vector<std::string>                          & alochtLitName
                                             , const std::vector<std::pair<std::string, std::string> > & faultsName
                                             )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { return m_pimpl->copyLithology( litName, layersName, alochtLitName, faultsName ); }

   catch ( const Exception & ex ) { reportError( ex.errorCode(), ex.what() ); }
   catch ( ... )                  { reportError( UnknownError, "Unknown error" ); }

   return std::vector<std::string>();
}
 

///////////////////////////////////////////////////////////////////////////////
// Actual implementation of CMB API

Model::ModelImpl::ModelImpl() {}

Model::ModelImpl::~ModelImpl() {}

struct RecordSorter
{
   RecordSorter( database::Table * tbl, double tol ) 
   {
      const database::TableDefinition & tblDef = tbl->getTableDefinition();
      m_eps = tol;

      // For TrapIoTbl and TrapperIoTbl sort table by the entries of the trapRecords list
      if ( tbl->name() == "TrapperIoTbl" or tbl->name() == "TrapIoTbl" )
      {
         std::set<std::string> trapRecords = {"ReservoirName", "Age", "XCoord", "YCoord"};
         std::set<std::string>::iterator trapRecordsIter = trapRecords.begin();

         for ( int k = 1; k > -1; --k )
         {
            for ( size_t i = 0; i < tblDef.size(); ++i )
            {
               std::string fieldDefinition = tblDef.getFieldDefinition( static_cast<int>(i) )->name();
               if (trapRecords.count( fieldDefinition ) == k)
               {
                  m_fldIDs.push_back( i );
                  m_fldTypes.push_back( tblDef.getFieldDefinition( static_cast<int>(i) )->dataType() );
               }         
            }
         }
      }
      // For all other tables sort first by string-type records and then everything else
      else
      {
      // cache fields index and data type 
      for ( size_t i = 0; i < tblDef.size(); ++i )
      {
         if ( tblDef.getFieldDefinition( static_cast<int>( i ) )->dataType() == datatype::String )
         {
            m_fldIDs.push_back( i );
            m_fldTypes.push_back( tblDef.getFieldDefinition( static_cast<int>( i ) )->dataType() );
         }         
      }
      for ( size_t i = 0; i < tblDef.size(); ++i )
      {
         if ( tblDef.getFieldDefinition( static_cast<int>( i ) )->dataType() != datatype::String )
         {
            m_fldIDs.push_back( i );
            m_fldTypes.push_back( tblDef.getFieldDefinition( static_cast<int>( i ) )->dataType() );
         }         
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
               { double v = r1->getValue<float>(id); double w = r2->getValue<float>(id); if ( !NumericFunctions::isEqual( v, w, m_eps ) ) return v < w; }
               break;
            case datatype::Double: 
               { double v = r1->getValue<double>(id); double w = r2->getValue<double>(id); if ( !NumericFunctions::isEqual( v, w, m_eps ) ) return v < w; }
               break;
            case datatype::String: { string v = r1->getValue<string>( id ); string w = r2->getValue<string>( id ); if ( v != w ) return v < w; } break;
            default: ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Unknown data type for TableIO database record: " << m_fldTypes[i];
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
      if ( (ignoreList.size() > 0 && ignoreList.count( (*it)->name() )  > 0 ) ||
           (procesList.size() > 0 && procesList.count( (*it)->name() ) == 0 ) 
         ) { continue; }

      if ( (*it)->size() > 0 ) { lst1.push_back( (*it)->name() ); }
   }
   std::stable_sort( lst1.begin(), lst1.end()  );

   for ( database::Database::iterator it = mdl->m_projHandle->getDataBase()->begin(); it != mdl->m_projHandle->getDataBase()->end(); ++it )
   {
      if ( (ignoreList.size() > 0 && ignoreList.count( (*it)->name() )  > 0 )||
           (procesList.size() > 0 && procesList.count( (*it)->name() ) == 0 )
         ) { continue; }

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
      std::vector<database::Record *>::iterator dit = std::set_symmetric_difference( tbl1Recs.begin()
                                                                                   , tbl1Recs.end()
                                                                                   , tbl2Recs.begin()
                                                                                   , tbl2Recs.end()
                                                                                   , diffRecs.begin()
                                                                                   , recCmp
                                                                                   );
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

         for ( size_t k = 0; k < tblDef.size(); ++k )
         {
            datatype::DataType dt = tblDef.getFieldDefinition( static_cast<int>( k ) )->dataType();
            const std::string & colName = tblDef.getFieldDefinition( static_cast<int>( k ) )->name();
            
            switch ( dt )
            {
               case datatype::Bool:
                  {
                     bool v1 = r1->getValue<bool>( static_cast<int>( k ) ); 
                     bool v2 = r2->getValue<bool>( static_cast<int>( k ) );
                     if ( v1 != v2 ) { oss << tblName << "("<< pos1 << "," << pos2 << ")." << colName << ": " << v1 << " != " << v2 << "\n"; }
                  }
                  break;

               case datatype::Int:
                  {
                     int v1 = r1->getValue<int>( static_cast<int>( k ) );
                     int v2 = r2->getValue<int>( static_cast<int>( k ) );
                     if ( v1 != v2 ) { oss << tblName << "("<< pos1 << "," << pos2 << ")." << colName << ": " << v1 << " != " << v2 << "\n"; }
                  }
                  break;

               case datatype::Long:
                  {
                     long v1 = r1->getValue<long>( static_cast<int>( k ) );
                     long v2 = r2->getValue<long>( static_cast<int>( k ) );
                     if ( v1 != v2 ) { oss << tblName << "("<< pos1 << "," << pos2 << ")." << colName << ": " << v1 << " != " << v2 << "\n"; }
                  }
                  break;

               case datatype::Float:
               case datatype::Double: 
                  {
                     double v1 = r1->getValue<double>( static_cast<int>( k ) );
                     double v2 = r2->getValue<double>( static_cast<int>( k ) );
                     if ( !NumericFunctions::isEqual( v1, v2, relTol ) )
                     {
                        oss << tblName << "("<< pos1 << "," << pos2 << ")." << colName << ": " << v1 << " != " << v2 << "\n";
                     }
                  }
                  break;

               case datatype::String:
                  {
                     string v1 = r1->getValue<string>( static_cast<int>( k ) );
                     string v2 = r2->getValue<string>( static_cast<int>( k ) );
                     if ( v1 != v2 ) { oss << tblName << "("<< pos1 << "," << pos2 << ")." << colName << ": " << v1 << " != " << v2 << "\n"; }
                  }
                  break;

               default: break;
            }
         }
      }
   }
   return oss.str();
}

std::string Model::ModelImpl::mergeProject( Model::ModelImpl                             * mdl
                                          , const std::set<std::string>                  & tblsList
                                          , const std::vector<std::vector<std::string> > & fltList
                                          , size_t                                       & dlRecNum
                                          , size_t                                       & cpRecNum
                                          )
{
   dlRecNum = 0;
   cpRecNum = 0;

   for ( std::set<std::string>::const_iterator it = tblsList.begin(); it != tblsList.end(); ++it )
   {
      database::Table * tblFrom = mdl->m_projHandle->getDataBase()->getTable( *it );
      database::Table * tblTo   =      m_projHandle->getDataBase()->getTable( *it );

      if ( tblFrom == NULL || tblTo == NULL ) continue; // skip empty tables 
     
      //
      bool tableHasFilter = false;
      for ( size_t i = 0; i < fltList.size() && !tableHasFilter; ++i )
      {
         if ( fltList[i][0] == *it ) { tableHasFilter = true; }
      }

      // clean records in current project which matched filter
      if ( !tableHasFilter ) // if no filter given - delete all records
      {
         dlRecNum += tblTo->size();
         clearTable( *it );
         // copy all records
         for ( database::Table::iterator tit = tblFrom->begin(); tit != tblFrom->end(); ++tit )
         {
            tblTo->addRecord( new database::Record( *(*tit) ) );
            ++cpRecNum;
         }
      }
      else
      {
         for ( size_t f = 0; f < fltList.size(); ++f )
         {
            if ( fltList[f][0] != *it ) continue; // skip filters not related to current table

            int index = tblFrom->getIndex( fltList[f][1] );
            if ( index < 0 )
            { 
               throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Wrong column name: " << fltList[f][1]  <<
                  " for the table: " << fltList[f][0];
            }
            // clean the current table records which matched filter
            database::Table::iterator tit = tblTo->begin();
            while ( tit != tblTo->end() )
            {
               if ( fltList[f][2] == (*tit)->getValue<std::string>( index ) )
               {
                  tit = tblTo->removeRecord( tit );
                  ++dlRecNum;
               }
               else { tit++; }
            }
           
            // copy records from table tblFrom to the table tblTo which matched filter
            for ( database::Table::iterator tit = tblFrom->begin(); tit != tblFrom->end(); ++tit )
            {
               database::Record * rec = *tit;
               const std::string & tblVal = rec->getValue<std::string>( index );
               if ( fltList[f][2] == tblVal )
               {
                  tblTo->addRecord( new database::Record( *(*tit) ) );
                  ++cpRecNum;
               }
            }
         }
      }
   }

   if ( dlRecNum > 0 || cpRecNum > 0 )
   {
      database::Database * db = m_projHandle->getDataBase();
      if ( !db->saveToFile( db->getFileName() ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Failed to write updated tables to file: " << db->getFileName();
      }
   }
   return "";
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

void Model::ModelImpl::removeRecordFromTable( const std::string & tableName, int ind )
{
   // get pointer to the table
   database::Table * table = m_projHandle->getDataBase( )->getTable( tableName.c_str( ) );

   if ( !table ) throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << tableName << " table could not be found in project";

   // get the record to remove
   database::Record * recordToRemove = table->getRecord( ind );

   // delete the record
   table->deleteRecord( recordToRemove );
}

long Model::ModelImpl::tableValueAsInteger( const std::string & tableName, size_t rowNumber, const std::string & propName )
{
   // get pointer to the table
   database::Table * table = m_projHandle->getDataBase()->getTable( tableName );

   // if table does not exist - report error
   if (                    !table ) throw ErrorHandler::Exception( UndefinedValue ) << tableName << " table could not be found in project";
   if ( table->size() < rowNumber ) throw ErrorHandler::Exception( UndefinedValue ) << tableName << " size is less then requested row number";

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
   if (                    !table ) throw ErrorHandler::Exception( UndefinedValue ) << tableName << " table could not be found in project";
   if ( table->size() < rowNumber ) throw ErrorHandler::Exception( UndefinedValue ) << tableName << " size is less then requested row number";

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
   if ( table->size() < rowNumber ) throw ErrorHandler::Exception( UndefinedValue ) << tableName << " size is less then requested row number";

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
   if (                    !table ) throw ErrorHandler::Exception( UndefinedValue ) << tableName << " table could not be found in project";
   if ( table->size() < rowNumber ) throw ErrorHandler::Exception( UndefinedValue ) << tableName << " size is less then requested row number";

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
   if (                    !table ) throw ErrorHandler::Exception( UndefinedValue ) << tableName << " table could not be found in project";
   if ( table->size() < rowNumber ) throw ErrorHandler::Exception( UndefinedValue ) << tableName << " size is less then requested row number";

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
Model::ModelImpl & Model::ModelImpl::operator = ( const Model::ModelImpl & /*otherModel*/ )
{
   throw ErrorHandler::Exception( ErrorHandler::NotImplementedAPI ) << "Not implemented yet";

   return *this;
}


void Model::ModelImpl::loadModelFromProjectFile( const char * projectFileName )
{
	m_factory.reset(new DataAccess::Interface::ObjectFactory);
	m_projHandle.reset( DataAccess::Interface::OpenCauldronProject( projectFileName, "rw", m_factory.get() ) );

   if ( !m_projHandle.get() )
   {
      throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Model::loadModelFromProjectFile() failed to load " << projectFileName;
   }

   m_projFileName = projectFileName;
   
   m_srkMgr.setDatabase(   m_projHandle->getDataBase() );                 // set database in source rock manager
   m_lithMgr.setDatabase(  m_projHandle->getDataBase(), &m_stratMgr );    // set database in lithologies type manager and link to strigraphy manager
   m_snpMgr.setDatabase(   m_projHandle->getDataBase(), m_projFileName ); // set database in snapshot manager
   m_prpMgr.setDatabase(   m_projHandle->getDataBase(), m_projFileName ); // set database in property manager
   m_stratMgr.setDatabase( m_projHandle->getDataBase() );                 // set database in stratigraphy manager

   m_mapMgr.setProject(    m_projHandle.get(),          m_projFileName ); // set project handle in maps manager
}

// Save model to the project file
void Model::ModelImpl::saveModelToProjectFile( const char * projectFileName, bool copyFiles )
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
   m_prpMgr.copyResultsFiles( m_projFileName, std::string( projectFileName ), copyFiles );
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

// calculate the window size for multi 1d projects
void Model::ModelImpl::windowSize( double x, double y, int & xMin, int & xMax, int & yMin, int & yMax, double & xc, double & yc)
{
   if ( !m_projHandle.get() ) { throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Model::origin(): no project was loaded"; }

   const DataAccess::Interface::ProjectData * pd = m_projHandle->getProjectData();

   xMin = static_cast<int>( std::floor( ( x - pd->getXOrigin() ) / pd->getDeltaX() ) );
   xMax = xMin + 1;
   
   yMin = static_cast<int>( std::floor( ( y - pd->getYOrigin() ) / pd->getDeltaY() ) );
   yMax = yMin + 1;
   
   // centre of calculated model
   xc = pd->getXOrigin() + ( xMin + xMax ) * 0.5 * pd->getDeltaX();
   yc = pd->getYOrigin() + ( yMin + yMax ) * 0.5 * pd->getDeltaY( );
}

// transform the lithofractions and interpolate the results
void Model::ModelImpl::interpolateLithoFractions( const std::vector<double> & xin
                                                , const std::vector<double> & yin
                                                , const std::vector<double> & lf1
                                                , const std::vector<double> & lf2
                                                , const std::vector<double> & lf3
                                                ,       std::vector<double> & xInt
                                                ,       std::vector<double> & yInt
                                                ,       std::vector<double> & rpInt
                                                ,       std::vector<double> & r13Int
                                                )
{
   const double shift                = 100.0;
   const int    convexHullEdgePoints = 25;
   const size_t nin                  = xin.size();

   // for all wells calculate rp, r12 
   std::vector<double> rp;
   std::vector<double> r13;
   std::vector<double> x;
   std::vector<double> y;

   // all wells are considered to calculate the mean values to use at the adges. this could be refined later on to use only the closest wells
   double sumLf1hat = 0.0;
   double sumLf2hat = 0.0;
   double sumLf3hat = 0.0;

   for ( size_t i = 0; i < nin; ++i )
   {
      sumLf1hat += lf1[i] + shift;
      sumLf2hat += lf2[i] + shift;
      sumLf3hat += lf3[i] + shift;
   }

   double meanLf1hat = sumLf1hat / nin;
   double meanLf2hat = sumLf2hat / nin;
   double meanLf3hat = sumLf3hat / nin;

   double rpmean  = ( meanLf1hat + meanLf3hat ) / meanLf2hat;
   double r13mean =   meanLf1hat / meanLf3hat;

   // the vertices of the domain must be included as interpolation points
   if ( !m_projHandle ) { throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Model::interpolateLithoFractions(): no project was loaded"; }
   const DataAccess::Interface::ProjectData * pd = m_projHandle->getProjectData();

   double xmin   = pd->getXOrigin();
   double ymin   = pd->getYOrigin();

   double deltaX = pd->getDeltaX();
   double deltaY = pd->getDeltaY();

   int    numI   = pd->getNumberOfXNodes();
   int    numJ   = pd->getNumberOfYNodes();

   double xmax   = xmin + numI * deltaX;
   double ymax   = ymin + numJ * deltaY;

   const double deltaXConvexHull = ( xmax - xmin ) / convexHullEdgePoints;
   const double deltaYConvexHull = ( ymax - ymin ) / convexHullEdgePoints;

   // first add the points at the edges in CCW order
   for ( int i = 0; i <= convexHullEdgePoints; ++i )
   {
      x.push_back( xmin + deltaXConvexHull * i );
      y.push_back( ymin );
      rp.push_back( rpmean );
      r13.push_back( r13mean );
   }

   for ( int i = 1; i < convexHullEdgePoints; ++i )
   {
      x.push_back( xmax );
      y.push_back( ymin + i * deltaYConvexHull );
      rp.push_back( rpmean );
      r13.push_back( r13mean );
   }

   for ( int i = convexHullEdgePoints; i >= 0; --i )
   {
      x.push_back( xmin + deltaXConvexHull * i );
      y.push_back( ymax );
      rp.push_back( rpmean );
      r13.push_back( r13mean );
   }

   for ( int i = convexHullEdgePoints - 1; i >= 1; --i )
   {
      x.push_back( xmin );
      y.push_back( ymin + i * deltaYConvexHull );
      rp.push_back( rpmean );
      r13.push_back( r13mean );
   }

   // add well points only after the convex hull has been defined
   for ( size_t i = 0; i < nin; ++i )
   {
      double lf1hat = lf1[i] + shift;
      double lf2hat = lf2[i] + shift;
      double lf3hat = lf3[i] + shift;
      x.push_back( xin[i] );
      y.push_back( yin[i] );
      rp.push_back( ( lf1hat + lf3hat ) / lf2hat );
      r13.push_back(  lf1hat / lf3hat );
   }

   // interpolate 
   if ( ErrorHandler::NoError != m_mapMgr.interpolateMap( x, y, rp
                                                        , xmin + deltaX * 0.5
                                                        , xmax - deltaX * 0.5
                                                        , ymin + deltaY * 0.5
                                                        , ymax - deltaY * 0.5
                                                        , numI, numJ, xInt, yInt, rpInt 
                                                        ) )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "NNlib interpolation failed for rp ";
   }

   if ( ErrorHandler::NoError != m_mapMgr.interpolateMap( x, y, r13
                                                        , xmin + deltaX * 0.5
                                                        , xmax - deltaX * 0.5
                                                        , ymin + deltaY * 0.5
                                                        , ymax - deltaY * 0.5
                                                        , numI, numJ, xInt, yInt, r13Int
                                                        ) )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "NNlib interpolation failed for r13 ";
   }
}

// correct and back transform the interpolated values
void Model::ModelImpl::backTransformLithoFractions( const std::vector<double> & rpInt
                                                  , const std::vector<double> & r13Int
                                                  , std::vector<double>       & lf1CorrInt
                                                  , std::vector<double>       & lf2CorrInt
                                                  , std::vector<double>       & lf3CorrInt
   )
{
   const double eps      = 1e-4;
   const double shift    = 100.0;
   const double sumLfInt = 100.0 + 3.0 * shift;

   lf1CorrInt.resize( rpInt.size() );
   lf2CorrInt.resize( rpInt.size() );
   lf3CorrInt.resize( rpInt.size() );
   
   // back-transform and correct the lithofractions
   for ( int i = 0; i != rpInt.size(); ++i )
   {
      double lf2Int =   sumLfInt            / ( rpInt[i]  + 1 );
      double lf3Int = ( sumLfInt - lf2Int ) / ( r13Int[i] + 1 );
      double lf1Int =   sumLfInt - lf2Int   -   lf3Int;  
      lf1Int -= shift;
      lf2Int -= shift;
      lf3Int -= shift;

      // correct lithofractions if something got wrong
      if ( ( lf1Int + lf2Int ) > 100.0 || lf1Int < 0.0 || lf2Int < 0.0 || lf3Int < 0.0 )
      {
         if ( ( lf1Int + lf2Int ) > 100.0 && ( lf1Int + lf2Int ) < 100.0 + eps )
         {
            double res = lf1Int + lf2Int - 100;
            if ( lf1Int > 2.0 * eps ) { lf1Int -= res * 0.51; }
            if ( lf2Int > 2.0 * eps ) { lf2Int -= res * 0.51; }
            lf3Int = 100.0 - lf1Int - lf2Int;
         }
         else if ( lf1Int < 0 && lf1Int > -eps ) // correct lf1Int
            {
            if ( lf2Int > abs( lf1Int ) ) { lf2Int -= lf1Int; }
            else                          { lf3Int -= lf1Int; }
            lf1Int = 0.0;
            }
         else if ( lf2Int < 0 && lf2Int > -eps ) // correct lf2Int
            {
            if ( lf1Int > abs( lf2Int ) ) { lf1Int -= lf2Int; }
            else                          { lf3Int -= lf2Int; }
            lf2Int = 0.0;
         }
         else if ( lf3Int < 0 && lf3Int > -eps ) // correct lf3Int
            {
            if ( lf1Int > abs( lf3Int ) ) { lf1Int -= lf3Int; }
            else                          { lf2Int -= lf3Int; }
            lf3Int = 0;
            }
            else
            {
            throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Negative interpolated lithofractions: lf1Int " << lf1Int << 
                                                                                                                   " lf2Int " << lf2Int << 
                                                                                                                   " lf3Int " << lf3Int;
         }
            }

      if ( ( lf1Int + lf2Int + lf3Int ) > 100 + eps ) 
            {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << 
                                      "The sum of the interpolated lithofractions is greater than 100.0001: " << lf1Int + lf2Int + lf3Int;
      }

      lf1CorrInt[i] = lf1Int;
      lf2CorrInt[i] = lf2Int;
      lf3CorrInt[i] = lf3Int;
   }
}

// Create the unique copies of lithology for each given layer, alochtonous lithology and fault cut from the given lists
// returns array of newly created lithologies name in order of layers->alochtonous lithologies->fault cuts
std::vector<std::string> Model::ModelImpl::copyLithology( const std::string                                       & litName      
                                                        , const std::vector< std::pair<std::string, size_t> >     & layersName   
                                                        , const std::vector<std::string>                          & allochtLitName
                                                        , const std::vector<std::pair<std::string, std::string> > & faultsName
                                                        )
{
   const size_t randStringSize = 3; // how long should be random part in copied lithology
   std::vector<std::string> copiedLithologiesName;

   // get lithology ID
   mbapi::LithologyManager::LithologyID lithID = m_lithMgr.findID( litName );

   if ( UndefinedIDValue == lithID ) 
   {
      throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "No lithology with name: " <<  litName << " in lithologies type table";
   }

   // go over the given layers list for stratigraphy
   for ( size_t i = 0; i < layersName.size(); ++i )
   {
      // get layer ID from stratigraphy manager
      mbapi::StratigraphyManager::LayerID lyd = m_stratMgr.layerID( layersName[i].first );
      if ( UndefinedIDValue == lyd )
      {
         throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "No such layer: " << layersName[i].first << " in stratigraphy table";
      }

      std::vector<std::string> mixList;

      // get list of lithologies for the layer
      std::vector<double> lithPerc;
      std::vector<std::string> percMaps;
      if ( ErrorHandler::NoError != m_stratMgr.layerLithologiesList( lyd, mixList, lithPerc, percMaps ) )
      {
         throw ErrorHandler::Exception( m_stratMgr.errorCode() ) << m_stratMgr.errorMessage();
      }

      // check if lithology is already copied
      if ( mixList.size() - 1 >= layersName[i].second && 
           mixList[layersName[i].second].rfind( "_CASA" ) != std::string::npos &&
           mixList[layersName[i].second].find( litName ) == 0
         )
      {
         copiedLithologiesName.push_back( mixList[layersName[i].second] );
         continue; // go to the next layer in the list
      }

      // check that mixing id is in the list and layer has the same lithology for the given mixing id
      if ( mixList.size() - 1 < layersName[i].second || mixList[layersName[i].second] != litName )
      {
         throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Layer " << layersName[i].first << " has no lithology " << litName;
      }

      if ( NumericFunctions::isEqual( lithPerc[layersName[i].second], 0.0, 1e-3 ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Layer " << layersName[i].first << " has zero percent of lithology: "
                                                                        << litName;
      }

      // create new lithology name, copy it and assign the new lithology name for the corresponded mixing position to the layer
      std::string newLithoName = litName + "_" + GetRandomString( randStringSize ) + "_CASA";

      mbapi::LithologyManager::LithologyID newLithID = m_lithMgr.copyLithology( lithID, newLithoName );
      if ( UndefinedIDValue == newLithID ) throw ErrorHandler::Exception( m_lithMgr.errorCode() ) << m_lithMgr.errorMessage();
      mixList[layersName[i].second] = newLithoName;
      
      // set updated lithologies list back to the layer
      if ( ErrorHandler::NoError != m_stratMgr.setLayerLithologiesList( lyd, mixList, lithPerc ) )
      {
         throw ErrorHandler::Exception( m_stratMgr.errorCode() ) << m_stratMgr.errorMessage();
      }
      copiedLithologiesName.push_back( newLithoName );
   }

   // go over allochtonous lithologies list
   for ( size_t i = 0; i < allochtLitName.size(); ++i )
   {
      mbapi::LithologyManager::AllochtLithologyID alID = m_lithMgr.findAllochtID( allochtLitName[i] );
      if ( UndefinedIDValue == alID )
      {
         throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "No allochtonous lithology for the layer: " <<  allochtLitName[i];
      }

      const std::string & alLitNm = m_lithMgr.allochtonLithology( alID );
      if ( alLitNm.empty() ) { throw ErrorHandler::Exception( m_lithMgr.errorCode() ) << m_lithMgr.errorMessage(); }

      // check if lithology is already copied
      if ( alLitNm.rfind( "_CASA" ) != std::string::npos &&  alLitNm.find( litName ) == 0 )
      {
         copiedLithologiesName.push_back( alLitNm );
         continue; // go to the next allochtonous lithology in the list
      }

      if ( alLitNm != litName )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Layer " << allochtLitName[i] << " has no allochtonous lithology " 
            << litName;
      }

      // create new lithology name, copy it and assign the new lithology name for the corresponded layer
      std::string newLithoName = litName + "_" + GetRandomString( randStringSize ) + "_CASA";

      mbapi::LithologyManager::LithologyID newLithID = m_lithMgr.copyLithology( lithID, newLithoName );
      if ( UndefinedIDValue == newLithID ) throw ErrorHandler::Exception( m_lithMgr.errorCode() ) << m_lithMgr.errorMessage();
      
      // set copied lithology back to the layer
      if ( ErrorHandler::NoError != m_lithMgr.setAllochtonLithology( alID, newLithoName ) )
      {
         throw ErrorHandler::Exception( m_lithMgr.errorCode() ) << m_lithMgr.errorMessage();
      }

      copiedLithologiesName.push_back( newLithoName );
   }

   // loop over fault table
   for ( size_t i = 0; i < faultsName.size(); ++i )
   {
      mbapi::StratigraphyManager::PrFaultCutID flID = m_stratMgr.findFaultCut( faultsName[i].first, faultsName[i].second );
      if ( UndefinedIDValue == flID )
      {
         throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "No lithology for the fault cut: " <<  faultsName[i].first <<
            ":" << faultsName[i].second;
      }

      const std::string & flLitNm = m_stratMgr.faultCutLithology( flID );
      if ( flLitNm.empty() ) { throw ErrorHandler::Exception( m_stratMgr.errorCode() ) << m_stratMgr.errorMessage(); }

      // check if lithology is already copied
      if ( flLitNm.rfind( "_CASA" ) != std::string::npos &&  flLitNm.find( litName ) == 0 )
      {
         copiedLithologiesName.push_back( flLitNm );
         continue; // go to the next allochtonous lithology in the list
      }

      if ( flLitNm != litName )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Fault cut " << faultsName[i].first << ":" << faultsName[i].second
            << " has different lithology: " << flLitNm << " than excpected: " <<  litName;
      }

      // create new lithology name, copy it and assign the new lithology name for the corresponded fault cut
      std::string newLithoName = litName + "_" + GetRandomString( randStringSize ) + "_CASA";

      mbapi::LithologyManager::LithologyID newLithID = m_lithMgr.copyLithology( lithID, newLithoName );
      if ( UndefinedIDValue == newLithID ) throw ErrorHandler::Exception( m_lithMgr.errorCode() ) << m_lithMgr.errorMessage();
      
      // set copied lithology back to the layer
      if ( ErrorHandler::NoError != m_stratMgr.setFaultCutLithology( flID, newLithoName ) )
      {
         throw ErrorHandler::Exception( m_stratMgr.errorCode() ) << m_stratMgr.errorMessage();
      }

      copiedLithologiesName.push_back( newLithoName );
   }

   if ( copiedLithologiesName.empty() ) { copiedLithologiesName.push_back( litName ); }
   return copiedLithologiesName;
}
 
}

