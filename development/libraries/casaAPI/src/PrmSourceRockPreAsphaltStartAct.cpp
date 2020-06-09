//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file PrmSourceRockPreAsphaltStartAct.cpp
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
                                                                , const char * lrName
                                                                , const char * srType
                                                                , int          mixingID
                                                                ) : PrmSourceRockProp( mdl, lrName, srType, mixingID )
{
   // PreAsphaltStartAct specific part
   m_propName = "PreAsphaltStartAct";

   mbapi::SourceRockManager   & srMgr = mdl.sourceRockManager();
   mbapi::StratigraphyManager & stMgr = mdl.stratigraphyManager();

   // get this layer has a mix of source rocks (all checking were done in parent constructor
   const std::vector<std::string> & srtNames = stMgr.sourceRockTypeName( stMgr.layerID( m_layerName ) );
   mbapi::SourceRockManager::SourceRockID sid = srMgr.findID( m_layerName, (!m_srTypeName.empty() ? m_srTypeName : srtNames[m_mixID-1]) );

   m_val = srMgr.preAsphActEnergy( sid );
}

 // Constructor
PrmSourceRockPreAsphaltStartAct::PrmSourceRockPreAsphaltStartAct( const VarPrmSourceRockPreAsphaltStartAct * parent
                                  , double                      val
                                  , const char                * lrName
                                  , const char                * srType
                                  , int                         mixingID
                                  ) : PrmSourceRockProp( parent, val, lrName, srType, mixingID )
{
   m_propName = "PreAsphaltStartAct";
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

      const std::vector<std::string> & srtNames = stMgr.sourceRockTypeName( lid );
      if ( srtNames.empty() || m_mixID < 1 || static_cast<size_t>( m_mixID ) > srtNames.size() )
      {
         throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Layer " << m_layerName <<
            " set as source rock layer but has no source rock lithology defined for the mixing ID: " << m_mixID;
      }

      mbapi::SourceRockManager::SourceRockID sid = srMgr.findID( m_layerName, (!m_srTypeName.empty() ? m_srTypeName : srtNames[m_mixID-1]) );
      if ( IsValueUndefined( sid ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Can not find source rock lithology for layer "
            << m_layerName << " and SR type " << (!m_srTypeName.empty() ? m_srTypeName : srtNames[m_mixID-1]);
      }

      if ( ErrorHandler::NoError != srMgr.setPreAsphActEnergy( sid, m_val ) )
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

   if (      m_val < 100 ) oss << "Preasphalt act. energy value for the layer " << m_layerName << ", can't be less then 100.0 kJ/mole: " << m_val << std::endl;
   else if ( m_val > 300 ) oss << "Preasphalt act. energy value for the layer " << m_layerName << ", can't be more than 300.0 kJ/mole: " << m_val << std::endl;

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

      // go to source rock lithology table for the source rock  PreAsphaltStartAct
      const std::vector<std::string> & srtNames = stMgr.sourceRockTypeName( lid );

      if ( srtNames.empty() )
      {
         throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Layer " << m_layerName <<
            " set as source rock layer but has no source rock lithology defined";
      }

      if ( srtNames.size() < static_cast<size_t>( m_mixID ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::ValidationError ) <<
            "Layer " << m_layerName << " has no source rock definition for the mixing ID: " << m_mixID;
      }

      if ( !m_srTypeName.empty() && srtNames[m_mixID - 1] != m_srTypeName )
      {
         throw ErrorHandler::Exception( ErrorHandler::ValidationError ) <<
            "Layer " << m_layerName << " has unmatched source rock type name :" << srtNames[m_mixID-1] <<
            " for the mixing ID " << m_mixID << " to source rock type defined for the PreAsphaltStartAct parameter: " << m_srTypeName;
      }

      mbapi::SourceRockManager::SourceRockID sid = srMgr.findID( m_layerName, (!m_srTypeName.empty() ? m_srTypeName : srtNames[m_mixID-1]) );
      if ( IsValueUndefined( sid ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Can not find source rock lithology for layer "
            << m_layerName << " and SR type " << (!m_srTypeName.empty() ? m_srTypeName : srtNames[m_mixID-1]);
      }

      double mdlVal = srMgr.preAsphActEnergy( sid );
      if ( !NumericFunctions::isEqual( mdlVal, m_val, 1.e-4 ) )
      {
         oss << "Value of Preasphalt act. energy in the model (" << mdlVal << ") is different from the parameter value (" <<
            m_val << ")" << std::endl;
      }
   }
   catch ( const ErrorHandler::Exception & e ) { oss << e.what() << std::endl; }

   return oss.str();
}


// Are two parameters equal?
bool PrmSourceRockPreAsphaltStartAct::operator == ( const Parameter & prm ) const
{
   const PrmSourceRockPreAsphaltStartAct * pp = dynamic_cast<const PrmSourceRockPreAsphaltStartAct *>( &prm );
   if ( !pp ) return false;

   const double eps = 1.e-6;

   if ( m_layerName  != pp->m_layerName  ) return false;
   if ( m_mixID      != pp->m_mixID      ) return false;
   if ( m_srTypeName != pp->m_srTypeName ) return false;

   if ( !NumericFunctions::isEqual( m_val, pp->m_val, eps ) ) return false;

   return true;
}


// Save all object data to the given stream, that object could be later reconstructed from saved data
bool PrmSourceRockPreAsphaltStartAct::save( CasaSerializer & sz ) const
{
   bool ok = PrmSourceRockProp::serializeCommonPart( sz );

   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
PrmSourceRockPreAsphaltStartAct::PrmSourceRockPreAsphaltStartAct( CasaDeserializer & dz, unsigned int objVer )
{
   bool ok = PrmSourceRockProp::deserializeCommonPart( dz, objVer );

   if ( m_propName.empty() ) { m_propName = "PreAsphaltStartAct"; }

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "PrmSourceRockPreAsphaltStartAct deserialization unknown error";
   }
}

} // namespace casa
