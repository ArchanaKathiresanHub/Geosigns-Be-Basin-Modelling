//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file LithologyManagerImpl.C
/// @brief This file keeps API implementation for manipulating lithologies in Cauldron model

// CMB API
#include "LithologyManagerImpl.h"
#include "StratigraphyManagerImpl.h"
#include "UndefinedValues.h"

// Utilities lib
#include "NumericFunctions.h"

// DataAccess
#include "database.h"

// STL
#include <stdexcept>
#include <string>
#include <set>
#include <cmath>
#include <sstream>

namespace mbapi
{

const char * LithologyManagerImpl::s_lithoTypesTableName        = "LithotypeIoTbl";

const char * LithologyManagerImpl::s_lithoTypeNameFieldName     = "Lithotype";

// Porosity model
const char * LithologyManagerImpl::s_porosityModelFieldName     = "Porosity_Model";
const char * LithologyManagerImpl::s_surfPorosityFieldName      = "SurfacePorosity";
const char * LithologyManagerImpl::s_ccExponentialFieldName     = "CompacCoefES";
const char * LithologyManagerImpl::s_ccaDblExponentialFieldName = "CompacCoefESA";
const char * LithologyManagerImpl::s_ccbDblExponentialFieldName = "CompacCoefESB";
const char * LithologyManagerImpl::s_ccSoilMechanicsFieldName   = "Compaction_Coefficient_SM";
const char * LithologyManagerImpl::s_minPorosityFieldName       = "MinimumPorosity";
const char * LithologyManagerImpl::s_stpThermalCondFieldName    = "StpThCond";
const char * LithologyManagerImpl::s_seisVelocityFieldName      = "SeisVelocity";

// Permeability model
const char * LithologyManagerImpl::s_permeabilityModelFieldName      = "PermMixModel";
// common for all model parameters
const char * LithologyManagerImpl::s_permeabilityAnisotropyFieldName = "PermAnisotropy";

// common for Mudstone and Sandstone models
const char * LithologyManagerImpl::s_DepositionalPermFieldName       = "DepoPerm";

// Specific for Multi-point
const char * LithologyManagerImpl::s_mpNumberOfDataPointsFieldName   = "Number_Of_Data_Points";
const char * LithologyManagerImpl::s_mpPorosityFieldName             = "Multipoint_Porosity";
const char * LithologyManagerImpl::s_mpPermpeabilityFieldName        = "Multipoint_Permeability";

// Specific for Mudstone
const char * LithologyManagerImpl::s_mudPermeabilityRecoveryCoeff    = "PermDecrStressCoef";
const char * LithologyManagerImpl::s_mudPermeabilitySensitivityCoeff = "PermIncrRelaxCoef";

// Specific for Sandstone
const char * LithologyManagerImpl::s_permSandClayPercentage          = "PermIncrRelaxCoef";


// thermo conductivity/heat capacity tables
const char * LithologyManagerImpl::s_lithoThCondTableName  = "LitThCondIoTbl";
const char * LithologyManagerImpl::s_lithoHeatCapTableName = "LitHeatCapIoTbl";

const char * LithologyManagerImpl::s_LithotypeFieldName    = "Lithotype";
const char * LithologyManagerImpl::s_TempIndexFieldName    = "TempIndex";
const char * LithologyManagerImpl::s_ThCondFieldName       = "ThCond";
const char * LithologyManagerImpl::s_HeatCapacityFieldName = "HeatCapacity";

// Allochtonous lithology
const char * LithologyManagerImpl::s_allochtLithTableName       = "AllochthonLithoIoTbl";
const char * LithologyManagerImpl::s_allochtLayerNameFieldName  = "LayerName";
const char * LithologyManagerImpl::s_allochtLithotypeFieldName  = "Lithotype";



static void ParseCoefficientsFromString( const std::string & str, std::vector<double> & result )
{
   std::istringstream stream( str );

   while ( stream )
   {
      double value = 0.0;
      stream >> value;
      if ( !stream ) break;
      result.push_back( value );
   }
}

static std::string PrintCoefficientsToString( const std::vector<double> & inp )
{
   std::ostringstream oss;

   for ( size_t i = 0; i < inp.size(); ++i )
   {
      oss << inp[i];
      if ( i < inp.size()-1 ) oss << " ";
   }
   return oss.str();
}

// Constructor
LithologyManagerImpl::LithologyManagerImpl()
{
   m_db = NULL;
   m_stMgr = NULL;
}

// Copy operator
LithologyManagerImpl & LithologyManagerImpl::operator = ( const LithologyManagerImpl & /*otherLithMgr*/ )
{
   throw std::runtime_error( "Not implemented yet" );
   return *this;
}

// Set project database. Reset all
void LithologyManagerImpl::setDatabase( database::ProjectFileHandlerPtr pfh, mbapi::StratigraphyManagerImpl * stratMgr )
{
   m_db = pfh;

   m_lithIoTbl   = m_db->getTable( s_lithoTypesTableName );
   m_alLithIoTbl = m_db->getTable( s_allochtLithTableName );

   m_stMgr = stratMgr;
}

// Get list of lithologies in the model
std::vector<LithologyManager::LithologyID> LithologyManagerImpl::lithologiesIDs() const
{
   std::vector<LithologyID> ltIDs; // if m_lithIoTbl does not exist - return empty array
   if ( m_lithIoTbl )
   {
      // fill IDs array with increasing indexes
      ltIDs.resize( m_lithIoTbl->size(), 0 );
      for ( size_t i = 0; i < ltIDs.size(); ++i ) ltIDs[ i ] = static_cast<LithologyID>( i );
   }

   return ltIDs;
}

// Create new lithology
LithologyManager::LithologyID LithologyManagerImpl::createNewLithology()
{
   throw std::runtime_error( "Not implemented yet" );
}


// duplicate records in thermal conductivity and heat capacity tables for the given lithology
void LithologyManagerImpl::copyRecordsHeatCoeffTbls( const char * tblName, const std::string & origLithoName, const std::string & newLithoName )
{
   database::Table * ttable = m_db->getTable( tblName );

   // if table does not exist - report error
   if ( !ttable ) { throw Exception( NonexistingID ) <<  tblName << " table could not be found in project"; }

   // go over all records and collect records for the source lithology
   std::vector<const database::Record *> recSet;
   for ( size_t k = 0; k < ttable->size(); ++k )
   {
      database::Record * rec = ttable->getRecord( static_cast<int>( k ) );
      if ( !rec ) continue;
      if ( rec->getValue<std::string>( s_LithotypeFieldName ) == origLithoName )
      {
         recSet.push_back( rec );
      }
   }

   // go over found records and duplicate them for the new lithology
   for ( size_t k = 0; k < recSet.size(); ++k )
   {
      database::Record * nrec = new database::Record( *(recSet[k]) );
      nrec->setValue( s_LithotypeFieldName, newLithoName );
      ttable->addRecord( nrec );
   }
}

// Make a copy of the given lithology. Also makes a new set of records in table [LitThCondIoTbl] for the new litholog
LithologyManager::LithologyID LithologyManagerImpl::copyLithology( LithologyID id, const std::string & newLithoName )
{
   LithologyID ret = Utilities::Numerical::NoDataIDValue;

   if ( errorCode() != NoError ) resetError();
   try
   {
      // first check if given name already exist
      if ( !IsValueUndefined( findID( newLithoName ) ) )
      {
         throw Exception( AlreadyDefined ) << "Create copy: " << newLithoName << ", already exist in the lithology table";
      }

      // proceed with copy

      // if table does not exist - report error
      if ( !m_lithIoTbl ) { throw Exception( NonexistingID ) <<  s_lithoTypesTableName << " table could not be found in project"; }

      // get record for copy
      database::Record * origRec = m_lithIoTbl->getRecord( static_cast<int>( id ) );
      if ( !origRec ) { throw Exception( NonexistingID ) << "No lithology type with such ID: " << id; }

      // create a copy of lithology
      database::Record * copyRec = new database::Record( *origRec );

      // get the orig lithology name
      const std::string & origLithoName = origRec->getValue<std::string>( s_lithoTypeNameFieldName );

      // change the name
      copyRec->setValue( s_lithoTypeNameFieldName, newLithoName );
      // add copy record with new name to the table end
      m_lithIoTbl->addRecord( copyRec );

      // duplicate records in Thermal conductivity and heat capacity tables
      copyRecordsHeatCoeffTbls( s_lithoThCondTableName,  origLithoName, newLithoName ); // duplicate records for the lithoName in ThermoCond table
      copyRecordsHeatCoeffTbls( s_lithoHeatCapTableName, origLithoName, newLithoName ); // duplicate records for the lithoName in HeatCap table

      // if all is OK - create the new LithologyID for lithology copy
      ret = m_lithIoTbl->size() - 1;
   }
   catch( const Exception & ex ) { reportError( ex.errorCode(), ex.what() ); }

   return ret;
}


// clean records from thermal conductivity and heat capacity tables for the given lithology
void LithologyManagerImpl::cleanHeatCoeffTbls( const char * tblName, const std::string & lithoName )
{
   database::Table * ttable = m_db->getTable( tblName );

   // if table does not exist - report error
   if ( !ttable ) { throw Exception( NonexistingID ) << tblName << " table could not be found in project"; }

   // go over all records and collect records for the source lithology
   std::vector<const database::Record *> recSet;
   for ( size_t k = 0; k < ttable->size(); ++k )
   {
      database::Record * rec = ttable->getRecord( static_cast<int>( k ) );
      if ( !rec ) continue;
      if ( rec->getValue<std::string>( s_LithotypeFieldName ) == lithoName )
      {
         ttable->deleteRecord( rec ); // because we deleting current record we need to shift k back
         k--;
      }
   }
}

ErrorHandler::ReturnCode LithologyManagerImpl::deleteLithology( LithologyID id )
{
   if ( errorCode() != NoError ) resetError();
   try
   {
      // if table does not exist - report error
      if ( !m_lithIoTbl ) { throw Exception( NonexistingID ) <<  s_lithoTypesTableName << " table could not be found in project"; }

      // get record for copy
      database::Record * lrec = m_lithIoTbl->getRecord( static_cast<int>( id ) );
      if ( !lrec ) { throw Exception( NonexistingID ) << "No lithology type with such ID: " << id; }

      const std::string & lithoName = lrec->getValue<std::string>( s_lithoTypeNameFieldName );

      // go over known tables and check if they have a reference to this lithology
      if ( m_stMgr )
      {
         //////////// check stratigraphy layers
         const std::vector<StratigraphyManager::LayerID> & layIDs = m_stMgr->layersIDs();
         for ( size_t i = 0; i < layIDs.size(); ++i )
         {
            std::vector<std::string> lithoNamesLst;
            std::vector<double>      lithoPerct;
            std::vector<std::string> percMaps;
            m_stMgr->layerLithologiesList( layIDs[i], lithoNamesLst, lithoPerct, percMaps );
            std::vector<std::string>::iterator it = std::find( lithoNamesLst.begin(), lithoNamesLst.end(), lithoName );
            if ( it != lithoNamesLst.end() ) // this lithology is referenced in stratigraphy, return error
            {
               throw Exception( ValidationError ) << "Can not remove lithology: " << lithoName <<
                  ", because it is referenced in stratigraphy by the layer: " << m_stMgr->layerName( layIDs[i] );
            }
         }

         ////////////// check fault cuts
         const std::vector<StratigraphyManager::PrFaultCutID> & fcIDs = m_stMgr->faultCutsIDs();
         for ( size_t i = 0; i < fcIDs.size(); ++i )
         {
            if ( m_stMgr->faultCutLithology( fcIDs[i] ) == lithoName )
            {
               throw Exception( ValidationError ) << "Can not remove lithology: " << lithoName <<
                  ", because it is referenced in fault cuts by the fault cut: " << m_stMgr->faultCutName( fcIDs[i] ) <<
                  " for the map: " << m_stMgr->faultCutMapName( fcIDs[i] );
            }
         }
      }

      ////////////// then go over alochotnous lithologies
      const std::vector<AllochtLithologyID> & alLithoIDs = allochtonLithologiesIDs();
      for ( size_t i = 0; i < alLithoIDs.size(); ++i )
      {
         if ( allochtonLithology( alLithoIDs[i] ) == lithoName )
         {
            throw Exception( ValidationError ) << "Can not remove lithology: " << lithoName <<
               ", because it is referenced in allochton lithology by the layer: " << allochtonLithologyLayerName( alLithoIDs[i] );
         }
      }

      // delete records in Thermal conductivity and heat capacity tables
      cleanHeatCoeffTbls( s_lithoThCondTableName,  lithoName ); // clean records for the lithoName in ThermoCond table
      cleanHeatCoeffTbls( s_lithoHeatCapTableName, lithoName ); // clean records for the lithoName in HeatCap table

      // and finaly remove the record in lithology table itself
      m_lithIoTbl->deleteRecord( lrec );
   }
   catch( const Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }

   return NoError;
}

// Record comparison method which compare values of each field in both record and skip fileds from the ignoreList
// this is needed for finding lithologies with the same properties
struct RecordSorter
{
   RecordSorter( database::Table * tbl, double tol, const std::vector<std::string> & ignoreList  )
   {
      const database::TableDefinition & tblDef = tbl->getTableDefinition();
      m_eps = tol;

      // cache fields index and data type
      for ( size_t i = 0; i < tblDef.size(); ++i )
      {
         if ( tblDef.getFieldDefinition( i )->dataType() == datatype::String )
         {
            if ( ! ignoreList.empty() &&
                 std::find( ignoreList.begin(), ignoreList.end(), tblDef.getFieldDefinition( i )->name() ) != ignoreList.end() )
            { continue; }

            m_fldIDs.push_back( i );
            m_fldTypes.push_back( tblDef.getFieldDefinition( i )->dataType() );
         }
      }
      for ( size_t i = 0; i < tblDef.size(); ++i )
      {
         if ( tblDef.getFieldDefinition( i )->dataType() != datatype::String )
         {
            if ( ! ignoreList.empty() &&
                 std::find( ignoreList.begin(), ignoreList.end(), tblDef.getFieldDefinition( i )->name() ) != ignoreList.end() )
            { continue; }

            m_fldIDs.push_back( i );
            m_fldTypes.push_back( tblDef.getFieldDefinition( i )->dataType() );
         }
      }
   }

