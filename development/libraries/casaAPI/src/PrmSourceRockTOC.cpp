//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file PrmSourceRockTOC.cpp
/// @brief This file keeps API implementation for Source Rock TOC parameter handling

// CASA API
#include "PrmSourceRockTOC.h"
#include "VarPrmSourceRockTOC.h"

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

static const char * s_mapFileSuffix        = ".HDF";
static const char * s_mapNameSuffix        = "_TOC";

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

   if ( m_srTypeName.empty() ) { m_srTypeName = srtNames[m_mixID-1]; }
   mbapi::SourceRockManager::SourceRockID sid = srMgr.findID( m_layerName, m_srTypeName );

   if ( Utilities::isValueUndefined( sid ) )
   {
      throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find source rock lithology: " << m_srTypeName;
   }

   // check if in base project TOC is defined as a map
   const std::string & mapName = srMgr.tocInitMapName( sid );
   if ( !mapName.empty() )
   {
      mbapi::MapsManager & mpMgr = mdl.mapsManager();
      const mbapi::MapsManager::MapID mID = mpMgr.findID( mapName );
      if ( Utilities::isValueUndefined( mID ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Source rock lithology for layer" << m_layerName
            << " has TOC defined as unknown map :" << mapName;
      }
      double minVal, maxVal;
      if ( ErrorHandler::NoError != mpMgr.mapValuesRange( mID, minVal, maxVal ) )
      {
         throw ErrorHandler::Exception( mpMgr.errorCode() ) << mpMgr.errorMessage();
      }
      m_val = maxVal;
      m_mapName = mapName;
   }
   else
   {
      m_val = srMgr.tocIni( sid );
   }
}

 // Constructor
PrmSourceRockTOC::PrmSourceRockTOC( const VarPrmSourceRockTOC * parent
                                  , double                      val
                                  , const char                * lrName
                                  , const std::string         & mapName
                                  , const char                * srType
                                  , int                         mixingID
                                 ) : PrmSourceRockProp( parent, val, lrName, srType, mixingID )
{
   m_mapName = mapName;
   m_propName = "TOC";
}

// Destructor
PrmSourceRockTOC::~PrmSourceRockTOC() {;}


