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
#include "Path.h" // for to_string

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
                                  , const char * srType
                                  , int          mixingID
                                  ) : PrmSourceRockProp( mdl, layerName, srType, mixingID )
{    
   // TOC specific part
   m_propName = "TOC";

   mbapi::SourceRockManager   & srMgr = mdl.sourceRockManager();
   mbapi::StratigraphyManager & stMgr = mdl.stratigraphyManager();

   // get this layer has a mix of source rocks (all checking were done in parent constructor
   const std::vector<std::string> & srtNames = stMgr.sourceRockTypeName( stMgr.layerID( m_layerName ) );
   mbapi::SourceRockManager::SourceRockID sid = srMgr.findID( m_layerName, (!m_srTypeName.empty() ? m_srTypeName : srtNames[m_mixID-1]) );

   // check if in base project TOC is defined as a map
   const std::string & mapName = srMgr.tocInitMapName( sid );
   if ( !mapName.empty() )
   {
      mbapi::MapsManager & mpMgr = mdl.mapsManager();
      const mbapi::MapsManager::MapID mID = mpMgr.findID( mapName );
      if ( UndefinedIDValue == mID )
      {
         throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Source rock lithology for layer" << m_layerName
            << " has TOC defined as unknown map :" << mapName; 
      }
      double minVal, maxVal;
      mpMgr.mapValuesRange( mID, minVal, maxVal );
      m_val = maxVal;
   }
   else
   {
      m_val = srMgr.tocIni( sid );
   }

   // construct parameter name
   std::ostringstream oss;
   oss << "SourceRockTOC(" << m_layerName << "," << m_mixID << ")";
   m_name = oss.str();
}

 // Constructor
PrmSourceRockTOC::PrmSourceRockTOC( const VarPrmSourceRockTOC * parent
                                  , double                      val
                                  , const char                * lrName
                                  , const char                * srType
                                  , int                         mixingID 
                                 ) : PrmSourceRockProp( parent, val, lrName, srType, mixingID )
{
   m_propName = "TOC";
   // construct parameter name
   std::ostringstream oss;
   oss << "SourceRockTOC(" << m_layerName << "," << mixingID << ")";
   m_name = oss.str();
}

// Destructor
PrmSourceRockTOC::~PrmSourceRockTOC() {;}