   //  this function is used as less operator for the strict weak ordering to compare cached fields
   bool operator() ( const database::Record * r1, const database::Record * r2 ) const
   {
      assert( r1 != NULL && r2 != NULL );

      for ( size_t i = 0; i < m_fldIDs.size(); ++ i )
      {  size_t id = m_fldIDs[i];
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
            default: ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Unknown data type for LithoType database record: " << m_fldTypes[i];
         }
      }
      return false;
   }

   std::vector<size_t>              m_fldIDs;
   std::vector<datatype::DataType>  m_fldTypes;
   double                           m_eps;
};



//////////////////////////////////////////////////////////////////////////////////////
// Scan lithology table for duplicated lithologies and delete them updating references
ErrorHandler::ReturnCode LithologyManagerImpl::cleanDuplicatedLithologies()
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      // if table does not exist - report error
      if ( !m_lithIoTbl ) { throw Exception( NonexistingID ) <<  s_lithoTypesTableName << " table could not be found in project"; }

      // create records comparer and add to ignore list the lithology name field
      RecordSorter recCmp( m_lithIoTbl, 1e-5, std::vector<std::string>( 1, s_lithoTypeNameFieldName ) );

      // keep unique records in the set
      std::set< const database::Record *, RecordSorter > uniqLst( recCmp );

      // go over all records and collect records for the source lithology  and check for duplicate records
      // first element in pair keeps original record, the second one - copy
      std::vector< std::pair< const database::Record *, const database::Record * > > forDelLst;
      for ( size_t i = 0; i < m_lithIoTbl->size(); ++i )
      {
         const database::Record * rec = m_lithIoTbl->getRecord( static_cast<int>( i ) );
         if ( !rec ) continue;

         std::pair< std::set<const database::Record*>::iterator, bool> insRes = uniqLst.insert( rec );
         if ( !insRes.second )
         {
            forDelLst.push_back( std::pair<const database::Record *, const database::Record * >( *(insRes.first), rec ) );
         }
      }

      //go over the list of duplicated records and delete them from the table correcting references from FaultCut and AlochtLith tables
      for ( size_t i = 0; i < forDelLst.size(); ++i )
      {
         const database::Record * rec = forDelLst[i].second; // this record will be deleted

         const std::string & oldLithName = rec->getValue<std::string>(                 s_lithoTypeNameFieldName );
         const std::string & newLithName = forDelLst[i].first->getValue<std::string>(  s_lithoTypeNameFieldName );

         // go over known tables and replace reference to lithology which will be deleted
         if ( m_stMgr )
         {  //////////// process stratigraphy layers
            const std::vector<StratigraphyManager::LayerID> & layIDs = m_stMgr->layersIDs();
            for ( size_t j = 0; j < layIDs.size(); ++j )
            {
               std::vector<std::string> lithoNamesLst;
               std::vector<double>      lithoPerct;
               std::vector<std::string> percMaps;
               m_stMgr->layerLithologiesList( layIDs[j], lithoNamesLst, lithoPerct, percMaps );
               bool isReplaced = false;
               for ( size_t k = 0; k < lithoNamesLst.size(); ++k )
               {
                  if ( lithoNamesLst[k] == oldLithName )
                  {
                     lithoNamesLst[k] = newLithName;
                     isReplaced = true;
                  }
               }
               if ( isReplaced ) { m_stMgr->setLayerLithologiesList( layIDs[j], lithoNamesLst, lithoPerct ); }
            }

            ////////////// process fault cuts
            const std::vector<StratigraphyManager::PrFaultCutID> & fcIDs = m_stMgr->faultCutsIDs();
            for ( size_t j = 0; j < fcIDs.size(); ++j )
            {
               if ( m_stMgr->faultCutLithology( fcIDs[j] ) == oldLithName )
               {
                  if ( NoError != m_stMgr->setFaultCutLithology( fcIDs[j], newLithName ) ) return moveError( *m_stMgr );
               }
            }
         }

         ////////////// then process alochotnous lithologies
         const std::vector<AllochtLithologyID> & alLithoIDs = allochtonLithologiesIDs();
         for ( size_t j = 0; j < alLithoIDs.size(); ++j )
         {
            if ( allochtonLithology( alLithoIDs[j] ) == oldLithName )
            {
               if ( NoError != setAllochtonLithology( alLithoIDs[j], newLithName ) ) { return errorCode(); }
            }
         }

         // at final - delete unneeded lithology record
         if ( NoError != deleteLithology( findID( oldLithName ) ) ) { return errorCode(); }
      }
   }
   catch ( const Exception & e ) { return reportError( e.errorCode(), e.what() ); }

   return NoError;
}


