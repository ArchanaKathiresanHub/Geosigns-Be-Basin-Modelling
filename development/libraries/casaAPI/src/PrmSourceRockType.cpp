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
PrmSourceRockType::PrmSourceRockType( mbapi::Model & mdl, const std::string & layerName, int mixID )
   : m_parent(0)
   , m_layerName( layerName )
   , m_mixID( mixID )
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

      const std::vector<std::string> & srtNames = stMgr.sourceRockTypeName( lid );
      if ( srtNames.empty() )
      {
         throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Layer " << m_layerName <<
               " is set as a source rock layer but has no source rock lithology defined";
      }
      if ( mixID < 1 || mixID > srtNames.size() )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Layer " << m_layerName <<
            " has not source rock mixing enabled or invalid mixing ID:  " << mixID << " is given";
      }

      mbapi::SourceRockManager::SourceRockID sid = srMgr.findID( m_layerName, srtNames[mixID-1] );
      if ( IsValueUndefined( sid ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Can not find source rock lithology for layer "
                  << m_layerName << " and SR type " << (m_srtName.empty() ? srtNames[mixID-1] : m_srtName);
      }
      m_srtName = srtNames[mixID-1];
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
                                    , int                          mixID
                                    ) : m_parent( parent )
                                      , m_layerName( layerName )
                                      , m_mixID( mixID )
                                      , m_srtName( sourceRockTypeName )
{
   // construct parameter name
   std::ostringstream oss;
   oss << "SourceRockType(" << m_layerName << "," << m_mixID << "," << m_srtName << ")";
   m_name = oss.str();
}

// Update given model with the parameter value
ErrorHandler::ReturnCode PrmSourceRockType::setInModel( mbapi::Model & caldModel, size_t /* caseID */ )
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

      std::vector<std::string> srtNames = stMgr.sourceRockTypeName( lid );
      if ( srtNames.empty() || m_mixID < 1 || m_mixID > srtNames.size() )
      {
         throw ErrorHandler::Exception(ErrorHandler::UndefinedValue) << "Layer " << m_layerName <<
            " set as source rock layer but has no source rock lithology defined for the mixing ID: " << m_mixID;
      }

      srtNames[m_mixID-1] = m_srtName; 
      if ( ErrorHandler::NoError != stMgr.setSourceRockTypeName( lid, srtNames ) )
      {
         throw ErrorHandler::Exception( stMgr.errorCode() ) << stMgr.errorMessage();
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

      if ( srtNames[m_mixID - 1] != m_srtName )
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
   if ( m_mixID     != pp->m_mixID     ) return false;

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
   ok = ok ? sz.save( m_mixID,     "mixingID"  ) : ok;

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
   ok = ok ? dz.load( m_mixID,     "mixingID"  ) : ok;

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) << "PrmSourceRockType deserialization unknown error";
   }
}

} // namespace casa
