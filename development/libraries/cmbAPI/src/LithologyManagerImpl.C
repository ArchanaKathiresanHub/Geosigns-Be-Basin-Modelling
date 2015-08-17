//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
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
#include "UndefinedValues.h"

// DataAccess
#include "database.h"

// STL
#include <stdexcept>
#include <string>
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
}

// Copy operator
LithologyManagerImpl & LithologyManagerImpl::operator = ( const LithologyManagerImpl & otherLithMgr )
{
   throw std::runtime_error( "Not implemented yet" );
   return *this;
}

// Set project database. Reset all
void LithologyManagerImpl::setDatabase( database::Database * db )
{
   m_db = db;
}

// Get list of lithologies in the model
std::vector<LithologyManager::LithologyID> LithologyManagerImpl::lithologiesIDs() const
{
   std::vector<LithologyID> ltIDs;
   if ( !m_db ) return ltIDs;

   // get pointer to the table
   database::Table * table = m_db->getTable( s_lithoTypesTableName );

   // if table does not exist - return empty array
   if ( !table ) return ltIDs;

   // fill IDs array with increasing indexes
   ltIDs.resize( table->size(), 0 );
 
   for ( size_t i = 0; i < ltIDs.size(); ++i ) ltIDs[ i ] = static_cast<LithologyID>( i );

   return ltIDs;
}

// Create new lithology
LithologyManager::LithologyID LithologyManagerImpl::createNewLithology()
{
   throw std::runtime_error( "Not implemented yet" );
}