///////////////////////////////////////////////////////////////////////////////
// Get lithology name
std::string LithologyManagerImpl::lithologyName( LithologyID id )
{
   if ( errorCode() != NoError ) resetError();

   std::string lName;

   // if table does not exist - report error
   if ( !m_lithIoTbl )
   {
      reportError( NonexistingID, std::string( s_lithoTypesTableName ) + " table could not be found in project" );
      return lName;
   }

   database::Record * rec = m_lithIoTbl->getRecord( static_cast<int>( id ) );
   if ( !rec )
   {
      reportError( NonexistingID, "No lithology type with such ID" );
      return lName;
   }
   lName = rec->getValue<std::string>( s_lithoTypeNameFieldName );

   return lName;
}

// find lithology ID by the lithology name
LithologyManager::LithologyID LithologyManagerImpl::findID( const std::string & lName )
{
   if ( errorCode() != NoError ) resetError();
   try
   {
      // if table does not exist - report error
      if ( !m_lithIoTbl ) { throw Exception( NonexistingID ) << s_lithoTypesTableName << " table could not be found in project"; }

      for ( size_t i = 0; i < m_lithIoTbl->size(); ++i )
      {
         database::Record * rec = m_lithIoTbl->getRecord( static_cast<unsigned int>( i ) );
         if ( !rec ) { throw Exception( NonexistingID ) << "No lithology type with such ID: " << i; }

         if ( lName == rec->getValue<std::string>( s_lithoTypeNameFieldName ) )
         {
            return static_cast<LithologyManager::LithologyID>( i );
         }
      }
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return Utilities::Numerical::NoDataIDValue;
}

///////////////////////////////////////////////////////////////////////////////
// Allochton lithology methods

// Get list of allochton lithologies in the model
// return array with IDs of allochton lygthologies defined in the model
std::vector<LithologyManager::AllochtLithologyID> LithologyManagerImpl::allochtonLithologiesIDs() const
{
   std::vector<LithologyManager::AllochtLithologyID>  ltIDs;

   // if m_alLithIoTbl does not exist - return empty array
   if ( m_alLithIoTbl )
   {
      // fill IDs array with increasing indexes
      ltIDs.resize( m_alLithIoTbl->size(), 0 );
      for ( size_t i = 0; i < ltIDs.size(); ++i ) ltIDs[ i ] = static_cast<AllochtLithologyID>( i );
   }
   return ltIDs;
}


// Search in AllochthonLithoIoTbl table for the given layer name
// AllochthonLithologyID for the found lithology on success, Utilities::Numerical::NoDataIDValue otherwise
LithologyManager::AllochtLithologyID LithologyManagerImpl::findAllochtID( const std::string & layerName )
{
   if ( errorCode() != NoError ) resetError();
   try
   {
      // if table does not exist - report error
      if ( !m_alLithIoTbl ) { throw Exception( NonexistingID ) << s_allochtLithTableName << " table could not be found in project"; }

      for ( size_t i = 0; i < m_alLithIoTbl->size(); ++i )
      {
         database::Record * rec = m_alLithIoTbl->getRecord( static_cast<unsigned int>( i ) );
         if ( !rec ) { throw Exception( NonexistingID ) << "No allochton lithology type with such ID: " << i; }

         if ( layerName == rec->getValue<std::string>( s_allochtLayerNameFieldName ) )
         {
            return static_cast<LithologyManager::AllochtLithologyID>( i );
         }
      }
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return Utilities::Numerical::NoDataIDValue;
}

// Get lithlogy name for the allochton lithology
// return Name of the allochton lithology
std::string LithologyManagerImpl::allochtonLithology( AllochtLithologyID alID )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      // if table does not exist - report error
      if ( !m_alLithIoTbl ) { throw Exception( NonexistingID ) << s_allochtLithTableName << " table could not be found in project"; }

      database::Record * rec = m_alLithIoTbl->getRecord( static_cast<int>( alID ) );
      if ( !rec ) { throw Exception( NonexistingID ) << "No allochtonous lithology type with such ID: " << alID; }

      return rec->getValue<std::string>( s_allochtLithotypeFieldName );
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return "";
}

// Get layer name for the allochton lithology
// return Name of the layer for allochton lithology
std::string LithologyManagerImpl::allochtonLithologyLayerName( AllochtLithologyID alID )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      // if table does not exist - report error
      if ( !m_alLithIoTbl ) { throw Exception( NonexistingID ) << s_allochtLithTableName << " table could not be found in project"; }

      database::Record * rec = m_alLithIoTbl->getRecord( static_cast<int>( alID ) );
      if ( !rec ) { throw Exception( NonexistingID ) << "No allochtonous lithology type with such ID: " << alID; }

      return rec->getValue<std::string>( s_allochtLayerNameFieldName );
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return "";
}

