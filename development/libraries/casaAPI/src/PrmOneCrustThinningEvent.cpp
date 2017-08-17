//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PrmOneCrustThinningEvent.C
/// @brief This file keeps API implementation for single event crust thinning parameter handling

// CASA API
#include "PrmOneCrustThinningEvent.h"
#include "VarPrmOneCrustThinningEvent.h"

// CMB API
#include "cmbAPI.h"

// Utilities lib
#include <NumericFunctions.h>

// STL/C lib
#include <cassert>
#include <cmath>
#include <sstream>

using namespace Utilities::Numerical;

namespace casa
{

static const char * s_stratIoTblName              = "StratIoTbl";
static const char * s_stratIoTblAgeCol            = "DepoAge";
static const char * s_crustIoTblName              = "CrustIoTbl";
static const char * s_crustIoTblAgeCol            = "Age";
static const char * s_crustIoTblThicknessCol      = "Thickness";

static const double s_eps = 1.e-8;

// Constructor. Get parameter values from the model
PrmOneCrustThinningEvent::PrmOneCrustThinningEvent( mbapi::Model & mdl ) : m_parent( 0 )
{
   size_t crustIoTblSize = mdl.tableSize( s_crustIoTblName );

   m_initialThickness = IbsNoDataValue;
   m_t0               = IbsNoDataValue;
   m_dt               = IbsNoDataValue;
   m_coeff            = IbsNoDataValue;

   if ( ErrorHandler::NoError == mdl.errorCode() )
   {
      std::vector<double> t( crustIoTblSize );
      std::vector<double> d( crustIoTblSize );

      for ( size_t i = 0; i < crustIoTblSize; ++i ) // extract crust thickness profile with time, supposed to be sorted
      {
         t[i] = mdl.tableValueAsDouble( s_crustIoTblName, i, s_crustIoTblAgeCol );
         d[i] = mdl.tableValueAsDouble( s_crustIoTblName, i, s_crustIoTblThicknessCol );
      }

      if ( 4 == crustIoTblSize )
      {
         m_initialThickness = d.back();
         m_coeff = d.front() / d.back();
         m_t0 = t[2];
         m_dt = t[2] - t[1];
      }
   }
}

// Constructor. Set given parameter values
PrmOneCrustThinningEvent::PrmOneCrustThinningEvent( const VarPrmOneCrustThinningEvent * parent
                                                  , double                              thickIni
                                                  , double                              t0
                                                  , double                              dt
                                                  , double                              coeff
                                                  )
                                                  : m_parent( parent )
                                                  , m_initialThickness( thickIni ) 
                                                  , m_t0( t0 )
                                                  , m_dt( dt )
                                                  , m_coeff( coeff ) {}

// Set this parameter value in Cauldron model
ErrorHandler::ReturnCode PrmOneCrustThinningEvent::setInModel( mbapi::Model & mdl, size_t /* caseID */ )
{
   bool oK = true;
   oK = oK ? ErrorHandler::NoError == mdl.clearTable( s_crustIoTblName ) : oK;
   if ( !oK ) return mdl.errorCode();

   // get position in table the eldest layer
   int firstLayerRow = mdl.tableSize( s_stratIoTblName ) - 1;
   oK = oK ? ErrorHandler::NoError == mdl.errorCode() : oK;
   if ( !oK ) return mdl.errorCode();

   // get age of the eldest layer
   double eldestAge = mdl.tableValueAsDouble( s_stratIoTblName, firstLayerRow, s_stratIoTblAgeCol );
   oK = oK ? ErrorHandler::NoError == mdl.errorCode() : oK;

   // add 4 rows to the table
   for ( size_t i = 0; i < 4 && oK; ++i )
   {
      oK = oK ? ErrorHandler::NoError == mdl.addRowToTable( s_crustIoTblName ) : oK;
   }

   // 0 time
   oK = oK ? ErrorHandler::NoError == mdl.setTableValue( s_crustIoTblName, 0, s_crustIoTblAgeCol,       0.0                          ) : oK;
   oK = oK ? ErrorHandler::NoError == mdl.setTableValue( s_crustIoTblName, 0, s_crustIoTblThicknessCol, m_initialThickness * m_coeff ) : oK;
   // end event time
   oK = oK ? ErrorHandler::NoError == mdl.setTableValue( s_crustIoTblName, 1, s_crustIoTblAgeCol,       m_t0 - m_dt                  ) : oK;
   oK = oK ? ErrorHandler::NoError == mdl.setTableValue( s_crustIoTblName, 1, s_crustIoTblThicknessCol, m_initialThickness * m_coeff ) : oK;
   // request snapshot at this time
   // start event time
   oK = oK ? ErrorHandler::NoError == mdl.setTableValue( s_crustIoTblName, 2, s_crustIoTblAgeCol,       m_t0               ) : oK;
   oK = oK ? ErrorHandler::NoError == mdl.setTableValue( s_crustIoTblName, 2, s_crustIoTblThicknessCol, m_initialThickness ) : oK;

   // before time begin
   oK = oK ? ErrorHandler::NoError == mdl.setTableValue( s_crustIoTblName, 3, s_crustIoTblAgeCol,       eldestAge          ) : oK;
   oK = oK ? ErrorHandler::NoError == mdl.setTableValue( s_crustIoTblName, 3, s_crustIoTblThicknessCol, m_initialThickness ) : oK;

   // request snapshots at start event time and at finish event time
   oK = oK ? ErrorHandler::NoError == mdl.snapshotManager().requestMajorSnapshot( m_t0 - m_dt ) : oK;
   oK = oK ? ErrorHandler::NoError == mdl.snapshotManager().requestMajorSnapshot( m_t0        ) : oK;

   return oK ? ErrorHandler::NoError : mdl.errorCode();
}

// Validate crust thinning parameter values
std::string PrmOneCrustThinningEvent::validate( mbapi::Model & caldModel )
{
   std::ostringstream oss;

   if ( m_initialThickness < 0.0    ) { oss << "Initial crust thickness value can not be negative: "   << m_initialThickness << "\n"; }
   if ( m_initialThickness > 100000 ) { oss << "Initial crust thickness can not be more then 100 km: " << m_initialThickness << "\n"; }

   if ( m_t0 <= 0.0   ) { oss << "Starting time for the crust thinning event must be greater than 0: " << m_t0 << "\n"; }
   if ( m_t0 > 1000.0 ) { oss << "Too big value for the starting time of crust thinning event: "       << m_t0 << "\n"; }

   if ( m_dt < 0    ) { oss << "Duration of the crust thinning event can not be negative: "                   << m_dt << "\n"; }
   if ( m_dt > m_t0 ) { oss << "Duration of the crust thinning event can not be greater than starting time: " << m_dt << "\n"; }

   if ( m_coeff < 0 ) { oss << "Crust thinning factor can not be negative: "      << m_coeff << "\n"; }
   if ( m_coeff > 1 ) { oss << "Crust thinning factor can not greater than 1.0: " << m_coeff << "\n"; }
 

   int crustIoTblSize = caldModel.tableSize( s_crustIoTblName );
   
   if ( ErrorHandler::NoError != caldModel.errorCode() )
   {
      oss << caldModel.errorMessage() << std::endl;
      return oss.str();
   }
   
   if ( crustIoTblSize != 4 ) { oss << "Project has a mismatched profile to single crust thinning event for the crust thinning history\n"; }

   double t[4];
   double d[4];
   for ( size_t i = 0; i < 4; ++i ) // extract crust thickness profile with time, supposed to be sorted
   {
      t[i] = caldModel.tableValueAsDouble( s_crustIoTblName, i, s_crustIoTblAgeCol );
      d[i] = caldModel.tableValueAsDouble( s_crustIoTblName, i, s_crustIoTblThicknessCol );
   }
   
   if ( std::abs( d[3] - m_initialThickness ) > s_eps ) 
   { 
      oss << "Initial crust thickness value in the model (" << d[3] << ") is differ from parameter value (" << m_initialThickness << ")\n";
   }

   if ( std::abs( t[2] - m_t0 ) > s_eps ) 
   {
      oss << "Start time for crust thinning value in the model (" << t[2] << ") is differ from parameter value (" << m_t0 << ")\n";
   }

   if ( std::abs( (t[2] - t[1]) - m_dt ) > s_eps )
   {
      oss << "Crust thinning duration value in the model (" << t[2]-t[1] << ") is differ from parameter value (" << m_dt << ")\n";
   }

   if ( std::abs( (d[0]/d[3] ) - m_coeff ) > s_eps )
   {
      oss << "Crust thinning factor value in the model (" << d[0]/d[3] << ") is differ from parameter value (" << m_coeff << ")\n";
   }

   if ( std::abs( d[0] - d[1] ) > s_eps || std::abs( d[3] - d[2] ) > s_eps )
   {
      oss << "Crust thinning profile in model does not match to single crust thinning event pattern. Crust thickness must decrease only between 2nd and 3d point\n";
   }

   return oss.str(); // another model, no reason to check further
}
  
// Get parameter value as an array of doubles
std::vector<double> PrmOneCrustThinningEvent::asDoubleArray() const
{
   std::vector<double> vals(4);

   vals[0] = m_initialThickness;
   vals[1] = m_t0;
   vals[2] = m_dt;
   vals[3] = m_coeff;

   return vals;
}

// Are two parameters equal?
bool PrmOneCrustThinningEvent::operator == ( const Parameter & prm ) const
{
   const PrmOneCrustThinningEvent * pp = dynamic_cast<const PrmOneCrustThinningEvent *>( &prm );
   if ( !pp ) return false;
   
   const double eps = 1.e-4;

   if ( !NumericFunctions::isEqual( m_initialThickness, pp->m_initialThickness, eps ) ) return false;
   if ( !NumericFunctions::isEqual( m_t0              , pp->m_t0              , eps ) ) return false;
   if ( !NumericFunctions::isEqual( m_dt              , pp->m_dt              , eps ) ) return false;
   if ( !NumericFunctions::isEqual( m_coeff           , pp->m_coeff           , eps ) ) return false;

   return true;
}


// Save all object data to the given stream, that object could be later reconstructed from saved data
bool PrmOneCrustThinningEvent::save( CasaSerializer & sz, unsigned int /* version */ ) const
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

   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
PrmOneCrustThinningEvent::PrmOneCrustThinningEvent( CasaDeserializer & dz, unsigned int /* objVer */ )
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

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "PrmOneCrustThinningEvent deserialization unknown error";
   }
}

}