// Update given model with the parameter value
ErrorHandler::ReturnCode PrmSourceRockTOC::setInModel( mbapi::Model & caldModel, size_t /*caseID*/ )
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
      if ( srtNames.empty() || m_mixID < 1 || static_cast<size_t>( m_mixID ) > srtNames.size() )
      {
         throw ErrorHandler::Exception(ErrorHandler::UndefinedValue) << "Layer " << m_layerName <<
            " set as source rock layer but has no source rock lithology defined for the mixing ID: " << m_mixID;
      }

      mbapi::SourceRockManager::SourceRockID sid = srMgr.findID( m_layerName, (!m_srTypeName.empty() ? m_srTypeName : srtNames[m_mixID-1]) );
      if ( Utilities::isValueUndefined( sid ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Can not find source rock lithology for layer "
            << m_layerName << " and SR type " << (!m_srTypeName.empty() ? m_srTypeName : srtNames[m_mixID-1]);
      }

      mbapi::MapsManager & mpMgr = caldModel.mapsManager();
      if ( m_mapName.empty() && m_minMapName.empty() && m_maxMapName.empty() )
      {
         const std::string & mapName = srMgr.tocInitMapName( sid );
         // check if in base project TOC is defined as a map
         if ( ! mapName.empty() )
         {
            mbapi::MapsManager::MapID mID = mpMgr.findID( mapName );

            if ( Utilities::isValueUndefined( mID ) )
            {
               throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Source rock lithology for layer"   << m_layerName
                                                                            << " has TOC defined as unknown map :" << mapName;
            }

            // extract min/max values from the map
            double minVal, maxVal;
            if ( ErrorHandler::NoError != mpMgr.mapValuesRange( mID, minVal, maxVal ) )
            {
               throw ErrorHandler::Exception( mpMgr.errorCode() ) << mpMgr.errorMessage();
            }

            double scaleCoeff = NumericFunctions::isEqual( 0.0, maxVal, 1e-10 ) ? 0.0 : ( value() / maxVal );

            if ( scaleCoeff != 1.0 ) // for some cases we will have base case value, in this case we do not need copy map
            {
               // copy map to avoid influence on other project parts
               std::string newMapName = "CasaGeneratedMap_" + caldModel.randomString( 3 ) + "_" + s_mapNameSuffix;
               mbapi::MapsManager::MapID cmID = mpMgr.copyMap( mID, newMapName );
               if ( Utilities::isValueUndefined( cmID ) )
               {
                  throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Copy initial TOC map " << mapName << " failed";
               }

               // scale map with new maximum value
               if ( ErrorHandler::NoError != mpMgr.scaleMap( cmID, scaleCoeff ) )
               {
                  throw ErrorHandler::Exception( mpMgr.errorCode() ) << mpMgr.errorMessage();
               }

               // save map to separate HDF file
               if ( ErrorHandler::NoError != mpMgr.saveMapToHDF( cmID, newMapName + s_mapFileSuffix ) )
               {
                  throw ErrorHandler::Exception( mpMgr.errorCode() ) << mpMgr.errorMessage();
               }

               // update project with new map name
               if ( ErrorHandler::NoError != srMgr.setTOCInitMapName( sid, newMapName ) )
               {
                  throw ErrorHandler::Exception( srMgr.errorCode() ) << srMgr.errorMessage();
               }
            }
         }
         else if ( ErrorHandler::NoError != srMgr.setTOCIni( sid, value() ) )
         {
            throw ErrorHandler::Exception( srMgr.errorCode() ) << srMgr.errorMessage();
         }
      }
      else // interpolation between two min/max maps
      {
         if ( m_minMapName.empty() && m_maxMapName.empty() ) // no interpolation needed, just set map name
         {
            if ( ErrorHandler::NoError != srMgr.setTOCInitMapName( sid, m_mapName ) )
            {
               throw ErrorHandler::Exception( srMgr.errorCode() ) << srMgr.errorMessage();
            }
         }
         else // interpolation is needed
         {
            mbapi::MapsManager::MapID minID = mpMgr.findID( m_minMapName );
            if ( Utilities::isValueUndefined( minID ) ) { throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find map: " << m_minMapName; }

            mbapi::MapsManager::MapID maxID = mpMgr.findID( m_maxMapName );
            if ( Utilities::isValueUndefined( maxID ) ) { throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find map: " << m_maxMapName; }

            std::string newMapName;

            // value interval for maps range case is [-1:0:1] but min/max maps are set for [-1:0] or [0:1] interval
            double halfRangeVal = m_val < 0.0 ? m_val + 1.0 : m_val;

            if (      halfRangeVal == 0.0 ) { newMapName = m_minMapName; }
            else if ( halfRangeVal == 1.0 ) { newMapName = m_maxMapName; }

            if ( newMapName.empty() )
            {
               // copy map and overwrite it when interpolating between min/max maps values
               newMapName = "CasaGeneratedMap_" + caldModel.randomString( 3 ) + "_" + s_mapNameSuffix;
               mbapi::MapsManager::MapID cmID = mpMgr.copyMap( minID, newMapName );

               if ( Utilities::isValueUndefined( cmID ) )
               {
                  throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Copy initial TOC map " << m_mapName << " failed";
               }

               if ( ErrorHandler::NoError != mpMgr.interpolateMap( cmID, minID, maxID, halfRangeVal ) )
               {
                  throw ErrorHandler::Exception( mpMgr.errorCode() ) << mpMgr.errorMessage();
               }

               // Save new map to file
               if ( ErrorHandler::NoError != mpMgr.saveMapToHDF( cmID, newMapName + s_mapFileSuffix ) )
               {
                  throw ErrorHandler::Exception( mpMgr.errorCode() ) << mpMgr.errorMessage();
               }
            }
            // Set this map to table
            if ( ErrorHandler::NoError != srMgr.setTOCInitMapName( sid, newMapName ) )
            {
               throw ErrorHandler::Exception( srMgr.errorCode() ) << srMgr.errorMessage();
            }
         }
         if ( ErrorHandler::NoError != srMgr.setTOCIni( sid, Utilities::Numerical::IbsNoDataValue ) )
         {
            throw ErrorHandler::Exception( srMgr.errorCode() ) << srMgr.errorMessage();
         }
      }
   }
   catch ( const ErrorHandler::Exception & e ) { return caldModel.reportError( e.errorCode(), e.what() ); }

   return ErrorHandler::NoError;
}

// Validate TOC value if it is in [0:100] for simple range
std::string PrmSourceRockTOC::validate( mbapi::Model & caldModel )
{
   std::ostringstream oss;

   if ( m_mapName.empty() && m_minMapName.empty() && m_maxMapName.empty() ) // check this just for simple range
   {
      if (      m_val < 0   ) oss << "TOC value for the layer " << m_layerName << ", can not be negative: "       << m_val << std::endl;
      else if ( m_val > 100 ) oss << "TOC value for the layer " << m_layerName << ", can not be more than 100%: " << m_val << std::endl;
   }

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
         throw ErrorHandler::Exception( ErrorHandler::ValidationError ) << "TOC error: source rock is not active for the layer:" << m_layerName;
      }

      double tocInModel = Utilities::Numerical::IbsNoDataValue;

      // go to source rock lithology table for the source rock TOC
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
            " for the mixing ID " << m_mixID << " to source rock type defined for the TOC parameter: " << m_srTypeName;
      }

      mbapi::SourceRockManager::SourceRockID sid = srMgr.findID( m_layerName, (!m_srTypeName.empty() ? m_srTypeName : srtNames[m_mixID-1]) );
      if ( Utilities::isValueUndefined( sid ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Can not find source rock lithology for layer "
            << m_layerName << " and SR type " << (!m_srTypeName.empty() ? m_srTypeName : srtNames[m_mixID-1]);
      }

      // check if in base project TOC is defined as a map
      const std::string & mapName = srMgr.tocInitMapName( sid );

      if ( m_mapName.empty() && m_minMapName.empty() && m_maxMapName.empty() )
      {
         tocInModel = srMgr.tocIni( sid );

         if ( mapName.empty() )
         {
            if ( !NumericFunctions::isEqual( tocInModel, m_val, 1.e-4 ) ) // just scalar value
            {
               oss << "Value of TOC in the model (" << tocInModel << ") is different from the parameter value (" << m_val << ")" << std::endl;
            }
         }
         else // scaler
         {
            // get map and check that scaling was done right
            mbapi::MapsManager & mpMgr = caldModel.mapsManager();

            mbapi::MapsManager::MapID mID = mpMgr.findID( mapName ); // get map
            if ( Utilities::isValueUndefined( mID ) )
            {
               throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find the map: " << mapName
                                                                            << " defined for initial TOC in maps catalog";
            }

            double minVal, maxVal;
            if ( ErrorHandler::NoError != mpMgr.mapValuesRange( mID, minVal, maxVal ) )
            {
               throw ErrorHandler::Exception( mpMgr.errorCode() ) << mpMgr.errorMessage();
            }

            if ( !NumericFunctions::isEqual( m_val, maxVal, 1e-6 ) )
            {
               oss << "Scaled initial TOC map in the project: " << maxVal << ", is differ from the parameter value: " << m_val;
            }
         }
      }
      else
      {
         if ( m_minMapName.empty() && m_maxMapName.empty() ) // no interpolation needed, just check map name
         {
            if ( m_mapName != mapName )
            {
               oss << "Map name in project: " << mapName << ", is different from parameter value: " << m_mapName;
               return oss.str();
            }
         }
         else
         {
            // get map and check that scaling was done right
            mbapi::MapsManager & mpMgr = caldModel.mapsManager();

            mbapi::MapsManager::MapID minID = mpMgr.findID( m_minMapName );
            if ( Utilities::isValueUndefined( minID ) ) { throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find map: " << m_minMapName; }
            mbapi::MapsManager::MapID maxID = mpMgr.findID( m_maxMapName );
            if ( Utilities::isValueUndefined( maxID ) ) { throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find map: " << m_maxMapName; }
            mbapi::MapsManager::MapID mID = mpMgr.findID( mapName ); // get map
            if ( Utilities::isValueUndefined( mID   ) ) { throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find map: " << mapName; }

            // get min/max values for min/max maps
            double minMinVal, minMaxVal, maxMinVal, maxMaxVal, minVal, maxVal;
            if ( ErrorHandler::NoError != mpMgr.mapValuesRange( minID, minMinVal, minMaxVal ) )
            {
               throw ErrorHandler::Exception( mpMgr.errorCode() ) << mpMgr.errorMessage();
            }

            if ( ErrorHandler::NoError != mpMgr.mapValuesRange( maxID, maxMinVal, maxMaxVal ) )
            {
               throw ErrorHandler::Exception( mpMgr.errorCode() ) << mpMgr.errorMessage();
            }

            if ( ErrorHandler::NoError != mpMgr.mapValuesRange( mID, minVal, maxVal ) )
            {
               throw ErrorHandler::Exception( mpMgr.errorCode() ) << mpMgr.errorMessage();
            }

            if ( minVal < minMinVal ) { oss << "Minimal value of the TOC map is outside of the range"; }
            if ( maxVal > maxMaxVal ) { oss << "Maximal value of the TOC map is outside of the range"; }
            // TODO scale the map again and compare with map from project file
         }
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
   if ( m_mapName    != pp->m_mapName    ) return false;

   if ( !NumericFunctions::isEqual( m_val, pp->m_val, eps ) ) return false;

   return true;
}


// Save all object data to the given stream, that object could be later reconstructed from saved data
bool PrmSourceRockTOC::save( CasaSerializer & sz ) const
{
   bool ok = PrmSourceRockProp::serializeCommonPart( sz );

   ok = ok ? sz.save( m_mapName,    "mapName"    ) : ok;
   ok = ok ? sz.save( m_minMapName, "minMapName" ) : ok;
   ok = ok ? sz.save( m_maxMapName, "maxMapName" ) : ok;

   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
PrmSourceRockTOC::PrmSourceRockTOC( CasaDeserializer & dz, unsigned int objVer )
{
   bool ok = PrmSourceRockProp::deserializeCommonPart( dz, objVer );

   if ( m_propName.empty() ) { m_propName = "TOC"; }

   if ( objVer > 0 )
   {
      ok = ok ? dz.load( m_mapName,    "mapName"    ) : ok;
      ok = ok ? dz.load( m_minMapName, "minMapName" ) : ok;
      ok = ok ? dz.load( m_maxMapName, "maxMapName" ) : ok;
   }

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "PrmSourceRockTOC deserialization unknown error";
   }
}

} // namespace casa