// Set new allochton lithology for the layer
// return ErrorHandler::NoError on success, error code otherwise
ErrorHandler::ReturnCode LithologyManagerImpl::setAllochtonLithology( AllochtLithologyID alID, const std::string & newLithoName )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      // if table does not exist - report error
      if ( !m_alLithIoTbl ) { throw Exception( NonexistingID ) << s_allochtLithTableName << " table could not be found in project"; }

      database::Record * rec = m_alLithIoTbl->getRecord( static_cast<int>( alID ) );
      if ( !rec ) { throw Exception( NonexistingID ) << "No allochtonous lithology type with such ID: " << alID; }

      rec->setValue( s_allochtLithotypeFieldName, newLithoName );
   }
   catch ( const Exception & e ) { return reportError( e.errorCode(), e.what() ); }

   return NoError;
}


// Get lithology porosity model
ErrorHandler::ReturnCode LithologyManagerImpl::porosityModel( LithologyID         id              // [in] lithology ID
                                                            , PorosityModel       & porModel      // [out] porosity model type
                                                            , std::vector<double> & porModelPrms  // [out] porosity model parameters
                                                            )
{
   if ( errorCode() != NoError ) resetError();

   // if table does not exist - report error
   if ( !m_lithIoTbl )
   {
      return reportError( NonexistingID, std::string( s_lithoTypesTableName ) + " table could not be found in project" );
   }

   database::Record * rec = m_lithIoTbl->getRecord( static_cast<int>( id ) );
   if ( !rec )
   {
      return reportError( NonexistingID, "No lithology type with such ID" );
   }

   std::string tpName = rec->getValue<std::string>( s_porosityModelFieldName );

   if (      tpName == "Exponential"        ) { porModel = PorExponential; }
   else if ( tpName == "Soil_Mechanics"     ) { porModel = PorSoilMechanics; }
   else if ( tpName == "Double_Exponential" ) { porModel = PorDoubleExponential; }
   else { return reportError( NonexistingID, std::string( "Unsupported porosity model: " ) + tpName ); }

   // read model parameters
   porModelPrms.clear();
   switch ( porModel )
   {
      case PorExponential:
         porModelPrms.push_back( rec->getValue<double>( s_surfPorosityFieldName ) );
         porModelPrms.push_back( rec->getValue<double>( s_ccExponentialFieldName ) );
         porModelPrms.push_back( rec->getValue<double>( s_minPorosityFieldName ) );
         break;

      case PorSoilMechanics:
         porModelPrms.push_back( rec->getValue<double>( s_surfPorosityFieldName ) );
         porModelPrms.push_back( rec->getValue<double>( s_ccSoilMechanicsFieldName ) );
         break;

      case PorDoubleExponential:
         porModelPrms.push_back( rec->getValue<double>( s_surfPorosityFieldName ) );
         porModelPrms.push_back( rec->getValue<double>( s_minPorosityFieldName ) );
         porModelPrms.push_back( rec->getValue<double>( s_ccaDblExponentialFieldName ) );
         porModelPrms.push_back( rec->getValue<double>( s_ccbDblExponentialFieldName ) );
         break;

      default: return reportError( NonexistingID, std::string( "Unsupported porosity model: " ) + tpName );
   }
   return NoError;
}

