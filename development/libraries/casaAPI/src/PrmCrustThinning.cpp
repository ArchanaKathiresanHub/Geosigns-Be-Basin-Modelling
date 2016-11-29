//                                                                      
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file PrmCrustThinning.C
/// @brief This file keeps API implementation for single event crust thinning parameter handling

// CASA API
#include "PrmCrustThinning.h"
#include "VarPrmCrustThinning.h"

// CMB API
#include "cmbAPI.h"

// Utilities lib
#include <NumericFunctions.h>
#include "Path.h" // for to_string

// STL/C lib
#include <cassert>
#include <cmath>
#include <sstream>

namespace casa
{
static const char * s_crustIoTblName              = "CrustIoTbl";

static const char * s_crustIoTblAgeCol            = "Age";
static const char * s_crustIoTblThicknessCol      = "Thickness";
static const char * s_crustIoTblMapNameCol        = "ThicknessGrid";

static const double s_eps = 1.e-8;

// Constructor. Get parameter values from the model
PrmCrustThinning::PrmCrustThinning( mbapi::Model & mdl ) : m_parent( 0 )
{
   size_t crustIoTblSize = mdl.tableSize( s_crustIoTblName );

   m_initialThickness = UndefinedDoubleValue;
   m_eventsNumber     = 0;

   std::vector<double>      t( crustIoTblSize );
   std::vector<double>      d( crustIoTblSize );
   std::vector<std::string> m( crustIoTblSize );

   for ( size_t i = 0; i < crustIoTblSize; ++i ) // extract crust thickness profile with time, supposed to be sorted
   {
      t[crustIoTblSize - i - 1] = mdl.tableValueAsDouble( s_crustIoTblName, i, s_crustIoTblAgeCol );
      d[crustIoTblSize - i - 1] = mdl.tableValueAsDouble( s_crustIoTblName, i, s_crustIoTblThicknessCol );
      m[crustIoTblSize - i - 1] = mdl.tableValueAsString( s_crustIoTblName, i, s_crustIoTblMapNameCol );
   }

   if ( crustIoTblSize > 0 )
   {
      m_initialThickness = d[0];

      for ( size_t i = 1; i < t.size(); ++i )
      {
         if ( !m[i].empty() ) // got a map 
         {
            if ( m[i-1].empty() || m[i-1] != m[i] ) // change of the map - new event
            {
               m_maps.push_back( m[i] );          // add map
               m_t0.push_back( t[i-1] );            // add start time
               m_dt.push_back( t[i-1] - t[i] );   // add duration
               m_coeff.push_back( 1.0 );          // if we have map change - coeff always 1.0
               ++m_eventsNumber;
            }
         }
         else 
         {
            if ( !m[i-1].empty() )
            {
               throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Crust thinning from map to constant value is not supported";
            }
            if ( !NumericFunctions::isEqual( d[i-1], d[i], s_eps )  )
            {
               m_maps.push_back( "" );           // add no map
               m_t0.push_back( t[i-1] );         // add start time
               m_dt.push_back( t[i-1] - t[i] );  // add duration
               m_coeff.push_back( d[i-1] > s_eps ? d[i]/d[i-1] : 1.0 ); // thinning factor
               ++m_eventsNumber;
            }
         }
      }
   }
}

// Constructor. Set given parameter values
PrmCrustThinning::PrmCrustThinning( const VarPrmCrustThinning * parent, const std::vector<double> & prmValues, const std::vector<std::string> & mapsList ) : 
     m_parent( parent )
{
   m_maps.insert( m_maps.begin(), mapsList.begin(), mapsList.end() ); // copy maps
   m_eventsNumber = mapsList.size();

   if ( m_eventsNumber != ((prmValues.size() - 1) / 3) ) throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Wrong parameters number for CrustThinning";

   m_initialThickness = prmValues[0];

   for ( size_t i = 1; i < prmValues.size(); ++i )
   {
      m_t0.push_back( prmValues[i++] );
      m_dt.push_back( prmValues[i++] );
      m_coeff.push_back( prmValues[i] );
   }
}

// Set this parameter value in Cauldron model
ErrorHandler::ReturnCode PrmCrustThinning::setInModel( mbapi::Model & caldModel, size_t caseID )
{
   try
   {
      // get age of the eldest layer
      double eldestAge = caldModel.stratigraphyManager().eldestLayerAge();
      if ( eldestAge == UndefinedDoubleValue )
      {
         throw ErrorHandler::Exception( caldModel.stratigraphyManager().errorCode() ) << caldModel.stratigraphyManager().errorMessage();
      }

      // get model maps manager
      mbapi::MapsManager & mMgr = caldModel.mapsManager();

      // create history
      std::vector<double>      t;
      std::vector<double>      d;
      std::vector<std::string> m;
      
      // initial point - always constant thickness
      t.push_back( eldestAge );
      d.push_back( m_initialThickness );
      m.push_back( "" );

      for ( size_t i = 0; i < m_eventsNumber; ++i )
      {
         // event start point
         t.push_back( m_t0[i] );
         d.push_back( d.back() );
         m.push_back( m.back() );

         // event end point
         t.push_back( m_t0[i] - m_dt[i] );

         if ( m_maps[i].empty() && m.back().empty() ) // just constants
         {
            d.push_back( d.back() * m_coeff[i] );
            m.push_back( "" );
            continue;
         }

         // will be some map
         std::string oldMapName = m_maps[i].empty() ? m.back() : m_maps[i];
         std::string newMapName = oldMapName;

         if ( !NumericFunctions::isEqual( m_coeff[i], 1.0, s_eps ) ) // copy and scale maps
         {
            mbapi::MapsManager::MapID id = mMgr.findID( oldMapName );
            if ( UndefinedIDValue == id )
            {
               throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Crust thinning, unknown map is given: " << oldMapName;
            }
            newMapName += std::string( "_Case_" ) + ibs::to_string( caseID );
            newMapName += "_CrustThinningEvent_";
            newMapName += ibs::to_string( i );

            id = mMgr.copyMap( id, newMapName );
            if ( UndefinedIDValue == id )
            {
               throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Crust thinning, copying " << oldMapName << " map failed";
            }

            if ( ErrorHandler::NoError != mMgr.scaleMap( id, m_coeff[i]              ) )
            {
               throw ErrorHandler::Exception( mMgr.errorCode() ) << mMgr.errorMessage();
            }
            if ( ErrorHandler::NoError != mMgr.saveMapToHDF( id, newMapName + ".HDF", 0 ) )
            {
               throw ErrorHandler::Exception( mMgr.errorCode() ) << mMgr.errorMessage();
            }
         }
         d.push_back( UndefinedDoubleValue );
         m.push_back( newMapName ); // just put map - no any scaling

         // because we generated new map - replace scaler with new map name, otherwise validation will fail
         m_coeff[i] = 1.0;
         m_maps[i] = newMapName;
      }

      if ( t.back() != 0.0 ) // add last point - present time
      {
         t.push_back( 0.0 );
         d.push_back( d.back() );
         m.push_back( m.back() );
      }

      bool ok = true;
      // add rows to the table eventsNumber + 2
      ok = ok ? ErrorHandler::NoError == caldModel.clearTable( s_crustIoTblName ) : ok;

      for ( size_t i = 0; i < m.size() && ok; ++i )
      {
         ok = ok ? ErrorHandler::NoError == caldModel.addRowToTable( s_crustIoTblName ) : ok;
      }

      for ( size_t i = 0; i < m.size() && ok; ++i )
      {
         ok = ok ? ErrorHandler::NoError == caldModel.setTableValue( s_crustIoTblName, i, s_crustIoTblAgeCol, t[t.size() - i - 1] ) : ok;
         ok = ok ? ErrorHandler::NoError == caldModel.setTableValue( s_crustIoTblName, i, s_crustIoTblThicknessCol, d[d.size() - i - 1] ) : ok;
         ok = ok ? ErrorHandler::NoError == caldModel.setTableValue( s_crustIoTblName, i, s_crustIoTblMapNameCol, m[m.size() - i - 1] ) : ok;
     }
     if ( !ok ) { throw ErrorHandler::Exception( caldModel.errorCode() ) << caldModel.errorMessage(); }
   }
   catch( const ErrorHandler::Exception & ex ) { return caldModel.reportError( ex.errorCode(), ex.what() ); }

   return ErrorHandler::NoError;
}

// Validate crust thinning parameter values
std::string PrmCrustThinning::validate( mbapi::Model & caldModel )
{
   std::ostringstream oss;

   if (      m_initialThickness < 0.0    ) { oss << "Initial crust thickness value can not be negative: "                             << m_initialThickness << "\n"; }
   else if ( m_initialThickness > 100000 ) { oss << "Too big value for the initial crust thickness, it can not be more then 100 km: " << m_initialThickness << "\n"; }

   if ( m_eventsNumber == 0 ) oss << "No any crust thinnnig event is defined\n";


   for ( size_t i = 0; i < m_eventsNumber; ++ i )
   {
      if      ( m_t0[i] <= 0.0   ) { oss << "Starting time for the crust thinning event " << ibs::to_string( i ) << " must be greater than 0: " << m_t0[i] << "\n"; }
      else if ( m_t0[i] > 1000.0 ) { oss << "Too big value for the starting time of crust thinning event " << ibs::to_string( i ) << ": "       << m_t0[i] << "\n"; }
   
      if (      m_dt[i] < 0      ) { oss << "Duration of the crust thinning event " << ibs::to_string( i ) << " can not be negative: "                   << m_dt[i] << "\n"; }
      else if ( m_dt[i] > m_t0[i]) { oss << "Duration of the crust thinning event " << ibs::to_string( i ) << " can not be greater than starting time: " << m_dt[i] << "\n"; }

      if (      m_coeff[i] < 0 ) { oss << "Crust thinning factor of the crust thinning event " << ibs::to_string( i ) << " can not be negative: "      << m_coeff[i] << "\n"; }
      else if ( m_coeff[i] > 1 ) { oss << "Crust thinning factor of the crust thinning event " << ibs::to_string( i ) << " can not greater than 1.0: " << m_coeff[i] << "\n"; }

      if ( i > 0 )
      {
         if ( m_t0[i-1] - m_dt[i-1] < m_t0[i] ) 
         {
            oss << "Crust thinning events: " << ibs::to_string( i ) << " and " << ibs::to_string( i ) << " are overlapping\n";
         }
      }
   }
   PrmCrustThinning mprm( caldModel );
   if ( mprm != *this ) { oss << "Mutated project has a mismatched profile of crust thinning event for the crust thinning history\n"; }

   return oss.str(); // another model, no reason to check further
}
  
// Get parameter value as an array of doubles
std::vector<double> PrmCrustThinning::asDoubleArray() const
{
   std::vector<double> vals;

   vals.push_back( m_initialThickness );
   for ( size_t i = 0; i < m_eventsNumber; ++i )
   {
      vals.push_back( m_t0[i] );
      vals.push_back( m_dt[i] );
      vals.push_back( m_coeff[i] );
   }
   return vals;
}

// Are two parameters equal?
bool PrmCrustThinning::operator == ( const Parameter & prm ) const
{
   const PrmCrustThinning * pp = dynamic_cast<const PrmCrustThinning *>( &prm );
   if ( !pp ) return false;
   
   const double eps = 1.e-4;

   if ( !NumericFunctions::isEqual( m_initialThickness, pp->m_initialThickness, eps ) ) return false;
   if ( m_eventsNumber != pp->m_eventsNumber ) return false;

   for ( size_t i = 0; i < m_eventsNumber; ++i )
   {
      if ( !NumericFunctions::isEqual( m_t0[i],    pp->m_t0[i],    eps ) ) return false;
      if ( !NumericFunctions::isEqual( m_dt[i],    pp->m_dt[i],    eps ) ) return false;
      if ( !NumericFunctions::isEqual( m_coeff[i], pp->m_coeff[i], eps ) ) return false;

      if ( m_maps[i] != pp->m_maps[i] ) return false;
   }

   return true;
}


// Save all object data to the given stream, that object could be later reconstructed from saved data
bool PrmCrustThinning::save( CasaSerializer & sz, unsigned int /* version */) const
{
   bool hasParent = m_parent ? true : false;
   bool ok = sz.save( hasParent, "hasParent" );
   
   if ( hasParent )
   {
      CasaSerializer::ObjRefID parentID = sz.ptr2id( m_parent );
      ok = ok ? sz.save( parentID, "VarParameterID" ) : ok;
   }
   ok = ok ? sz.save( m_name,             "name"             ) : ok;
   ok = ok ? sz.save( m_initialThickness, "initialThickness" ) : ok;
   ok = ok ? sz.save( m_t0,               "t0"               ) : ok;
   ok = ok ? sz.save( m_dt,               "dt"               ) : ok;
   ok = ok ? sz.save( m_coeff,            "coeff"            ) : ok;
   ok = ok ? sz.save( m_maps,             "maps"             ) : ok;

   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
PrmCrustThinning::PrmCrustThinning( CasaDeserializer & dz, unsigned int /* objVer */)
{
   CasaDeserializer::ObjRefID parentID;

   bool hasParent;
   bool ok = dz.load( hasParent, "hasParent" );
   
   if ( hasParent )
   {
      bool ok = dz.load( parentID, "VarParameterID" );
      m_parent = ok ? dz.id2ptr<VarParameter>( parentID ) : 0;
   }

   ok = ok ? dz.load( m_name,             "name" )             : ok;
   ok = ok ? dz.load( m_initialThickness, "initialThickness" ) : ok;
   ok = ok ? dz.load( m_t0,               "t0" )               : ok;
   ok = ok ? dz.load( m_dt,               "dt" )               : ok;
   ok = ok ? dz.load( m_coeff,            "coeff" )            : ok;
   ok = ok ? dz.load( m_maps,             "maps" )             : ok;

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "PrmCrustThinning deserialization unknown error";
   }
}

}


