//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PrmSourceRockPreAsphaltStartAct.C
/// @brief This file keeps API implementation for Source Rock Preasphaltene Activation Energy parameter handling 

// CASA API
#include "PrmSourceRockPreAsphaltStartAct.h"
#include "VarPrmSourceRockPreAsphaltStartAct.h"

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
 PrmSourceRockPreAsphaltStartAct::PrmSourceRockPreAsphaltStartAct( mbapi::Model & mdl
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
            "Preasphalt act. energy setting error: source rock is not active for the layer:" << m_layerName;
      }
   
      // in case of SR mixing report as unimplemented
      if ( stMgr.isSourceRockMixingEnabled( lid ) )
      {
         throw ErrorHandler::Exception(ErrorHandler::NotImplementedAPI) <<
            "Setting Preasphalt act. energy parameter for the mixing of source rocks is not implemented yet";
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

      m_value = srMgr.preAsphActEnergy( sid );
   }
   catch (const ErrorHandler::Exception & e) { mdl.reportError(e.errorCode(), e.what()); }

   // construct parameter name
   std::ostringstream oss;
   oss << "SourceRockPreAsphaltStartAct(" << m_layerName << ")";
   m_name = oss.str();
}

 // Constructor
PrmSourceRockPreAsphaltStartAct::PrmSourceRockPreAsphaltStartAct( const VarPrmSourceRockPreAsphaltStartAct * parent
                                  , double                      val
                                  , const char                * layerName
                                  )
                                  : m_parent( parent )
                                  , m_value( val )
                                  , m_layerName( layerName )
{
   // construct parameter name
   std::ostringstream oss;
   oss << "SourceRockPreAsphaltStartAct(" << m_layerName << ")";
   m_name = oss.str();
}

// Destructor
PrmSourceRockPreAsphaltStartAct::~PrmSourceRockPreAsphaltStartAct() {;}


// Update given model with the parameter value
ErrorHandler::ReturnCode PrmSourceRockPreAsphaltStartAct::setInModel( mbapi::Model & caldModel, size_t /* caseID */ )
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
            "Preasphalt act. energy setting error: source rock is not active for the layer:" << m_layerName;
      }

      // in case of SR mixing report as unimplemented
      if ( stMgr.isSourceRockMixingEnabled( lid ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::NotImplementedAPI ) << 
            "Setting preasphalt act. energy parameter for the mixing of source rocks is not implemented yet";
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

      if ( ErrorHandler::NoError != srMgr.setPreAsphActEnergy( sid, m_value ) )
      {
         throw ErrorHandler::Exception( srMgr.errorCode() ) << srMgr.errorMessage();
      }
   }
   catch ( const ErrorHandler::Exception & e ) { return caldModel.reportError( e.errorCode(), e.what() ); }

   return ErrorHandler::NoError;
}

// Validate parameter value if it is in valid range
std::string PrmSourceRockPreAsphaltStartAct::validate( mbapi::Model & caldModel )
{
   std::ostringstream oss;

   if (      m_value < 100 ) oss << "Preasphalt act. energy value for the layer " << m_layerName << ", can not be less then 100.0 kJ/mole: " << m_value << std::endl;
   else if ( m_value > 300 ) oss << "Preasphalt act. energy value for the layer " << m_layerName << ", can not be more than 300.0 kJ/mole: " << m_value << std::endl;

   mbapi::SourceRockManager & mgr = caldModel.sourceRockManager();

   bool layerFound = false;

   // go over all source rock lithologies and check do we have TOC map set for the layer with the same name
   const std::vector<mbapi::SourceRockManager::SourceRockID> & srIDs = mgr.sourceRockIDs();
   for ( size_t i = 0; i < srIDs.size(); ++i )
   {
      if ( mgr.layerName( srIDs[i] ) == m_layerName )
      {
         layerFound = true;
         
         double mdlVal = mgr.preAsphActEnergy( srIDs[i] );
         if ( !NumericFunctions::isEqual( mdlVal, m_value, 1.e-4 ) )
         {
            oss << "Value of Preasphalt act. energy in the model (" << mdlVal << ") is different from the parameter value (" << m_value << ")" << std::endl;
         }
      }
   }

   if ( !layerFound ) oss << "There is no such layer in the model: " << m_layerName << std::endl;

   return oss.str();
}


// Are two parameters equal?
bool PrmSourceRockPreAsphaltStartAct::operator == ( const Parameter & prm ) const
{
   const PrmSourceRockPreAsphaltStartAct * pp = dynamic_cast<const PrmSourceRockPreAsphaltStartAct *>( &prm );
   if ( !pp ) return false;
   
   const double eps = 1.e-6;

   if ( m_layerName != pp->m_layerName ) return false;

   if ( !NumericFunctions::isEqual( m_value, pp->m_value, eps ) ) return false;

   return true;
}


// Save all object data to the given stream, that object could be later reconstructed from saved data
bool PrmSourceRockPreAsphaltStartAct::save( CasaSerializer & sz, unsigned int version ) const
{
   bool hasParent = m_parent ? true : false;
   bool ok = sz.save( hasParent, "hasParent" );

   if ( hasParent )
   {
      CasaSerializer::ObjRefID parentID = sz.ptr2id( m_parent );
      ok = ok ? sz.save( parentID, "VarParameterID" ) : ok;
   }
   ok = ok ? sz.save( m_name,      "name"             ) : ok;
   ok = ok ? sz.save( m_layerName, "layerName"        ) : ok;
   ok = ok ? sz.save( m_value,     "PreasphActEnergy" ) : ok;

   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
PrmSourceRockPreAsphaltStartAct::PrmSourceRockPreAsphaltStartAct( CasaDeserializer & dz, unsigned int objVer )
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
   ok = ok ? dz.load( m_value,     "PreasphActEnergy" ) : ok;

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "PrmSourceRockPreAsphaltStartAct deserialization unknown error";
   }
}

} // namespace casa
