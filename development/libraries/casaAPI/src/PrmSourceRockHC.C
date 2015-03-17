//                                                                      
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PrmSourceRockHC.C
/// @brief This file keeps API implementation for Source Rock H/C parameter handling 

// CASA API
#include "PrmSourceRockHC.h"
#include "VarPrmSourceRockHC.h"

// CMB API
#include "cmbAPI.h"

// Utilities lib
#include <NumericFunctions.h>

// STL/C lib
#include <cassert>
#include <cmath>
#include <sstream>
#include <vector>

namespace casa
{

// Constructor
PrmSourceRockHC::PrmSourceRockHC( mbapi::Model & mdl, const char * layerName ) : m_parent(0), m_layerName( layerName )
{ 
   try
   {
      mbapi::SourceRockManager   & srMgr = mdl.sourceRockManager();
      mbapi::StratigraphyManager & stMgr = mdl.stratigraphyManager();

      // get check is this layer has a mix of source rocks
      mbapi::StratigraphyManager::LayerID lid = stMgr.layerID( m_layerName );
      if ( stMgr.errorCode() != ErrorHandler::NoError ) { throw ErrorHandler::Exception( stMgr.errorCode() ) << stMgr.errorMessage(); }

      // check if layer set as active source rock
      if ( !stMgr.isSourceRockActive( lid ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::ValidationError ) <<
            "TOC setting error: source rock is not active for the layer:" << m_layerName;
      }

      // in case of SR mixing get H/C from the HI mix value
      if ( stMgr.isSourceRockMixingEnabled( lid ) )
      {
         m_hc = stMgr.sourceRockMixHC( lid );
         if ( stMgr.errorCode() != ErrorHandler::NoError )
         {
            throw ErrorHandler::Exception(ErrorHandler::NonexistingID) <<
               "Can not get HI parameter for the mixing of source rocks for the layer " << m_layerName;
         }
      }
      else // otherwise go to source rock lithology table for the source rock hi
      {
         const std::vector<std::string> & srtNames = stMgr.sourceRockTypeName( lid );
         if ( srtNames.empty() )
         {
            throw ErrorHandler::Exception(ErrorHandler::UndefinedValue) << "Layer " << m_layerName <<
               " set as source rock layer but has no source rock lithology defined";
         }

         mbapi::SourceRockManager::SourceRockID sid = srMgr.findID( m_layerName, srtNames[0] );
         if ( IsValueUndefined( sid ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Can not find source rock lithology for layer "
               << m_layerName << " and SR type " << srtNames[0];
         }
         m_hc = srMgr.hcIni( sid );
      }
   }
   catch (const ErrorHandler::Exception & e) { mdl.reportError(e.errorCode(), e.what()); }

   // construct parameter name
   std::ostringstream oss;
   oss << "SourceRockHC(" << m_layerName << ")";
   m_name = oss.str();
}

 // Constructor
PrmSourceRockHC::PrmSourceRockHC( const VarPrmSourceRockHC * parent, double val, const char * layerName ) :
     m_parent( parent )
   , m_hc( val )
   , m_layerName( layerName )
{
   // construct parameter name
   std::ostringstream oss;
   oss << "SourceRockHC(" << m_layerName << ")";
   m_name = oss.str();
}

// Destructor
PrmSourceRockHC::~PrmSourceRockHC() {;}


// Update given model with the parameter value
ErrorHandler::ReturnCode PrmSourceRockHC::setInModel( mbapi::Model & caldModel )
{
   try
   {
      mbapi::SourceRockManager   & srMgr = caldModel.sourceRockManager();
      mbapi::StratigraphyManager & stMgr = caldModel.stratigraphyManager();

      // get check is this layer has a mix of source rocks
      mbapi::StratigraphyManager::LayerID lid = stMgr.layerID( m_layerName );
      if ( stMgr.errorCode() != ErrorHandler::NoError ) { throw ErrorHandler::Exception( stMgr.errorCode() ) << stMgr.errorMessage(); }

      // check how many source rock types for this level is defined
      if ( !stMgr.isSourceRockActive( lid ) )
      { 
         throw ErrorHandler::Exception( ErrorHandler::ValidationError ) <<
            "HI setting error: source rock is not active for the layer:" << m_layerName;
      }

      // in case of SR mixing, HI is set in stratigraphy table
      if ( stMgr.isSourceRockMixingEnabled( lid ) )
      {
         if ( ErrorHandler::NoError != stMgr.setSourceRockMixHC( lid, m_hc ) )
         {
            throw ErrorHandler::Exception( stMgr.errorCode() ) << stMgr.errorMessage();
         }
      }
      else
      {
         const std::vector<std::string> & srtNames = stMgr.sourceRockTypeName( lid );
         if ( srtNames.empty() )
         { 
            throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Layer " << m_layerName <<
               " set as source rock layer but has no source rock lithology defined";
         }
         mbapi::SourceRockManager::SourceRockID sid = srMgr.findID( m_layerName, srtNames[0] );
         if ( IsValueUndefined( sid ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Can not find source rock lithology for layer "
               << m_layerName << " and SR type " << srtNames[0];
         }
         if ( ErrorHandler::NoError != srMgr.setHCIni( sid, m_hc ) )
         {
            throw ErrorHandler::Exception( srMgr.errorCode() ) << srMgr.errorMessage();
         }
      }      
   }
   catch ( const ErrorHandler::Exception & e ) { return caldModel.reportError( e.errorCode(), e.what() ); }

   return ErrorHandler::NoError;
}

// Validate H/C value if it is in [0:2] range
std::string PrmSourceRockHC::validate( mbapi::Model & caldModel )
{
   std::ostringstream oss;

   if (      m_hc < 0 ) oss << "H/C value for the layer " << m_layerName << ", can not be negative: " << m_hc << std::endl;
   else if ( m_hc > 2 ) oss << "H/C value for the layer " << m_layerName << ", can not be more than 2 kg/tonne C: " << m_hc << std::endl;

   try
   {
      mbapi::SourceRockManager   & srMgr = caldModel.sourceRockManager();
      mbapi::StratigraphyManager & stMgr = caldModel.stratigraphyManager();

      // get check is this layer has a mix of source rocks
      mbapi::StratigraphyManager::LayerID lid = stMgr.layerID( m_layerName );
      if ( stMgr.errorCode() != ErrorHandler::NoError ) { throw ErrorHandler::Exception( stMgr.errorCode() ) << stMgr.errorMessage(); }

      // check if layer set as active source rock
      if ( !stMgr.isSourceRockActive( lid ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::ValidationError ) <<
            "TOC setting error: source rock is not active for the layer:" << m_layerName;
      }

      double hcInModel = UndefinedDoubleValue;

      // in case of SR mixing get HI from the mix
      if ( stMgr.isSourceRockMixingEnabled( lid ) )
      {
         hcInModel = stMgr.sourceRockMixHC( lid );
         if ( stMgr.errorCode() != ErrorHandler::NoError )
         {
            throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) <<
               "Can not get H/C parameter for the mixing of source rocks for the layer " << m_layerName;
         }
      }
      else // otherwise go to source rock lithology table for the source rock H/C
      {
         const std::vector<std::string> & srtNames = stMgr.sourceRockTypeName( lid );
         if ( srtNames.empty() )
         {
            throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Layer " << m_layerName <<
               " set as source rock layer but has no source rock lithology defined";
         }

         mbapi::SourceRockManager::SourceRockID sid = srMgr.findID( m_layerName, srtNames[0] );
         if ( IsValueUndefined( sid ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Can not find source rock lithology for layer "
               << m_layerName << " and SR type " << srtNames[0];
         }
         hcInModel = srMgr.hcIni( sid );
      }

      if ( !NumericFunctions::isEqual( hcInModel, m_hc, 1.e-4 ) )
      {
         oss << "Value of H/C in the model (" << hcInModel << ") is different from the parameter value (" << m_hc << ")" << std::endl;
      }
   }
   catch ( const ErrorHandler::Exception & e ) { oss << e.what() << std::endl; }

   return oss.str();
}


// Are two parameters equal?
bool PrmSourceRockHC::operator == ( const Parameter & prm ) const
{
   const PrmSourceRockHC * pp = dynamic_cast<const PrmSourceRockHC *>( &prm );
   if ( !pp ) return false;
   
   const double eps = 1.e-6;

   if ( m_layerName != pp->m_layerName ) return false;

   if ( !NumericFunctions::isEqual( m_hc, pp->m_hc, eps ) ) return false;

   return true;
}


// Save all object data to the given stream, that object could be later reconstructed from saved data
bool PrmSourceRockHC::save( CasaSerializer & sz, unsigned int version ) const
{
   bool hasParent = m_parent ? true : false;
   bool ok = sz.save( hasParent, "hasParent" );

   if ( hasParent )
   {
      CasaSerializer::ObjRefID parentID = sz.ptr2id( m_parent );
      ok = ok ? sz.save( parentID, "VarParameterID" ) : ok;
   }
   ok = ok ? sz.save( m_name,      "name"      ) : ok;
   ok = ok ? sz.save( m_layerName, "layerName" ) : ok;
   ok = ok ? sz.save( m_hc,        "HC"       ) : ok;

   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
PrmSourceRockHC::PrmSourceRockHC( CasaDeserializer & dz, unsigned int objVer )
{
   CasaDeserializer::ObjRefID parentID;

   bool hasParent;
   bool ok = dz.load( hasParent, "hasParent" );

   if ( hasParent )
   {
      bool ok = dz.load( parentID, "VarParameterID" );
      m_parent = ok ? dz.id2ptr<VarParameter>( parentID ) : 0;
   }

   ok = ok ? dz.load( m_name,      "name" ) : ok;
   ok = ok ? dz.load( m_layerName, "layerName" ) : ok;
   ok = ok ? dz.load( m_hc,        "HC" ) : ok;

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "PrmSourceRockHC deserialization unknown error";
   }
}

} // namespace casa