// Update given model with the parameter value
ErrorHandler::ReturnCode PrmSourceRockTOC::setInModel( mbapi::Model & caldModel, size_t caseID )
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

      const std::vector<std::string> & srtNames = stMgr.sourceRockTypeName( lid );
      if ( srtNames.empty() || m_mixID < 1 || m_mixID > srtNames.size() )
      {
         throw ErrorHandler::Exception(ErrorHandler::UndefinedValue) << "Layer " << m_layerName <<
            " set as source rock layer but has no source rock lithology defined for the mixing ID: " << m_mixID;
      }

      mbapi::SourceRockManager::SourceRockID sid = srMgr.findID( m_layerName, (!m_srTypeName.empty() ? m_srTypeName : srtNames[m_mixID-1]) );
      if ( IsValueUndefined( sid ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Can not find source rock lithology for layer "
            << m_layerName << " and SR type " << (!m_srTypeName.empty() ? m_srTypeName : srtNames[m_mixID-1]);
      }

      // check if in base project TOC is defined as a map
      const std::string & mapName = srMgr.tocInitMapName( sid );
      if ( !mapName.empty() )
      {
         mbapi::MapsManager & mpMgr = caldModel.mapsManager();
         mbapi::MapsManager::MapID mID = mpMgr.findID( mapName );

         if ( UndefinedIDValue == mID )
         {
            throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Source rock lithology for layer" << m_layerName
               << " has TOC defined as unknown map :" << mapName; 
         }

         // copy map and rescale it for max TOC
         mbapi::MapsManager::MapID cmID = mpMgr.copyMap( mID, mapName + "_Case_" + ibs::to_string( caseID ) + "_VarTOC" );
         if ( UndefinedIDValue == cmID )
         {
            ErrorHandler::Exception( ErrorHandler::IoError ) << "Copy TOC map " << mapName << " failed";
         }

         double minVal, maxVal;

         bool ok = ErrorHandler::NoError == mpMgr.mapValuesRange( mID, minVal, maxVal ) ? true : false;
         ok = ErrorHandler::NoError == mpMgr.scaleMap( cmID, (NumericFunctions::isEqual(0.0, maxVal, 1e-10) ? 0.0 : m_val/maxVal)) ? true:ok;
         ok = ErrorHandler::NoError == mpMgr.saveMapToHDF( cmID, mapName + "_VarTOC.HDF" ) ? true : ok;

         if ( !ok ) { throw ErrorHandler::Exception( srMgr.errorCode() ) << srMgr.errorMessage(); }
    
      }
      else if ( ErrorHandler::NoError != srMgr.setTOCIni( sid, m_val ) )
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

   if (      m_val < 0   ) oss << "TOC value for the layer " << m_layerName << ", can not be negative: "       << m_val << std::endl;
   else if ( m_val > 100 ) oss << "TOC value for the layer " << m_layerName << ", can not be more than 100%: " << m_val << std::endl;

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

      // go to source rock lithology table for the source rock TOC
      const std::vector<std::string> & srtNames = stMgr.sourceRockTypeName( lid );

      if ( srtNames.empty() )
      {
         throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Layer " << m_layerName <<
            " set as source rock layer but has no source rock lithology defined";
      }

      if ( srtNames.size() < m_mixID )
      {
         throw ErrorHandler::Exception( ErrorHandler::ValidationError ) <<
            "Layer " << m_layerName << " has no source rock definition for the mixing ID: " << m_mixID;
      }

      if ( !m_srTypeName.empty() && srtNames[m_mixID - 1] != m_srTypeName )
      {
         throw ErrorHandler::Exception( ErrorHandler::ValidationError ) <<
            "Layer " << m_layerName << " has unmatched source rock type name :" << srtNames[m_mixID-1] << 
            " for the mixing ID " << m_mixID << " to source rock type defined for the TOC parameter: " << m_srTypeName; 
      }

      mbapi::SourceRockManager::SourceRockID sid = srMgr.findID( m_layerName, (!m_srTypeName.empty() ? m_srTypeName : srtNames[m_mixID-1]) );
      if ( IsValueUndefined( sid ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Can not find source rock lithology for layer "
            << m_layerName << " and SR type " << (!m_srTypeName.empty() ? m_srTypeName : srtNames[m_mixID-1]);
      }

      // check if in base project TOC is defined as a map
      const std::string & mapName = srMgr.tocInitMapName( sid );
      if ( !mapName.empty() )
      {
         mbapi::MapsManager & mpMgr = caldModel.mapsManager();
         mbapi::MapsManager::MapID mID = mpMgr.findID( mapName );

         if ( UndefinedIDValue == mID )
         {
            throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Source rock lithology for layer" << m_layerName
               << " has TOC defined as unknown map :" << mapName; 
         }
         // copy map and rescale it for max TOC
         double minV;
         mpMgr.mapValuesRange( mID, minV, tocInModel );
      }
      else { tocInModel = srMgr.tocIni( sid ); }

      if ( !NumericFunctions::isEqual( tocInModel, m_val, 1.e-4 ) )
      {
         oss << "Value of TOC in the model (" << tocInModel << ") is different from the parameter value (" << m_val << ")" << std::endl;
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

   if ( m_layerName  != pp->m_layerName  ) return false;
   if ( m_mixID      != pp->m_mixID      ) return false;
   if ( m_srTypeName != pp->m_srTypeName ) return false;

   if ( !NumericFunctions::isEqual( m_val, pp->m_val, eps ) ) return false;

   return true;
}


// Save all object data to the given stream, that object could be later reconstructed from saved data
bool PrmSourceRockTOC::save( CasaSerializer & sz, unsigned int version ) const
{
   bool ok = PrmSourceRockProp::serializeCommonPart( sz, version ); 

   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
PrmSourceRockTOC::PrmSourceRockTOC( CasaDeserializer & dz, unsigned int objVer )
{
   bool ok = PrmSourceRockProp::deserializeCommonPart( dz, objVer );

   if ( m_propName.empty() ) { m_propName = "TOC"; }

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "PrmSourceRockTOC deserialization unknown error";
   }
}

} // namespace casa
