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

// DataAccess library
#include "Interface/ProjectHandle.h"
//#include "cauldronschema.h"
//#include "cauldronschemafuncs.h"

#include <stdexcept>
#include <string>
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

   // Set of universal access interfaces. Project file level
   int         tableSize( const std::string & tableName );
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
   
   // data members
   LithologyManagerImpl              m_lithMgr;
   StratigraphyManagerImpl           m_stratMgr;
   FluidManagerImpl                  m_fluidMgr;
   SourceRockManagerImpl             m_srkMgr;

   std::auto_ptr<database::Database> m_projDatabase;
   std::string                       m_projFileName;
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
   catch ( std::exception & ex ) { return this->ErrorHandler::reportError( UndefinedValue, ex.what( ) ); }
   catch ( ... ) { return this->ErrorHandler::reportError( UnknownError, "Unknown error" ); }

   return UndefinedIntegerValue;
}

// Get value from the table
double Model::tableValueAsDouble( const std::string & tableName, size_t rowNumber, const std::string & propName )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { return m_pimpl->tableValueAsDouble( tableName, rowNumber, propName ); }
   catch ( std::exception & ex ) { this->ErrorHandler::reportError( UndefinedValue, ex.what() ); }
   catch ( ...                 ) { this->ErrorHandler::reportError( UnknownError, "Unknown error" ); }

   return UndefinedDoubleValue;
}

// Get value from the table
std::string Model::tableValueAsString( const std::string & tableName, size_t rowNumber, const std::string & propName )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { return m_pimpl->tableValueAsString( tableName, rowNumber, propName ); }
   catch ( std::exception & ex ) { this->ErrorHandler::reportError( UndefinedValue, ex.what() ); }
   catch ( ...                 ) { this->ErrorHandler::reportError( UnknownError, "Unknown error" ); }

   return UndefinedStringValue;
}

// Set value in the table
ErrorHandler::ReturnCode Model::setTableValue( const std::string & tableName, size_t rowNumber, const std::string & propName, double propValue )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { m_pimpl->setTableDoubleValue( tableName, rowNumber, propName, propValue ); }
   catch ( std::exception & ex ) { return this->ErrorHandler::reportError( UndefinedValue, ex.what() ); }
   catch ( ... ) { return this->ErrorHandler::reportError( UnknownError, "Unknown error" ); }

   return NoError;
}

// Set value in the table
ErrorHandler::ReturnCode Model::setTableValue( const std::string & tableName, size_t rowNumber, const std::string & propName, const std::string & propValue )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { m_pimpl->setTableStringValue( tableName, rowNumber, propName, propValue ); }
   catch ( std::exception & ex ) { return this->ErrorHandler::reportError( UndefinedValue, ex.what() ); }
   catch ( ... ) { return this->ErrorHandler::reportError( UnknownError, "Unknown error" ); }

   return NoError;
}


Model::ReturnCode Model::loadModelFromProjectFile( const char * projectFileName )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { m_pimpl->loadModelFromProjectFile( projectFileName ); }
   catch( std::exception & ex ) { return this->ErrorHandler::reportError( IoError, ex.what() ); }
   catch( ...                 ) { return this->ErrorHandler::reportError( UnknownError, "Unknown error" ); }

   return NoError;
}

Model::ReturnCode Model::saveModelToProjectFile( const char * projectFileName )
{
   if ( errorCode() != NoError ) resetError(); // clean any previous error

   try { m_pimpl->saveModelToProjectFile( projectFileName ); }
   catch( std::exception & ex ) { return this->ErrorHandler::reportError( IoError, ex.what() ); }
   catch( ...                 ) { return this->ErrorHandler::reportError( UnknownError, "Unknown error" ); }

   return NoError;
}

LithologyManager    & Model::lithologyManager(   ) { return m_pimpl->lithologyManager(   ); }
StratigraphyManager & Model::stratigraphyManager() { return m_pimpl->stratigraphyManager(); }
FluidManager        & Model::fluidManager(       ) { return m_pimpl->fluidManager(       ); }
SourceRockManager   & Model::sourceRockManager(  ) { return m_pimpl->sourceRockManager(  ); }


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
   database::Table * table = m_projDatabase->getTable( tableName.c_str() );

   // if table does not exist - report error
   if ( !table ) throw std::runtime_error( tableName + " table could not be found in project" );

   // return table size
   return static_cast<int>( table->size() );
}

double Model::ModelImpl::tableValueAsDouble( const std::string & tableName, size_t rowNumber, const std::string & propName )
{
   // get pointer to the table
   database::Table * table = m_projDatabase->getTable( tableName );

   // if table does not exist - report error
   if ( !table ) throw std::runtime_error( tableName + " table could not be found in project" );
   if ( table->size() < rowNumber ) throw std::runtime_error( tableName + " size is less then requested row number" );

   database::Record * record = table->getRecord( static_cast<int>( rowNumber ) );
   if ( !record ) throw std::runtime_error( tableName + " does not have any records" );

   const database::TableDefinition & tblDef  = record->getTableDefinition();
   int ind = tblDef.getIndex( propName );
   
   if ( ind < 0 ) throw std::runtime_error( propName + " - unknown column name in the table " + tableName );
   
   datatype::DataType dt = tblDef.getFieldDefinition( ind )->dataType();
   switch ( dt )
   {
   case datatype::Double: return record->getValue<double>( ind );
   case datatype::Float:  return record->getValue<float>( ind );
   default:
      throw std::runtime_error( tableName + "(" + propName + ") - data type can't be cast to float point value" );
   }
   return UndefinedDoubleValue;
}