// Set lithology porosity model
ErrorHandler::ReturnCode LithologyManagerImpl::setPorosityModel( LithologyID                 id           // [in] lithology ID
                                                               , PorosityModel               porModel     // [in] new type of porosity model
                                                               , const std::vector<double> & porModelPrms // [in] porosity model parameters
                                                               )
{
   // check parameters number
   switch ( porModel )
   {
      case PorExponential:
         if ( porModelPrms.size() != 3 ) return reportError( OutOfRangeValue, "Wrong parameters number for Exponential porosity model" );
         break;

      case PorSoilMechanics:
         if ( porModelPrms.size() != 2 ) return reportError( OutOfRangeValue, "Wrong parameters number for Soil Mechanics porosity model" );
         break;

      case PorDoubleExponential:
         if ( porModelPrms.size() != 4 ) return reportError( OutOfRangeValue, "Wrong parameters number for Double Exponential porosity model" );
         break;

      default: return reportError( NonexistingID, "Unsupported porosity model" );
   }


   // if table does not exist - report error
   if ( !m_lithIoTbl ) return reportError( NonexistingID, std::string( s_lithoTypesTableName ) + " table could not be found in project" );

   size_t recNum = m_lithIoTbl->size();
   if ( id >= recNum ) { return reportError( OutOfRangeValue, "Wrong lithology ID" ); }

   database::Record * rec = m_lithIoTbl->getRecord(  static_cast<int>( id ) );
   if ( !rec ) { return reportError( OutOfRangeValue, "Can't get lithology with given ID from project" ); }

   switch ( porModel )
   {
      case PorExponential:
         if ( porModelPrms[PhiSurf] < 0 || porModelPrms[PhiSurf] > 100 ){
            return reportError( OutOfRangeValue, "Surface porosity value must be in range [0:100]" );
         }
         if ( porModelPrms[CompactionCoef] < 0 || porModelPrms[CompactionCoef] > 50 ){
            return reportError( OutOfRangeValue, "Compaction coefficient value must be in range [0:50]" );
         }
         if ( porModelPrms[PhiMin] < 0 || porModelPrms[CompactionCoef] > 100 ){
            return reportError( OutOfRangeValue, "Minimal porosity value must be in range [0:100]" );
         }
         if ( porModelPrms[PhiMin] > porModelPrms[PhiSurf] ){
            return reportError( OutOfRangeValue, "Minimal porosity value must be less then surface porosity value" );
         }
         rec->setValue( s_surfPorosityFieldName,  porModelPrms[PhiSurf] );
         rec->setValue( s_ccExponentialFieldName, porModelPrms[CompactionCoef] );
         rec->setValue( s_minPorosityFieldName,   porModelPrms[PhiMin] );
         rec->setValue( s_porosityModelFieldName, std::string( "Exponential" ) );
         break;

      case PorSoilMechanics:
         if ( porModelPrms[0] < 0 || porModelPrms[0] > 100 ) return reportError( OutOfRangeValue, "Surface porosity value must be in range [0:100]" );
         if ( porModelPrms[1] < 0 || porModelPrms[1] > 50  ) return reportError( OutOfRangeValue, "Compaction coefficient value must be in range [0:50]" );
         rec->setValue( s_surfPorosityFieldName,    porModelPrms[0] );
         rec->setValue( s_ccSoilMechanicsFieldName, porModelPrms[1] );
         rec->setValue( s_porosityModelFieldName, std::string( "Soil_Mechanics" ) );
         break;

      case PorDoubleExponential:
         if ( porModelPrms[0] < 0 || porModelPrms[0] > 100 ) return reportError( OutOfRangeValue, "Surface porosity value must be in range [0:100]" );
         if ( porModelPrms[1] < 0 || porModelPrms[1] > 100 ) return reportError( OutOfRangeValue, "Minimal porosity value must be in range [0:100]" );
         if ( porModelPrms[1] > porModelPrms[0]            ) return reportError( OutOfRangeValue, "Minimal porosity value must be less then surface porosity value" );
         if ( porModelPrms[2] < 0 || porModelPrms[2] > 50  ) return reportError( OutOfRangeValue, "Compaction coefficient A value must be in range [0:50]" );
         if ( porModelPrms[3] < 0 || porModelPrms[3] > 50  ) return reportError( OutOfRangeValue, "Compaction coefficient B value must be in range [0:50]" );

         rec->setValue( s_surfPorosityFieldName,      porModelPrms[0] );
         rec->setValue( s_minPorosityFieldName,       porModelPrms[1] );
         rec->setValue( s_ccaDblExponentialFieldName, porModelPrms[2] );
         rec->setValue( s_ccbDblExponentialFieldName, porModelPrms[3] );
         rec->setValue( s_porosityModelFieldName, std::string( "Double_Exponential" ) );
         break;

      default: return reportError( NonexistingID, "Unsupported porosity model" );
   }
   return NoError;
}

