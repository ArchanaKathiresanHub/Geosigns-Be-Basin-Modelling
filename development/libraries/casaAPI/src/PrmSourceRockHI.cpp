//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PrmSourceRockHI.C
/// @brief This file keeps API implementation for Source Rock HI parameter handling 

// CASA API
#include "PrmSourceRockHI.h"
#include "VarPrmSourceRockHI.h"

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
PrmSourceRockHI::PrmSourceRockHI( mbapi::Model & mdl
                                , const char   * layerName
                                , const char * srType
                                , int          mixingID
                                ) : PrmSourceRockProp( mdl, layerName, srType, mixingID )
{ 
   m_propName = "HI";

   mbapi::SourceRockManager   & srMgr = mdl.sourceRockManager();
   mbapi::StratigraphyManager & stMgr = mdl.stratigraphyManager();

   mbapi::StratigraphyManager::LayerID lid = stMgr.layerID( m_layerName );
   const std::vector<std::string> & srtNames = stMgr.sourceRockTypeName( lid );
   mbapi::SourceRockManager::SourceRockID sid = srMgr.findID( m_layerName, (srType ? std::string( srType ) : srtNames[m_mixID-1]) );
   m_val = srMgr.hiIni( sid );

   // construct parameter name
   std::ostringstream oss;
   oss << "SourceRockHI(" << m_layerName << "," << m_mixID << ")";
   m_name = oss.str();
}

 // Constructor
PrmSourceRockHI::PrmSourceRockHI( const VarPrmSourceRockHI * parent
                                , double                     val
                                , const char               * lrName
                                , const char               * srType
                                , int                        mixingID 
                                ) : PrmSourceRockProp( parent, val, lrName, srType, mixingID )
{
   m_propName = "HI";
   // construct parameter name
   std::ostringstream oss;
   oss << "SourceRockHI(" << m_layerName << "," << m_mixID << ")";
   m_name = oss.str();
}


// Update given model with the parameter value
ErrorHandler::ReturnCode PrmSourceRockHI::setInModel( mbapi::Model & caldModel, size_t /* caseID */ )
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
      if ( ErrorHandler::NoError != srMgr.setHIIni( sid, m_val ) )
      {
         throw ErrorHandler::Exception( srMgr.errorCode() ) << srMgr.errorMessage();
      }
   }
   catch ( const ErrorHandler::Exception & e ) { return caldModel.reportError( e.errorCode(), e.what() ); }

   return ErrorHandler::NoError;
}

// Validate HI value if it is in [0:100] range
std::string PrmSourceRockHI::validate( mbapi::Model & caldModel )
{
   std::ostringstream oss;

   if (      m_val < 0    ) oss << "HI value for the layer " << m_layerName << ", can not be negative: " << m_val << std::endl;
   else if ( m_val > 1000 ) oss << "HI value for the layer " << m_layerName << ", can not be more than 1000 kg/tonne: " << m_val << std::endl;

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
            "HI setting error: source rock is not active for the layer:" << m_layerName;
      }

      double hiInModel = UndefinedDoubleValue;

      // go to source rock lithology table for the source rock hi
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
            " for the mixing ID " << m_mixID << " to source rock type defined for the H/C parameter: " << m_srTypeName; 
      }

      mbapi::SourceRockManager::SourceRockID sid = srMgr.findID( m_layerName, (!m_srTypeName.empty() ? m_srTypeName : srtNames[m_mixID-1]) );
      if ( IsValueUndefined( sid ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Can not find source rock lithology for layer "
            << m_layerName << " and SR type " << (!m_srTypeName.empty() ? m_srTypeName : srtNames[m_mixID-1]);
      }
      hiInModel = srMgr.hiIni( sid );

      if ( !NumericFunctions::isEqual( hiInModel, m_val, 1.0 ) ) // HI is set through recalculation to H/C 
      {
         oss << "Value of HI in the model (" << hiInModel << ") is different from the parameter value (" << m_val << ")" << std::endl;
      }
   }
   catch ( const ErrorHandler::Exception & e ) { oss << e.what() << std::endl; }

   return oss.str();
}


// Are two parameters equal?
bool PrmSourceRockHI::operator == ( const Parameter & prm ) const
{
   const PrmSourceRockHI * pp = dynamic_cast<const PrmSourceRockHI *>( &prm );
   if ( !pp ) return false;
   
   const double eps = 1.e-6;

   if ( m_layerName != pp->m_layerName   ) return false;
   if ( m_mixID      != pp->m_mixID      ) return false;
   if ( m_srTypeName != pp->m_srTypeName ) return false;

   if ( !NumericFunctions::isEqual( m_val, pp->m_val, eps ) ) return false;

   return true;
}


// Save all object data to the given stream, that object could be later reconstructed from saved data
bool PrmSourceRockHI::save( CasaSerializer & sz, unsigned int version ) const
{
   bool ok = PrmSourceRockProp::serializeCommonPart( sz, version ); 
   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
PrmSourceRockHI::PrmSourceRockHI( CasaDeserializer & dz, unsigned int objVer )
{
   bool ok = PrmSourceRockProp::deserializeCommonPart( dz, objVer );
   if ( m_propName.empty() ) { m_propName = "HI"; }

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "PrmSourceRockHI deserialization unknown error";
   }
}

} // namespace casa
