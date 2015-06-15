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

// C Library
#include <cmath>

// STL
#include <string>

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

   // Set of universal access interfaces. Project file level
   int         tableSize(           const std::string & tableName );
   void        clearTable(          const std::string & tableName );
   void        addRowToTable(       const std::string & tableName );

   double      tableValueAsDouble(  const std::string & tableName, size_t rowNumber, const std::string & propName );
   std::string tableValueAsString(  const std::string & tableName, size_t rowNumber, const std::string & propName );
   
   void        setTableDoubleValue( const std::string & tableName, size_t rowNumber, const std::string & propName, double propValue );
   void        setTableStringValue( const std::string & tableName, size_t rowNumber, const std::string & propName, const std::string & propValue );
   
   // IO methods
   void loadModelFromProjectFile( const char * projectFileName );
   void saveModelToProjectFile(   const char * projectFileName );

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

Model::ModelImpl::ModelImpl() 
{
}

Model::ModelImpl::~ModelImpl( )
{
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