// Get lithology permeability model
ErrorHandler::ReturnCode LithologyManagerImpl::permeabilityModel( LithologyID           id
                                                                , PermeabilityModel   & prmModel
                                                                , std::vector<double> & modelPrms
                                                                , std::vector<double> & mpPor
                                                                , std::vector<double> & mpPerm
                                                                )
{
   if ( errorCode() != NoError ) resetError();
   try
   {

      // if table does not exist - report error
      if ( !m_lithIoTbl ) { throw Exception( NonexistingID ) << s_lithoTypesTableName << " table could not be found in project"; }

      database::Record * rec = m_lithIoTbl->getRecord( static_cast<int>( id ) );
      if ( !rec ) { throw Exception( NonexistingID ) << "No lithology type with such ID: " << id ; }

      modelPrms.clear();
      const std::string & permModelName = rec->getValue<std::string>( s_permeabilityModelFieldName );
      if (      permModelName == "None"        ) prmModel = PermNone;
      else if ( permModelName == "Sands"       ) prmModel = PermSandstone;
      else if ( permModelName == "Shales"      ) prmModel = PermMudstone;
      else if ( permModelName == "Multipoint"  ) prmModel = PermMultipoint;
      else if ( permModelName == "Impermeable" ) prmModel = PermImpermeable;
      else { throw Exception( UndefinedValue ) << "Unknown permeability model:" << permModelName; }

      // now extract parameters of the permeability model
      switch( prmModel )
      {
         case PermNone:
         case PermImpermeable: break; // no any parameters

         case PermSandstone:
            modelPrms.push_back( rec->getValue<double>( s_permeabilityAnisotropyFieldName ) );
            modelPrms.push_back( rec->getValue<double>( s_DepositionalPermFieldName       ) );
            modelPrms.push_back( rec->getValue<double>( s_permSandClayPercentage          ) );
            break;

         case PermMudstone:
            modelPrms.push_back( rec->getValue<double>( s_permeabilityAnisotropyFieldName ) );
            modelPrms.push_back( rec->getValue<double>( s_DepositionalPermFieldName       ) );
            modelPrms.push_back( rec->getValue<double>( s_mudPermeabilitySensitivityCoeff ) );
            modelPrms.push_back( rec->getValue<double>( s_mudPermeabilityRecoveryCoeff    ) );
            break;

         case PermMultipoint:
            {
               modelPrms.push_back( rec->getValue<double>( s_permeabilityAnisotropyFieldName ) );
               int numPts = rec->getValue<int>( s_mpNumberOfDataPointsFieldName );

               ParseCoefficientsFromString( rec->getValue<std::string>( s_mpPorosityFieldName ), mpPor );
               ParseCoefficientsFromString( rec->getValue<std::string>( s_mpPermpeabilityFieldName ), mpPerm );
               mpPor.resize(  numPts );
               mpPerm.resize( numPts );
            }
            break;
         default: throw Exception( UndefinedValue ) << "Unknown permeability model:" << permModelName;
      }
   }
   catch ( const Exception & e ) { return reportError( e.errorCode(), e.what() ); }

   return NoError;
}

