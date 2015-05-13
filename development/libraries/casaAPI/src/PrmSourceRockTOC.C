//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PrmSourceRockTOC.C
/// @brief This file keeps API implementation for Source Rock TOC parameter handling 

// CASA API
#include "PrmSourceRockTOC.h"
#include "VarPrmSourceRockTOC.h"

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
 PrmSourceRockTOC::PrmSourceRockTOC( mbapi::Model & mdl
                                   , const char * layerName
                                   ) : m_parent(0), m_layerName( layerName )
{    
   try
   {
      mbapi::SourceRockManager   & srMgr = mdl.sourceRockManager();
      mbapi::StratigraphyManager & stMgr = mdl.stratigraphyManager();

      // get check is this layer has a mix of source rocks
      mbapi::StratigraphyManager::LayerID lid = stMgr.layerID( m_layerName );
      if (stMgr.errorCode() != ErrorHandler::NoError) { throw ErrorHandler::Exception( stMgr.errorCode() ) << stMgr.errorMessage(); }

      // check if layer set as active source rock
      if ( !stMgr.isSourceRockActive( lid ) )
      {
         throw ErrorHandler::Exception(ErrorHandler::ValidationError) <<
            "TOC setting error: source rock is not active for the layer:" << m_layerName;
      }
   
      // in case of SR mixing report as unimplemented
      if ( stMgr.isSourceRockMixingEnabled( lid ) )
      {
         throw ErrorHandler::Exception(ErrorHandler::NotImplementedAPI) <<
            "Setting TOC parameter for the mixing of source rocks is not implemented yet";
      }

      const std::vector<std::string> & srtNames = stMgr.sourceRockTypeName( lid );
      if ( srtNames.empty() )
      {
         throw ErrorHandler::Exception(ErrorHandler::UndefinedValue) << "Layer " << m_layerName <<
            " set as source rock layer but has no source rock lithology defined";
      }

      mbapi::SourceRockManager::SourceRockID sid = srMgr.findID(m_layerName, srtNames[0]);
      if ( IsValueUndefined( sid ) )
      {
         throw ErrorHandler::Exception(ErrorHandler::UndefinedValue) << "Can not find source rock lithology for layer "
            << m_layerName << " and SR type " << srtNames[0];
      }

      // check if in base project TOC is defined as a map
      const std::string & mapName = srMgr.tocInitMapName(sid);
      if ( !mapName.empty() )
      {
         throw ErrorHandler::Exception(ErrorHandler::AlreadyDefined) <<
            "Source rock lithology for layer" << m_layerName << 
            " has TOC already defined as a map. This is unsupported yet";
      }

      m_toc = srMgr.tocIni( sid );
   }
   catch (const ErrorHandler::Exception & e) { mdl.reportError(e.errorCode(), e.what()); }

   // construct parameter name
   std::ostringstream oss;
   oss << "SourceRockTOC(" << m_layerName << ")";
   m_name = oss.str();
}

 // Constructor
PrmSourceRockTOC::PrmSourceRockTOC( const VarPrmSourceRockTOC * parent
                                  , double                      val
                                  , const char                * layerName
                                  )
                                  : m_parent( parent )
                                  , m_toc( val )
                                  , m_layerName( layerName )
{
   // construct parameter name
   std::ostringstream oss;
   oss << "SourceRockTOC(" << m_layerName << ")";
   m_name = oss.str();
}

// Destructor
PrmSourceRockTOC::~PrmSourceRockTOC() {;}


// Update given model with the parameter value
ErrorHandler::ReturnCode PrmSourceRockTOC::setInModel( mbapi::Model & caldModel )
{
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

      // in case of SR mixing report as unimplemented
      if ( stMgr.isSourceRockMixingEnabled( lid ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::NotImplementedAPI ) << 
            "Setting TOC parameter for the mixing of source rocks is not implemented yet";
      }

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

      // check if in base project TOC is defined as a map
      const std::string & mapName = srMgr.tocInitMapName( sid );
      if ( !mapName.empty() )
      {
         throw ErrorHandler::Exception( ErrorHandler::AlreadyDefined ) << "Source rock lithology for layer" <<
            m_layerName << " has TOC already defined as a map. This is unsupported yet";
      }

      if ( ErrorHandler::NoError != srMgr.setTOCIni( sid, m_toc ) )
      {
         throw ErrorHandler::Exception( srMgr.errorCode() ) << srMgr.errorMessage();
      }
   }
   catch ( const ErrorHandler::Exception & e ) { return caldModel.reportError( e.errorCode(), e.what() ); }

   return ErrorHandler::NoError;
}

// Validate TOC value if it is in [0:100] range
std::string PrmSourceRockTOC::validate( mbapi::Model & caldModel )
{
   std::ostringstream oss;

   if (      m_toc < 0   ) oss << "TOC value for the layer " << m_layerName << ", can not be negative: " << m_toc << std::endl;
   else if ( m_toc > 100 ) oss << "TOC value for the layer " << m_layerName << ", can not be more than 100%: " << m_toc << std::endl;

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

      double tocInModel = UndefinedDoubleValue;

      // in case of SR mixing get HI from the mix
      if ( stMgr.isSourceRockMixingEnabled( lid ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) <<
               "Can not get TOC parameter for the mixing of source rocks for the layer " << m_layerName << ", not implemented yet";
      }
      else // otherwise go to source rock lithology table for the source rock hi
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
         tocInModel = srMgr.tocIni( sid );
      }

      if ( !NumericFunctions::isEqual( tocInModel, m_toc, 1.e-4 ) )
      {
         oss << "Value of TOC in the model (" << tocInModel << ") is different from the parameter value (" << m_toc << ")" << std::endl;
      }
   }
   catch ( const ErrorHandler::Exception & e ) { oss << e.what() << std::endl; }

   return oss.str();
}


// Are two parameters equal?
bool PrmSourceRockTOC::operator == ( const Parameter & prm ) const
{
   const PrmSourceRockTOC * pp = dynamic_cast<const PrmSourceRockTOC *>( &prm );
   if ( !pp ) return false;
   
   const double eps = 1.e-6;

   if ( m_layerName != pp->m_layerName ) return false;

   if ( !NumericFunctions::isEqual( m_toc, pp->m_toc, eps ) ) return false;

   return true;
}


// Save all object data to the given stream, that object could be later reconstructed from saved data
bool PrmSourceRockTOC::save( CasaSerializer & sz, unsigned int version ) const
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
   ok = ok ? sz.save( m_toc,       "toc"       ) : ok;

   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
PrmSourceRockTOC::PrmSourceRockTOC( CasaDeserializer & dz, unsigned int objVer )
{
   CasaDeserializer::ObjRefID parentID;

   bool hasParent;
   bool ok = dz.load( hasParent, "hasParent" );

   if ( hasParent )
   {
      bool ok = dz.load( parentID, "VarParameterID" );
      m_parent = ok ? dz.id2ptr<VarParameter>( parentID ) : 0;
   }

   ok = ok ? dz.load( m_name, "name" ) : ok;
   ok = ok ? dz.load( m_layerName, "layerName" ) : ok;
   ok = ok ? dz.load( m_toc, "toc" ) : ok;

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "PrmSourceRockTOC deserialization unknown error";
   }
}

} // namespace casa
