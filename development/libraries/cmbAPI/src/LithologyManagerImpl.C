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

// DataAccess
#include "database.h"

// STL
#include <stdexcept>
#include <string>
#include <cmath>

namespace mbapi
{

const char * LithologyManagerImpl::m_lithoTypesTableName        = "LithotypeIoTbl";
const char * LithologyManagerImpl::m_lithoTypeNameFieldName     = "Lithotype";
const char * LithologyManagerImpl::m_porosityModelFieldName     = "Porosity_Model"; 
const char * LithologyManagerImpl::m_surfPorosityFieldName      = "SurfacePorosity";
const char * LithologyManagerImpl::m_ccExponentialFieldName     = "CompacCoefES";
const char * LithologyManagerImpl::m_ccaDblExponentialFieldName = "CompacCoefESA";
const char * LithologyManagerImpl::m_ccbDblExponentialFieldName = "CompacCoefESB";
const char * LithologyManagerImpl::m_ccSoilMechanicsFieldName   = "Compaction_Coefficient_SM";
const char * LithologyManagerImpl::m_minPorosityFieldName       = "MinimumPorosity";

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
   database::Table * table = m_db->getTable( m_lithoTypesTableName );

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


// Get lithology name
std::string LithologyManagerImpl::lithologyName( LithologyID id )
{
   if ( errorCode() != NoError ) resetError();

   std::string lName;

   // get pointer to the table
   database::Table * table = m_db->getTable( m_lithoTypesTableName );

   // if table does not exist - report error
   if ( !table )
   {
      reportError( NonexistingID, std::string( m_lithoTypesTableName ) + " table could not be found in project" );
      return lName;
   }

   database::Record * rec = table->getRecord( static_cast<int>( id ) );
   if ( !rec )
   {
      reportError( NonexistingID, "No lithology type with such ID" );
      return lName;
   }
   lName = rec->getValue<std::string>( m_lithoTypeNameFieldName );

   return lName;
}

// Get lithology porosity model
ErrorHandler::ReturnCode LithologyManagerImpl::porosityModel( LithologyID         id              // [in] lithology ID
                                                            , PorosityModel       & porModel      // [out] porosity model type
                                                            , std::vector<double> & porModelPrms  // [out] porosity model parameters
                                                            )
{
   if ( errorCode() != NoError ) resetError();

   // get pointer to the table
   database::Table * table = m_db->getTable( m_lithoTypesTableName );

   // if table does not exist - report error
   if ( !table )
   {
      return reportError( NonexistingID, std::string( m_lithoTypesTableName ) + " table could not be found in project" );
   }

   database::Record * rec = table->getRecord( static_cast<int>( id ) );
   if ( !rec )
   {
      return reportError( NonexistingID, "No lithology type with such ID" );
   }

   std::string tpName = rec->getValue<std::string>( m_porosityModelFieldName );

   if (      tpName == "Exponential"        ) { porModel = Exponential; }
   else if ( tpName == "Soil_Mechanics"     ) { porModel = SoilMechanics; }
   else if ( tpName == "Double_Exponential" ) { porModel = DoubleExponential; }
   else { return reportError( NonexistingID, std::string( "Unsupported porosity model: " ) + tpName ); }

   // read model parameters
   porModelPrms.clear();
   switch ( porModel )
   {
      case Exponential:
         porModelPrms.push_back( rec->getValue<double>( m_surfPorosityFieldName ) );
         porModelPrms.push_back( rec->getValue<double>( m_ccExponentialFieldName ) );
         break;

      case SoilMechanics:
         porModelPrms.push_back( rec->getValue<double>( m_surfPorosityFieldName ) );
         porModelPrms.push_back( rec->getValue<double>( m_ccSoilMechanicsFieldName ) );
         break;
      
      case DoubleExponential:
         porModelPrms.push_back( rec->getValue<double>( m_surfPorosityFieldName ) );
         porModelPrms.push_back( rec->getValue<double>( m_minPorosityFieldName ) );
         porModelPrms.push_back( rec->getValue<double>( m_ccaDblExponentialFieldName ) );
         porModelPrms.push_back( rec->getValue<double>( m_ccbDblExponentialFieldName ) );
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
      case Exponential:
         if ( porModelPrms.size() != 2 ) return reportError( OutOfRangeValue, "Wrong parameters number for Exponential porosity model" );
         break;

      case SoilMechanics:
         if ( porModelPrms.size() != 2 ) return reportError( OutOfRangeValue, "Wrong parameters number for Soil Mechanics porosity model" );
         break;

      case DoubleExponential:
         if ( porModelPrms.size() != 4 ) return reportError( OutOfRangeValue, "Wrong parameters number for Double Exponential porosity model" );
         break;

      default: return reportError( NonexistingID, "Unsupported porosity model" );
   }

   // get pointer to the table
   database::Table * table = m_db->getTable( m_lithoTypesTableName );

   // if table does not exist - report error
   if ( !table ) return reportError( NonexistingID, std::string( m_lithoTypesTableName ) + " table could not be found in project" );
   
   size_t recNum = table->size();
   if ( id >= recNum ) { return reportError( OutOfRangeValue, "Wrong lithology ID" ); }
      
   database::Record * rec = table->getRecord(  static_cast<int>( id ) );
   if ( !rec ) { return reportError( OutOfRangeValue, "Can't get lithology with given ID from project" ); }

   switch ( porModel )
   {
      case Exponential:
         if ( porModelPrms[0] < 0 || porModelPrms[0] > 100 ) return reportError( OutOfRangeValue, "Surface porosity value must be in range [0:100]" );
         if ( porModelPrms[1] < 0 || porModelPrms[1] > 50  ) return reportError( OutOfRangeValue, "Compaction coefficient value must be in range [0:50]" );
         rec->setValue( m_surfPorosityFieldName,  porModelPrms[0] );
         rec->setValue( m_ccExponentialFieldName, porModelPrms[1] );
         break;

      case SoilMechanics:
         if ( porModelPrms[0] < 0 || porModelPrms[0] > 100 ) return reportError( OutOfRangeValue, "Surface porosity value must be in range [0:100]" );
         if ( porModelPrms[1] < 0 || porModelPrms[1] > 50  ) return reportError( OutOfRangeValue, "Compaction coefficient value must be in range [0:50]" );
         rec->setValue( m_surfPorosityFieldName,    porModelPrms[0] );
         rec->setValue( m_ccSoilMechanicsFieldName, porModelPrms[1] );
         break;

      case DoubleExponential:
         if ( porModelPrms[0] < 0 || porModelPrms[0] > 100 ) return reportError( OutOfRangeValue, "Surface porosity value must be in range [0:100]" );
         if ( porModelPrms[1] < 0 || porModelPrms[1] > 100 ) return reportError( OutOfRangeValue, "Minimal porosity value must be in range [0:100]" );
         if ( porModelPrms[1] > porModelPrms[0]            ) return reportError( OutOfRangeValue, "Minimal porosity value must be less then surface porosity value" );
         if ( porModelPrms[2] < 0 || porModelPrms[2] > 50  ) return reportError( OutOfRangeValue, "Compaction coefficient A value must be in range [0:50]" );
         if ( porModelPrms[3] < 0 || porModelPrms[3] > 50  ) return reportError( OutOfRangeValue, "Compaction coefficient B value must be in range [0:50]" );

         rec->setValue( m_surfPorosityFieldName,      porModelPrms[0] );
         rec->setValue( m_minPorosityFieldName,       porModelPrms[1] );
         rec->setValue( m_ccaDblExponentialFieldName, porModelPrms[2] );
         rec->setValue( m_ccbDblExponentialFieldName, porModelPrms[3] );
         break;
   }
   return NoError;
}


// Set lithology STP thermal conductivity coefficient
ErrorHandler::ReturnCode LithologyManagerImpl::STPThermalConductivityCoeff( LithologyID id        // [in] lithology ID
                                                                          , double & stpThermCond // [out] thermal cond. coeff., or unchanged on error
                                                                          )
{
   throw std::runtime_error( "Not implemented yet" );
   return NotImplementedAPI;
}


// Set lithology STP thermal conductivity coefficient
ErrorHandler::ReturnCode LithologyManagerImpl::setSTPThermalConductivityCoeff( LithologyID id      // [in] lithology ID
                                                                             , double stpThermCond // [in] the new value of therm. cond. coeff.
                                                                             )
{
   throw std::runtime_error( "Not implemented yet" );
   return NotImplementedAPI;
}


}