std::string Model::ModelImpl::tableValueAsString( const std::string & tableName, size_t rowNumber, const std::string & propName )
{
   // get pointer to the table
   database::Table * table = m_projDatabase->getTable( tableName );

   // if table does not exist - report error
   if ( !table ) throw std::runtime_error( tableName + " table could not be found in project" );
   if ( table->size() < rowNumber ) throw std::runtime_error( tableName + " size is less then requested row number" );

   database::Record * record = table->getRecord( static_cast<int>( rowNumber ) );
   if ( !record ) throw std::runtime_error( tableName + " does not have any records" );

   const database::TableDefinition & tblDef = record->getTableDefinition();
   int ind = tblDef.getIndex( propName );

   if ( ind < 0 ) throw std::runtime_error( propName + " - unknown column name in the table " + tableName );

   datatype::DataType dt = tblDef.getFieldDefinition( ind )->dataType();
   switch ( dt )
   {
   case datatype::String: return record->getValue<std::string>( ind );
   default:
      throw std::runtime_error( tableName + "(" + propName + ") - data type can't be cast to string" );
   }
   return UndefinedStringValue;
}


void Model::ModelImpl::setTableDoubleValue( const std::string & tableName, size_t rowNumber, const std::string & propName, double propValue )
{
   // get pointer to the table
   database::Table * table = m_projDatabase->getTable( tableName );

   // if table does not exist - report error
   if ( !table ) throw std::runtime_error( tableName + " table could not be found in project" );
   if ( table->size() < rowNumber ) throw std::runtime_error( tableName + " size is less then requested row number" );

   database::Record * record = table->getRecord( static_cast<int>( rowNumber ) );
   if ( !record ) throw std::runtime_error( tableName + " does not have any records" );

   const database::TableDefinition & tblDef = record->getTableDefinition();
   int ind = tblDef.getIndex( propName );

   if ( ind < 0 ) throw std::runtime_error( propName + " - unknown column name in the table " + tableName );

   datatype::DataType dt = tblDef.getFieldDefinition( ind )->dataType();
   switch ( dt )
   {
   case datatype::Double: record->setValue<double>( ind, propValue );                        break;
   case datatype::Float:  record->setValue<float>( ind, static_cast<float>( propValue ) );   break;
   default:
      throw std::runtime_error( tableName + "(" + propName + ") - data type can't be cast to float point value" );
   }
}

void Model::ModelImpl::setTableStringValue( const std::string & tableName, size_t rowNumber, const std::string & propName, const std::string & propValue )
{
   // get pointer to the table
   database::Table * table = m_projDatabase->getTable( tableName );

   // if table does not exist - report error
   if ( !table ) throw std::runtime_error( tableName + " table could not be found in project" );
   if ( table->size() < rowNumber ) throw std::runtime_error( tableName + " size is less then requested row number" );

   database::Record * record = table->getRecord( static_cast<int>( rowNumber ) );
   if ( !record ) throw std::runtime_error( tableName + " does not have any records" );

   const database::TableDefinition & tblDef = record->getTableDefinition();
   int ind = tblDef.getIndex( propName );

   if ( ind < 0 ) throw std::runtime_error( propName + " - unknown column name in the table " + tableName );

   datatype::DataType dt = tblDef.getFieldDefinition( ind )->dataType();
   switch ( dt )
   {
   case datatype::String: record->setValue<std::string>( ind, propName );
   default:
      throw std::runtime_error( tableName + "(" + propName + ") - data type can't be cast to string" );
   }
}

// Load model from the project file
Model::ModelImpl & Model::ModelImpl::operator = ( const Model::ModelImpl & otherModel )
{
   throw std::runtime_error( "Not implemented yet" );
   return *this;
}


void Model::ModelImpl::loadModelFromProjectFile( const char * projectFileName )
{
   m_projDatabase.reset( DataAccess::Interface::CreateDatabaseFromCauldronProject( projectFileName ) );

   if ( !m_projDatabase.get() )
   {
      std::ostringstream oss;
      oss << "Model::loadModelFromProjectFile() failed to load " << projectFileName;
      throw std::runtime_error( oss.str() );
   }

   m_projFileName = projectFileName;
   
   m_srkMgr.setDatabase( m_projDatabase.get() ); // set database in source rock manager
}

// Save model to the project file
void Model::ModelImpl::saveModelToProjectFile( const char * projectFileName )
{
   if ( m_projDatabase.get() )
   {
      if ( !m_projDatabase->saveToFile( projectFileName ) )
      {
         std::ostringstream oss;
         oss << "Model::saveModelToProjectFile() failed to save to " << projectFileName << " project file";
         throw std::runtime_error( oss.str() );
      }
   }
   else
   {
      throw std::runtime_error( "Model::saveModelToProjectFile(): no project to save" );
   }
}

}