// Make a copy of the given lithology. Also makes a new set of records in table [LitThCondIoTbl] for the new litholog
LithologyManager::LithologyID LithologyManagerImpl::copyLithology( LithologyID id, const std::string & newLithoName )
{
   LithologyID ret = UndefinedIDValue;

   if ( errorCode() != NoError ) resetError();
   try
   {
      // first check if given name already exist
      if ( findID( newLithoName ) != UndefinedIDValue ) { throw Exception( AlreadyDefined ) << "Create copy: " << newLithoName << ", already exist in the lithology table"; }

      // proceed with copy
      // get pointer to the table
      database::Table * table = m_db->getTable( s_lithoTypesTableName );

      // if table does not exist - report error
      if ( !table ) { throw Exception( NonexistingID ) <<  s_lithoTypesTableName << " table could not be found in project"; }

      // get record for copy
      database::Record * origRec = table->getRecord( id );
      if ( !origRec ) { throw Exception( NonexistingID ) << "No lithology type with such ID: " << id; }

      // create a copy of lithology
      database::Record * copyRec = new database::Record( *origRec );

      // get the orig lithology name
      const std::string & origLithoName = origRec->getValue<std::string>( s_lithoTypeNameFieldName );

      // change the name
      copyRec->setValue( s_lithoTypeNameFieldName, newLithoName );
      // add copy record with new name to the table end
      table->addRecord( copyRec );

      // duplicate records in Thermal conductivity and heat capacity tables
      for ( size_t j = 0; j < 2; ++j ) // first process thermal conductivity then heat capacity
      {
         const std::string & tblName = j == 0 ? s_lithoThCondTableName : s_lithoHeatCapTableName;
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

      // if all is OK - create the new LithologyID for lithology copy
      ret = table->size() - 1;
   }
   catch( const Exception & ex ) { reportError( ex.errorCode(), ex.what() ); }

   return ret;
}


// Get lithology name
std::string LithologyManagerImpl::lithologyName( LithologyID id )
{
   if ( errorCode() != NoError ) resetError();

   std::string lName;

   // get pointer to the table
   database::Table * table = m_db->getTable( s_lithoTypesTableName );

   // if table does not exist - report error
   if ( !table )
   {
      reportError( NonexistingID, std::string( s_lithoTypesTableName ) + " table could not be found in project" );
      return lName;
   }

   database::Record * rec = table->getRecord( static_cast<int>( id ) );
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
      // get pointer to the table
      database::Table * table = m_db->getTable( s_lithoTypesTableName );

      // if table does not exist - report error
      if ( !table ) { throw Exception( NonexistingID ) << s_lithoTypesTableName << " table could not be found in project"; }

      int tblSize = table->size();
      for ( int i = 0; i < tblSize; ++i )
      {
         database::Record * rec = table->getRecord( i );
         if ( !rec ) { throw Exception( NonexistingID ) << "No lithology type with such ID: " << i; }

         if ( lName == rec->getValue<std::string>( s_lithoTypeNameFieldName ) )
         {
            return static_cast<LithologyManager::LithologyID>( i );
         }
      }
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return UndefinedIDValue;
}



// Get lithology porosity model
ErrorHandler::ReturnCode LithologyManagerImpl::porosityModel( LithologyID         id              // [in] lithology ID
                                                            , PorosityModel       & porModel      // [out] porosity model type
                                                            , std::vector<double> & porModelPrms  // [out] porosity model parameters
                                                            )
{
   if ( errorCode() != NoError ) resetError();

   // get pointer to the table
   database::Table * table = m_db->getTable( s_lithoTypesTableName );

   // if table does not exist - report error
   if ( !table )
   {
      return reportError( NonexistingID, std::string( s_lithoTypesTableName ) + " table could not be found in project" );
   }

   database::Record * rec = table->getRecord( static_cast<int>( id ) );
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
         if ( porModelPrms.size() != 2 ) return reportError( OutOfRangeValue, "Wrong parameters number for Exponential porosity model" );
         break;

      case PorSoilMechanics:
         if ( porModelPrms.size() != 2 ) return reportError( OutOfRangeValue, "Wrong parameters number for Soil Mechanics porosity model" );
         break;

      case PorDoubleExponential:
         if ( porModelPrms.size() != 4 ) return reportError( OutOfRangeValue, "Wrong parameters number for Double Exponential porosity model" );
         break;

      default: return reportError( NonexistingID, "Unsupported porosity model" );
   }

   // get pointer to the table
   database::Table * table = m_db->getTable( s_lithoTypesTableName );

   // if table does not exist - report error
   if ( !table ) return reportError( NonexistingID, std::string( s_lithoTypesTableName ) + " table could not be found in project" );
   
   size_t recNum = table->size();
   if ( id >= recNum ) { return reportError( OutOfRangeValue, "Wrong lithology ID" ); }
      
   database::Record * rec = table->getRecord(  static_cast<int>( id ) );
   if ( !rec ) { return reportError( OutOfRangeValue, "Can't get lithology with given ID from project" ); }

   switch ( porModel )
   {
      case PorExponential:
         if ( porModelPrms[0] < 0 || porModelPrms[0] > 100 ) return reportError( OutOfRangeValue, "Surface porosity value must be in range [0:100]" );
         if ( porModelPrms[1] < 0 || porModelPrms[1] > 50  ) return reportError( OutOfRangeValue, "Compaction coefficient value must be in range [0:50]" );
         rec->setValue( s_surfPorosityFieldName,  porModelPrms[0] );
         rec->setValue( s_ccExponentialFieldName, porModelPrms[1] );
         break;

      case PorSoilMechanics:
         if ( porModelPrms[0] < 0 || porModelPrms[0] > 100 ) return reportError( OutOfRangeValue, "Surface porosity value must be in range [0:100]" );
         if ( porModelPrms[1] < 0 || porModelPrms[1] > 50  ) return reportError( OutOfRangeValue, "Compaction coefficient value must be in range [0:50]" );
         rec->setValue( s_surfPorosityFieldName,    porModelPrms[0] );
         rec->setValue( s_ccSoilMechanicsFieldName, porModelPrms[1] );
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
         break;
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

      // get pointer to the table
      database::Table * table = m_db->getTable( s_lithoTypesTableName );

      // if table does not exist - report error
      if ( !table ) { throw Exception( NonexistingID ) << s_lithoTypesTableName << " table could not be found in project"; }

      database::Record * rec = table->getRecord( static_cast<int>( id ) );
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
      // get pointer to the table
      database::Table * table = m_db->getTable( s_lithoTypesTableName );

      // if table does not exist - report error
      if ( !table ) { throw Exception( NonexistingID ) << s_lithoTypesTableName << " table could not be found in project"; }

      database::Record * rec = table->getRecord( static_cast<int>( id ) );
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
            if ( modelPrms.size() > 0 ) rec->setValue( s_permeabilityAnisotropyFieldName, modelPrms[0] );
            if ( modelPrms.size() > 1 ) rec->setValue( s_DepositionalPermFieldName,       modelPrms[1] );
            if ( modelPrms.size() > 2 ) rec->setValue( s_permSandClayPercentage,          modelPrms[2] );
            break;

         case PermMudstone:
            rec->setValue<std::string>( s_permeabilityModelFieldName, "Shales" );
            if ( modelPrms.size() > 0 ) rec->setValue( s_permeabilityAnisotropyFieldName, modelPrms[0] );
            if ( modelPrms.size() > 1 ) rec->setValue( s_DepositionalPermFieldName,       modelPrms[1] );
            if ( modelPrms.size() > 2 ) rec->setValue( s_mudPermeabilitySensitivityCoeff, modelPrms[2] );
            if ( modelPrms.size() > 3 ) rec->setValue( s_mudPermeabilityRecoveryCoeff,    modelPrms[3] );
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
      }
   }
   catch ( const Exception & e ) { return reportError( e.errorCode(), e.what() ); }

   return NoError;
}
 
// Set lithology STP thermal conductivity coefficient
double LithologyManagerImpl::stpThermalConductivityCoeff( LithologyID id )
{
   double val = UndefinedDoubleValue;

   if ( errorCode() != NoError ) resetError();
   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( s_lithoTypesTableName );

      // if table does not exist - report error
      if ( !table ) { throw Exception( NonexistingID ) << s_lithoTypesTableName << " table could not be found in project"; }

      database::Record * rec = table->getRecord( static_cast<int>( id ) );
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
 
      // get pointer to the table
      database::Table * table = m_db->getTable( s_lithoTypesTableName );

      // if table does not exist - report error
      if ( !table ) { throw Exception( NonexistingID ) << s_lithoTypesTableName << " table could not be found in project"; }

      database::Record * rec = table->getRecord( static_cast<int>( id ) );
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