// Set lithology permeability model with parameters
ErrorHandler::ReturnCode LithologyManagerImpl::setPermeabilityModel( LithologyID                 id
                                                                   , PermeabilityModel           prmModel
                                                                   , const std::vector<double> & modelPrms
                                                                   , const std::vector<double> & mpPor
                                                                   , const std::vector<double> & mpPerm
                                                                   )
{
   if ( errorCode() != NoError ) resetError();
   try
   {
      // if table does not exist - report error
      if ( !m_lithIoTbl ) { throw Exception( NonexistingID ) << s_lithoTypesTableName << " table could not be found in project"; }

      database::Record * rec = m_lithIoTbl->getRecord( static_cast<int>( id ) );
      if ( !rec ) { throw Exception( NonexistingID ) << "No lithology type with such ID: " << id ; }

      switch( prmModel )
      {
         case PermNone:
            rec->setValue<std::string>( s_permeabilityModelFieldName, "None" );
            rec->setValue( s_permeabilityAnisotropyFieldName, 1.0 );
            break;

         case PermImpermeable:
            rec->setValue<std::string>( s_permeabilityModelFieldName, "Impermeable" );
            rec->setValue( s_permeabilityAnisotropyFieldName, 1.0 );
            break; // no any parameter for

         case PermSandstone:
            rec->setValue<std::string>( s_permeabilityModelFieldName, "Sands" );
            if ( modelPrms.size() > 0 ) rec->setValue( s_permeabilityAnisotropyFieldName, modelPrms[AnisotropySand] );
            if ( modelPrms.size() > 1 ) rec->setValue( s_DepositionalPermFieldName,       modelPrms[PermSurfSand] );
            if ( modelPrms.size() > 2 ) rec->setValue( s_permSandClayPercentage,          modelPrms[SandClayPercentage] );
            break;

         case PermMudstone:
            rec->setValue<std::string>( s_permeabilityModelFieldName, "Shales" );
            if ( modelPrms.size() > 0 ) rec->setValue( s_permeabilityAnisotropyFieldName, modelPrms[AnisotropyMud] );
            if ( modelPrms.size() > 1 ) rec->setValue( s_DepositionalPermFieldName,       modelPrms[PermSurfMud] );
            if ( modelPrms.size() > 2 ) rec->setValue( s_mudPermeabilitySensitivityCoeff, modelPrms[SensitivityCoef] );
            if ( modelPrms.size() > 3 ) rec->setValue( s_mudPermeabilityRecoveryCoeff,    modelPrms[RecoveryCoeff] );
            break;

         case PermMultipoint:
            {
               rec->setValue<std::string>( s_permeabilityModelFieldName, "Multipoint" );
               if ( modelPrms.size() > 0 ) rec->setValue( s_permeabilityAnisotropyFieldName, modelPrms[0] );

               assert( mpPor.size() == mpPerm.size() );

               rec->setValue( s_mpNumberOfDataPointsFieldName, static_cast<int>( mpPor.size() ) );
               rec->setValue<std::string>( s_mpPorosityFieldName,      PrintCoefficientsToString( mpPor ) );
               rec->setValue<std::string>( s_mpPermpeabilityFieldName, PrintCoefficientsToString( mpPerm ) );
            }
            break;

         default: throw Exception( UndefinedValue ) << "Unknown permeability model:" << prmModel;
      }
   }
   catch ( const Exception & e ) { return reportError( e.errorCode(), e.what() ); }

   return NoError;
}

