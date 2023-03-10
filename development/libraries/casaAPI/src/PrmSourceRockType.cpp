//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file PrmSourceRockType.cpp
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
   : Parameter( )
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
      if ( mixID < 1 || static_cast<size_t>( mixID ) > srtNames.size() )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Layer " << m_layerName <<
            " has not source rock mixing enabled or invalid mixing ID:  " << mixID << " is given";
      }

      mbapi::SourceRockManager::SourceRockID sid = srMgr.findID( m_layerName, srtNames[mixID-1] );
      if ( Utilities::isValueUndefined( sid ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Can not find source rock lithology for layer "
                  << m_layerName << " and SR type " << (m_srtName.empty() ? srtNames[mixID-1] : m_srtName);
      }
      m_srtName = srtNames[mixID-1];
   }
   catch ( const ErrorHandler::Exception & e)
   {
      mdl.reportError( e.errorCode(), e.what() );
   }
}

 // Constructor
PrmSourceRockType::PrmSourceRockType( const VarParameter * parent
                                    , const std::string  & layerName
                                    , const std::string  & sourceRockTypeName
                                    , int                  mixID
                                    )
                                    : Parameter( parent )
                                    , m_layerName( layerName )
                                    , m_srtName( sourceRockTypeName )
                                    , m_mixID( mixID )
{
}

// Update given model with the parameter value
ErrorHandler::ReturnCode PrmSourceRockType::setInModel( mbapi::Model & caldModel, size_t /* caseID */ )
{
   try
   {
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
      if ( srtNames.empty() || m_mixID < 1 || static_cast<size_t>( m_mixID ) > srtNames.size() )
      {
         throw ErrorHandler::Exception(ErrorHandler::UndefinedValue) << "Layer " << m_layerName <<
            " set as source rock layer but has no source rock lithology defined for the mixing ID: " << m_mixID;
      }

      if ( srtNames.size() > 1 ) // source rock mixint not allowed for categorical source rock parameter
      {
         if ( srtNames[m_mixID-1] != m_srtName ) // for the base case do not change mixing!!
         {
            srtNames[ m_mixID == 1 ? 1 : 0 ] = "";          // remove other source rock
            stMgr.setSourceRockMixHI( lid, 0.0 );           // set HI of the mix to 0.0
            stMgr.setSourceRockMixHIMapName( lid, "" );     // delete HI map

            srtNames[m_mixID-1] = m_srtName;                // change source rock type
         }
      }
      else { srtNames[m_mixID-1] = m_srtName; }

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
      if ( Utilities::isValueUndefined( sid ) )
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
   if ( !parent() ) return -1; // no parent - no enumeration

   const VarPrmSourceRockType * prnt = dynamic_cast<const VarPrmSourceRockType*>( parent() );
   if ( !prnt ) return -1;

   return prnt->index( this );
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
bool PrmSourceRockType::save( CasaSerializer & sz ) const
{
   bool ok = saveCommonPart(sz);
   ok = ok ? sz.save( m_layerName, "layerName" ) : ok;
   ok = ok ? sz.save( m_srtName,   "srtName"   ) : ok;
   ok = ok ? sz.save( m_mixID,     "mixingID"  ) : ok;

   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
PrmSourceRockType::PrmSourceRockType( CasaDeserializer & dz, unsigned int objVer ) :
  Parameter(dz, objVer)
{
   bool ok = true;
   if ( objVer < 1 )
   {
     std::string name;
     ok = ok && dz.load( name, "name" );
   }
   ok = ok ? dz.load( m_layerName, "layerName" ) : ok;
   ok = ok ? dz.load( m_srtName,   "srtName"   ) : ok;
   ok = ok ? dz.load( m_mixID,     "mixingID"  ) : ok;

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) << "PrmSourceRockType deserialization unknown error";
   }
}

} // namespace casa
