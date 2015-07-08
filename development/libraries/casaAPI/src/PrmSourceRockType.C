//                                                                      
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PrmSourceRockType.C
/// @brief This file keeps API implementation for Source Rock Type categorical parameter

// CASA API
#include "PrmSourceRockType.h"

// CMB API
#include "cmbAPI.h"

// STL/C lib
#include <cmath>
#include <sstream>
#include <vector>

namespace casa
{

// Constructor
PrmSourceRockType::PrmSourceRockType( mbapi::Model & mdl, const std::string & layerName ) : m_parent(0), m_layerName( layerName )
{ 
   try
   {
      mbapi::SourceRockManager   & srMgr = mdl.sourceRockManager();
      mbapi::StratigraphyManager & stMgr = mdl.stratigraphyManager();

      // get layer id
      mbapi::StratigraphyManager::LayerID lid = stMgr.layerID( m_layerName );
      if ( stMgr.errorCode() != ErrorHandler::NoError ) { throw ErrorHandler::Exception( stMgr.errorCode() ) << stMgr.errorMessage(); }

      // check if layer set as active source rock
      if ( !stMgr.isSourceRockActive( lid ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::ValidationError ) <<
            "Source Rock Type setting error: source rock is not active for the layer:" << m_layerName;
      }

      // in case of SR mixing throw exception as we can't handle it now
      if ( stMgr.isSourceRockMixingEnabled( lid ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::NotImplementedAPI ) <<
               "Can not variate source rock type for the layer "<< m_layerName << " which has source rock types mixing enabled";
      }
      else // otherwise go to source rock lithology table for the source rock type ID
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

         m_srtName = srtNames[0];
      }
   }
   catch ( const ErrorHandler::Exception & e) { mdl.reportError( e.errorCode(), e.what() ); }

   // construct parameter name
   std::ostringstream oss;
   oss << "SourceRockType(" << m_layerName << "," << m_srtName << ")";
   m_name = oss.str();
}

 // Constructor
PrmSourceRockType::PrmSourceRockType( const VarPrmSourceRockType * parent
                                    , const std::string          & layerName
                                    , const std::string          & sourceRockTypeName
                                    ) : m_parent( parent )
                                      , m_layerName( layerName )
                                      , m_srtName( sourceRockTypeName )
{
   // construct parameter name
   std::ostringstream oss;
   oss << "SourceRockType(" << m_layerName << "," << m_srtName << ")";
   m_name = oss.str();
}

// Update given model with the parameter value
ErrorHandler::ReturnCode PrmSourceRockType::setInModel( mbapi::Model & caldModel )
{
   try
   {
      mbapi::SourceRockManager   & srMgr = caldModel.sourceRockManager();
      mbapi::StratigraphyManager & stMgr = caldModel.stratigraphyManager();

      // get check is this layer has a mix of source rocks
      mbapi::StratigraphyManager::LayerID lid = stMgr.layerID( m_layerName );
      if ( stMgr.errorCode() != ErrorHandler::NoError ) { throw ErrorHandler::Exception( stMgr.errorCode() ) << stMgr.errorMessage(); }

      // check if this layer has source rock active
      if ( !stMgr.isSourceRockActive( lid ) )
      { 
         throw ErrorHandler::Exception( ErrorHandler::ValidationError ) <<
            "HI setting error: source rock is not active for the layer:" << m_layerName;
      }

      // check if mixing enabled - throw error as it unsupported yet 
      if ( stMgr.isSourceRockMixingEnabled( lid ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::NotImplementedAPI ) << 
            "Changing of source rock type for stratigraphy layer with source rock mis is not implemented yet";
      }
      else
      {
         std::vector<std::string> srtNames;
         srtNames.push_back( m_srtName );
         if ( ErrorHandler::NoError != stMgr.setSourceRockTypeName( lid, srtNames ) )
         {
            throw ErrorHandler::Exception( stMgr.errorCode() ) << stMgr.errorMessage();
         }
      }      
   }
   catch ( const ErrorHandler::Exception & e ) { return caldModel.reportError( e.errorCode(), e.what() ); }

   return ErrorHandler::NoError;
}

// Validate parameter value if it exist in source rock lithology table and if stratigraphy layer has this source rock type 
std::string PrmSourceRockType::validate( mbapi::Model & caldModel )
{
   std::ostringstream oss;

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
            "Source rock type setting error: source rock is not active for the layer:" << m_layerName;
      }

      const std::vector<std::string> & srtNames = stMgr.sourceRockTypeName( lid );
      if ( srtNames.empty() )
      {
         throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Layer " << m_layerName <<
               " set as source rock layer but has no source rock lithology defined";
      }

      if ( srtNames[0] != m_srtName )
      {
         oss << "Source rock type in the model (" << srtNames[0] << ") is different from the parameter value (" << m_srtName << ")" << std::endl;
      }

      mbapi::SourceRockManager::SourceRockID sid = srMgr.findID( m_layerName, m_srtName );
      if ( IsValueUndefined( sid ) )
      {
         oss << "Source rock lithology table does not contains layer/source rock type combination for " << m_layerName << "/" << m_srtName;
      }
   }
   catch ( const ErrorHandler::Exception & e ) { oss << e.what() << std::endl; }

   return oss.str();
}

// Get parameter value as integer
int PrmSourceRockType::asInteger() const
{
   if ( !m_parent ) return -1; // no parent - no enumeration

   return m_parent->index( this );
}


// Are two parameters equal?
bool PrmSourceRockType::operator == ( const Parameter & prm ) const
{
   const PrmSourceRockType * pp = dynamic_cast<const PrmSourceRockType *>( &prm );
   if ( !pp ) return false;
   
   if ( m_layerName != pp->m_layerName ) return false;
   if ( m_srtName   != pp->m_srtName   ) return false;

   return true;
}


// Save all object data to the given stream, that object could be later reconstructed from saved data
bool PrmSourceRockType::save( CasaSerializer & sz, unsigned int version ) const
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
   ok = ok ? sz.save( m_srtName,   "srtName"   ) : ok;

   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
PrmSourceRockType::PrmSourceRockType( CasaDeserializer & dz, unsigned int objVer )
{
   CasaDeserializer::ObjRefID parentID;

   bool hasParent;
   bool ok = dz.load( hasParent, "hasParent" );

   if ( hasParent )
   {
      bool ok = dz.load( parentID, "VarParameterID" );
      m_parent = static_cast<const VarPrmSourceRockType*>( ok ? dz.id2ptr<VarParameter>( parentID ) : 0 );
   }

   ok = ok ? dz.load( m_name,      "name"      ) : ok;
   ok = ok ? dz.load( m_layerName, "layerName" ) : ok;
   ok = ok ? dz.load( m_srtName,   "srtName"   ) : ok;

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "PrmSourceRockType deserialization unknown error";
   }
}

} // namespace casa