double LithologyManagerImpl::seisVelocity( LithologyID id )
{
   double val = Utilities::Numerical::IbsNoDataValue;

   if ( errorCode( ) != NoError ) resetError( );
   try
   {
      // if table does not exist - report error
      if ( !m_lithIoTbl ) { throw Exception( NonexistingID ) << s_lithoTypesTableName << " table could not be found in project"; }

      database::Record * rec = m_lithIoTbl->getRecord( static_cast<int>( id ) );
      if ( !rec ) { throw Exception( NonexistingID ) << "No lithology type with such ID: " << id; }

      val = rec->getValue<double>( s_seisVelocityFieldName );
   }
   catch ( const Exception & e )
   {
      reportError( e.errorCode( ), e.what( ) );
   }
   return val;
}

// Set lithology STP thermal conductivity coefficient
double LithologyManagerImpl::stpThermalConductivityCoeff( LithologyID id )
{
   double val = Utilities::Numerical::IbsNoDataValue;

   if ( errorCode() != NoError ) resetError();
   try
   {
      // if table does not exist - report error
      if ( !m_lithIoTbl ) { throw Exception( NonexistingID ) << s_lithoTypesTableName << " table could not be found in project"; }

      database::Record * rec = m_lithIoTbl->getRecord( static_cast<int>( id ) );
      if ( !rec ) { throw Exception( NonexistingID ) << "No lithology type with such ID: " << id ; }

      val = rec->getValue<double>( s_stpThermalCondFieldName );
   }
   catch ( const Exception & e )
   {
      reportError( e.errorCode(), e.what() );
   }
   return val;
}


// Set lithology STP thermal conductivity coefficient
ErrorHandler::ReturnCode LithologyManagerImpl::setSTPThermalConductivityCoeff( LithologyID id      // [in] lithology ID
                                                                             , double stpThermCond // [in] the new value of therm. cond. coeff.
                                                                             )
{
   try
   {
      if ( errorCode() != NoError ) resetError();

      if ( stpThermCond < 0.0 || stpThermCond > 100.0 )
      {
         throw Exception( OutOfRangeValue ) << "STP thermal conductivity value must be in range [0:100] but given is: " << stpThermCond;
      }

      // if table does not exist - report error
      if ( !m_lithIoTbl ) { throw Exception( NonexistingID ) << s_lithoTypesTableName << " table could not be found in project"; }

      database::Record * rec = m_lithIoTbl->getRecord( static_cast<int>( id ) );
      if ( !rec ) { throw Exception( NonexistingID ) << "No lithology type with such ID: " << id ; }

      rec->setValue( s_stpThermalCondFieldName, stpThermCond );
   }
   catch ( const Exception & e )
   {
      return reportError( e.errorCode(), e.what() );
   }
   return NoError;
}


}
