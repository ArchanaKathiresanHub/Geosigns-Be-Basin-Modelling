//                                                                      
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PrmLithoSTPThermalCond.C
/// @brief This file keeps API implementation for STP thermal conductivity coefficient parameter 

// CASA API
#include "PrmLithoSTPThermalCond.h"
#include "VarPrmLithoSTPThermalCond.h"

// CMB API
#include "cmbAPI.h"

// Utilities lib
#include "NumericFunctions.h"

// STL/C lib
#include <cassert>
#include <cmath>
#include <sstream>
#include <vector>

namespace casa
{

// Constructor
PrmLithoSTPThermalCond::PrmLithoSTPThermalCond( mbapi::Model & mdl, const char * lithoName )
   : m_parent( 0 )
   , m_lithoName(   lithoName )
   , m_value(       Utilities::Numerical::IbsNoDataValue )

{ 
   try
   {
      mbapi::LithologyManager & mgr = mdl.lithologyManager();

      bool isFound = false;

      // go over all lithologies and look for the first lithology with the same name as given
      const std::vector<mbapi::LithologyManager::LithologyID> & lIDs = mgr.lithologiesIDs();
      for ( size_t i = 0; i < lIDs.size() && !isFound; ++i )
      {
         std::string lName = mgr.lithologyName( lIDs[i] );
      
         if ( lName != m_lithoName ) { continue; }

         m_value  = mgr.stpThermalConductivityCoeff( lIDs[i] );
         if ( ErrorHandler::NoError != mgr.errorCode() ) { throw ErrorHandler::Exception( mgr.errorCode() ) << mgr.errorMessage(); }
         isFound = true;
      }
   
      if ( !isFound )
      {
         throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) <<  "Can't find lithology type with name " <<
            m_lithoName << " in lithologies type list";
      }
   }
   catch ( const ErrorHandler::Exception & e ) { mdl.reportError( e.errorCode(), e.what() ); }

   // construct parameter name
   std::ostringstream oss;
   oss << "LithoSTPThermalCond(" << m_lithoName << ")";
   m_name = oss.str();
}

 // Constructor
PrmLithoSTPThermalCond::PrmLithoSTPThermalCond( const VarPrmLithoSTPThermalCond * parent, const char * lithoName, double val )
   : m_parent(      parent )
   , m_lithoName(   lithoName )
   , m_value(       val )
{
  // construct parameter name
   std::ostringstream oss;
   oss << "LithoSTPThermalCond(" << m_lithoName << ")";
   m_name = oss.str();
}

// Update given model with the parameter value
ErrorHandler::ReturnCode PrmLithoSTPThermalCond::setInModel( mbapi::Model & caldModel, size_t /* caseID */ )
{
   try
   {
      mbapi::LithologyManager & mgr = caldModel.lithologyManager();

      bool found = false;
      // go over all lithologies to find the lithology type with given name and set up porosity model
      const std::vector<mbapi::LithologyManager::LithologyID> & ids = mgr.lithologiesIDs();
      for ( size_t i = 0; i < ids.size() && !found; ++i )
      {
         if ( mgr.lithologyName( ids[i] ) == m_lithoName )
         {
            if ( ErrorHandler::NoError != mgr.setSTPThermalConductivityCoeff( ids[i], m_value ) )
            {
               throw ErrorHandler::Exception( mgr.errorCode() ) << mgr.errorMessage();
            }
            found = true;;
         }
      }
      if ( !found ) throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Can not find lithology with name: " << m_lithoName;
   }
   catch ( const ErrorHandler::Exception & e ) { return caldModel.reportError( e.errorCode(), e.what() ); }

   return ErrorHandler::NoError;
}

// Validate all porosity model parameters
std::string PrmLithoSTPThermalCond::validate( mbapi::Model & caldModel )
{
   std::ostringstream oss;

   if ( m_value < 0   ) oss << "STP thermal conductivity coeff. for lithology " << m_lithoName << " can not be negative: "     << m_value << std::endl;
   if ( m_value > 20  ) oss << "STP thermal conductivity coeff. for lithology " << m_lithoName << " can not be more than 20: " << m_value << std::endl;

   PrmLithoSTPThermalCond prm( caldModel, m_lithoName.c_str() );

   if ( prm != *this )
   {
      oss << "STP thermal conductivity coeff. for lithology " << m_lithoName << " defined in project: " << prm.m_value << 
          " , is different from the parameter value: " << m_value << std::endl;
   }
   return oss.str();
}

// Get parameter value as an array of doubles
std::vector<double> PrmLithoSTPThermalCond::asDoubleArray() const
{
   std::vector<double> vals( 1, m_value );

   return vals;
}


// Are two parameters equal?
bool PrmLithoSTPThermalCond::operator == ( const Parameter & prm ) const
{
   const PrmLithoSTPThermalCond * pp = dynamic_cast<const PrmLithoSTPThermalCond *>( &prm );
   if ( !pp ) return false;
   
   const double eps = 1.e-6;

   if ( m_lithoName != pp->m_lithoName ) return false;
   if ( !NumericFunctions::isEqual( m_value, pp->m_value,  eps ) ) return false;

   return true;
}


// Save all object data to the given stream, that object could be later reconstructed from saved data
bool PrmLithoSTPThermalCond::save( CasaSerializer & sz, unsigned int /* version */ ) const
{
   bool hasParent = m_parent ? true : false;
   bool ok = sz.save( hasParent, "hasParent" );

   if ( hasParent )
   {
      CasaSerializer::ObjRefID parentID = sz.ptr2id( m_parent );
      ok = ok ? sz.save( parentID, "VarParameterID" ) : ok;
   }
   ok = ok ? sz.save( m_name,      "name"                   ) : ok;
   ok = ok ? sz.save( m_lithoName, "LithologyName"          ) : ok;
   ok = ok ? sz.save( m_value,     "STPThermoCondCoefValue" ) : ok;

   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
PrmLithoSTPThermalCond::PrmLithoSTPThermalCond( CasaDeserializer & dz, unsigned int /* objVer */ )
{
   CasaDeserializer::ObjRefID parentID;

   bool hasParent;
   bool ok = dz.load( hasParent, "hasParent" );

   if ( hasParent )
   {
      bool ok = dz.load( parentID, "VarParameterID" );
      m_parent = ok ? dz.id2ptr<VarParameter>( parentID ) : 0;
   }

   ok = ok ? dz.load( m_name,      "name"                   ) : ok;
   ok = ok ? dz.load( m_lithoName, "LithologyName"          ) : ok;
   ok = ok ? dz.load( m_value,     "STPThermoCondCoefValue" ) : ok;
   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "PrmLithoSTPThermalCond deserialization unknown error";
   }
}

} // namespace casa

