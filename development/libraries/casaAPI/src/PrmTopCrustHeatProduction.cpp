//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PrmTopCrustHeatProduction.h
/// @brief This file keeps API implementation for handling Top Crust Heat Production Rate basement parameter. 

// CASA API
#include "PrmTopCrustHeatProduction.h"
#include "VarPrmTopCrustHeatProduction.h"

// CMB API
#include "cmbAPI.h"
#include "ErrorHandler.h"
#include "UndefinedValues.h"

// utilities lib
#include <NumericFunctions.h>

// STL/C lib
#include <cassert>
#include <cmath>
#include <sstream>

namespace casa
{

static const char * s_basementTblName      = "BasementIoTbl";
static const char * s_bottomBoundaryModel  = "BottomBoundaryModel";
static const char * s_topCrustHeatProd     = "TopCrustHeatProd";
static const char * s_topCrustHeatProdGrid = "TopCrustHeatProdGrid";

static const char * s_mapFileSuffix        = "_VarRHPR.HDF";
static const char * s_mapNameSuffix        = "_RHPR";

// Constructor
PrmTopCrustHeatProduction::PrmTopCrustHeatProduction( const VarPrmTopCrustHeatProduction * parent, double val, std::string mapName )
   : m_parent( parent )
   , m_value( val )
   , m_mapName( mapName )
{
}

// This constructor extracts top crust heat production rate value from the model and initilize class with this value.
// If project has top crust production rate defined as a map, constructor extracts map maximum value and use it to
// initilize class with this value and stores map name also.
PrmTopCrustHeatProduction::PrmTopCrustHeatProduction( mbapi::Model & mdl ) : m_parent( 0 )
{
   const std::string & modelName = mdl.tableValueAsString( s_basementTblName, 0, s_bottomBoundaryModel );
   if ( ErrorHandler::NoError != mdl.errorCode() ) { throw ErrorHandler::Exception( mdl.errorCode() ) << mdl.errorMessage(); }

   if ( modelName != "Fixed Temperature" )
   {
      throw ErrorHandler::Exception( ErrorHandler::ValidationError ) <<  
            "Unsupported bottom boundary model: '" << modelName << "' for using with top crust heat production rate.";
   }
 
   const std::string & mapName = mdl.tableValueAsString( s_basementTblName, 0, s_topCrustHeatProdGrid );
   if ( ErrorHandler::NoError != mdl.errorCode() ) {  throw ErrorHandler::Exception( mdl.errorCode() ) << mdl.errorMessage(); }

   if ( ! mapName.empty() ) 
   {
      mbapi::MapsManager & mpMgr = mdl.mapsManager();
      mbapi::MapsManager::MapID mID = mpMgr.findID( mapName );

      if ( UndefinedIDValue == mID )
      {
         throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find the map: " << mapName
               << " defined for top crust heat production rate in maps catalog"; 
      }

      double minVal, maxVal;
      if ( ErrorHandler::NoError != mpMgr.mapValuesRange( mID, minVal, maxVal ) )
      {
         throw ErrorHandler::Exception( mpMgr.errorCode() ) << mpMgr.errorMessage();
      }
 
      m_value = maxVal;
      m_mapName = mapName;
   }
   else
   {
      m_value = mdl.tableValueAsDouble( s_basementTblName, 0, s_topCrustHeatProd );
      if ( ErrorHandler::NoError != mdl.errorCode() ) { throw ErrorHandler::Exception( mdl.errorCode() ) << mdl.errorMessage(); }
   }
}

// Destructor
PrmTopCrustHeatProduction::~PrmTopCrustHeatProduction() {;}

// Update given model with the parameter value
ErrorHandler::ReturnCode PrmTopCrustHeatProduction::setInModel( mbapi::Model & cldModel, size_t caseID )
{
   try
   {
      // check which model is used in project file for top crust heat production rate, "Fixed Temperature" is the only supported now model
      const std::string & modelName = cldModel.tableValueAsString( s_basementTblName, 0, s_bottomBoundaryModel );
      if ( ErrorHandler::NoError != cldModel.errorCode() ) { throw ErrorHandler::Exception( cldModel.errorCode() ) << cldModel.errorMessage(); }

      if ( modelName != "Fixed Temperature" )
      {
         throw ErrorHandler::Exception( ErrorHandler::ValidationError ) <<  
               "Unsupported bottom boundary model: '" << modelName << "' for using with top crust heat production rate.";
      }

      if ( m_mapName.empty() ) // scalar value or scaled map case
      {
         const std::string & mapName = cldModel.tableValueAsString( s_basementTblName, 0, s_topCrustHeatProdGrid );
         if ( ErrorHandler::NoError != cldModel.errorCode() ) throw ErrorHandler::Exception( cldModel.errorCode() ) << cldModel.errorMessage();

         // check if in base project heat production rate is defined as a map
         if ( ! mapName.empty() ) // if yes, rescale map according to the new maximum value
         {
            mbapi::MapsManager & mpMgr = cldModel.mapsManager();
            mbapi::MapsManager::MapID mID = mpMgr.findID( mapName ); // get map

            if ( UndefinedIDValue == mID )
            {
               throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find the map: " << mapName
                  << " defined for top crust heat production rate in maps catalog"; 
            }

            // copy map to avoid influence on other project parts
            std::string newMapName = mapName + "_Case_" + std::to_string( caseID + 1 ) + s_mapNameSuffix;
            mbapi::MapsManager::MapID cmID = mpMgr.copyMap( mID, newMapName );
            if ( UndefinedIDValue == cmID )
            {
               throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Copy radiogenic heat production rate map " << mapName << " failed";
            }

            // extract min/max values from the map
            double minVal, maxVal;

            if ( ErrorHandler::NoError != mpMgr.mapValuesRange( mID, minVal, maxVal ) )
            {
               throw ErrorHandler::Exception( cldModel.errorCode() ) << cldModel.errorMessage();
            }

            double scaleCoeff = NumericFunctions::isEqual( 0.0, maxVal, 1e-10 ) ? 0.0 : ( m_value / maxVal );

            // scale map with new maximum value
            if ( ErrorHandler::NoError != mpMgr.scaleMap( cmID, scaleCoeff ) )
            {
               throw ErrorHandler::Exception( cldModel.errorCode() ) << cldModel.errorMessage();
            }

            // save map to separate HDF file
            if ( ErrorHandler::NoError != mpMgr.saveMapToHDF( cmID, mapName + s_mapFileSuffix, 0 ) )
            {
               throw ErrorHandler::Exception( cldModel.errorCode() ) << cldModel.errorMessage(); 
            }
           
            // update project with new map name 
            if ( ErrorHandler::NoError != cldModel.setTableValue( s_basementTblName, 0, s_topCrustHeatProdGrid, newMapName ) )
            {
               throw ErrorHandler::Exception( cldModel.errorCode() ) << cldModel.errorMessage();
            }
            if ( ErrorHandler::NoError != cldModel.setTableValue( s_basementTblName, 0, s_topCrustHeatProd, UndefinedDoubleValue ) )
            {
               throw ErrorHandler::Exception( cldModel.errorCode() ) << cldModel.errorMessage();
            }
         }
         else if ( ErrorHandler::NoError != cldModel.setTableValue( s_basementTblName, 0, s_topCrustHeatProd, m_value ) )
         {
            throw ErrorHandler::Exception( cldModel.errorCode() ) << cldModel.errorMessage();
         }
      }
      else // interpolation between two min/max maps 
      {
         if ( m_minMapName.empty() && m_maxMapName.empty() ) // no interpolation needed, just set map name
         {
            if ( ErrorHandler::NoError != cldModel.setTableValue( s_basementTblName, 0, s_topCrustHeatProdGrid, m_mapName ) )
            {
               throw ErrorHandler::Exception( cldModel.errorCode() ) << cldModel.errorMessage();
            }
         }
         else // interpolation is needed
         {
            mbapi::MapsManager & mpMgr = cldModel.mapsManager();

            mbapi::MapsManager::MapID bsID = mpMgr.findID( m_mapName );
            if ( UndefinedIDValue == bsID ) { throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find map: " << m_mapName; }

            mbapi::MapsManager::MapID minID = mpMgr.findID( m_minMapName );
            if ( UndefinedIDValue == minID ) { throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find map: " << m_minMapName; }

            mbapi::MapsManager::MapID maxID = mpMgr.findID( m_maxMapName );
            if ( UndefinedIDValue == maxID ) { throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find map: " << m_maxMapName; }

            // copy map and overwrite it when interpolating between min/max maps values
            std::string newMapName = m_mapName + "_Case_" + std::to_string( caseID + 1 ) + s_mapNameSuffix;
            mbapi::MapsManager::MapID cmID = mpMgr.copyMap( bsID, newMapName );

            if ( UndefinedIDValue == cmID )
            {
               throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Copy radiogenic heat production rate map " << m_mapName << " failed";
            }

            // value interval for maps range case is [-1:0:1] but min/max maps are set for [-1:0] or [0:1] interval
            if ( ErrorHandler::NoError != mpMgr.interpolateMap( cmID, minID, maxID, m_value < 0.0 ? m_value + 1.0 : m_value ) )
            {
               throw ErrorHandler::Exception( mpMgr.errorCode() ) << mpMgr.errorMessage();
            }
            
            // Save new map to file
            if ( ErrorHandler::NoError != mpMgr.saveMapToHDF( cmID, m_mapName + s_mapFileSuffix, 0 ) )
            {
               throw ErrorHandler::Exception( cldModel.errorCode() ) << cldModel.errorMessage();
            }
           
            // Set this map to table
            if ( ErrorHandler::NoError != cldModel.setTableValue( s_basementTblName, 0, s_topCrustHeatProdGrid, newMapName ) )
            {
               throw ErrorHandler::Exception( cldModel.errorCode() ) << cldModel.errorMessage();
            }
         }

         if ( ErrorHandler::NoError != cldModel.setTableValue( s_basementTblName, 0, s_topCrustHeatProd, UndefinedDoubleValue ) )
         {
            throw ErrorHandler::Exception( cldModel.errorCode() ) << cldModel.errorMessage();
         }
      }
   }
   catch ( const ErrorHandler::Exception & e ) { return cldModel.reportError( e.errorCode(), e.what() ); }

   return ErrorHandler::NoError;
}

// Validate top crust heat production rate value if it is in positive range\n
// also it checks does the given model has the same value for this parameter.
std::string PrmTopCrustHeatProduction::validate( mbapi::Model & cldModel )
{
   std::ostringstream oss;

   // Check for supported model
   const std::string & modelName = cldModel.tableValueAsString( s_basementTblName, 0, s_bottomBoundaryModel );
   if ( ErrorHandler::NoError != cldModel.errorCode() )
   {
      oss << cldModel.errorMessage() << std::endl;
      return oss.str();
   }

   if ( modelName == "Fixed HeatFlow" )
   {
      oss << "Unsupported bottom boundary model: 'Fixed HeatFlow' for using top crust heat production rate." << std::endl;
      return oss.str(); // another model, no reason to check further
   }

   // get value from the project
   const std::string & heatProdMap = cldModel.tableValueAsString( s_basementTblName, 0, s_topCrustHeatProdGrid );
   if ( ErrorHandler::NoError != cldModel.errorCode() )
   {
      oss << cldModel.errorMessage() << std::endl;
      return oss.str();
   }

   double valInModel = cldModel.tableValueAsDouble( s_basementTblName, 0, s_topCrustHeatProd );

   if ( ErrorHandler::NoError != cldModel.errorCode() )
   {
      oss << cldModel.errorMessage() << std::endl;
      return oss.str();
   }

   
   if ( m_mapName.empty() ) // scalar value or scaled map case
   {  
      if ( m_value < 0.0 )
      {
         oss << "Top crust heat production rate value can not be negative: " << m_value << std::endl;
      }

      if ( valInModel > 0.0  && !heatProdMap.empty() )
      {
         oss << "Project has a map defined for top crust heat production rate.";
         oss << " Map and one rate value can not be used together" << std::endl;
         return oss.str();
      }

      // just one value
      if ( heatProdMap.empty() )
      {
         if ( !NumericFunctions::isEqual( valInModel, m_value, 1e-6 ) )
         {
            oss << "Top crust heat production rate parameter value in the model (" << valInModel; 
            oss << ") is differ from a parameter value (" << m_value << ")" << std::endl;
         }  
      }
      else // scaler
      {
         // check that map was generated by this parameter
         size_t oldMapNameLen = heatProdMap.rfind( "_Case" );
         if ( oldMapNameLen == std::string::npos || heatProdMap.rfind( s_mapNameSuffix ) == std::string::npos )
         {
            oss << "Scaled top crust heat production rate map in the project is differ from the parameter value";
         }

         // get map and check that scaling was done right
         mbapi::MapsManager & mpMgr = cldModel.mapsManager();

         mbapi::MapsManager::MapID mID = mpMgr.findID( heatProdMap ); // get map
         if ( UndefinedIDValue == mID )
         {
            throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find the map: " << heatProdMap 
                                                 << " defined for top crust heat production rate in maps catalog"; 
         }

         double minVal, maxVal;
         if ( ErrorHandler::NoError != mpMgr.mapValuesRange( mID, minVal, maxVal ) )
         {
            throw ErrorHandler::Exception( cldModel.errorCode() ) << cldModel.errorMessage();
         }

         if ( !NumericFunctions::isEqual( m_value, maxVal, 1e-6 ) )
         {
            oss << "Scaled top crust heat production rate map in the project: " << maxVal << 
               ", is differ from the parameter value: " << m_value;
         }
      }
   }
   else
   {
      if ( m_minMapName.empty() && m_maxMapName.empty() ) // no interpolation needed, just check map name
      {
         if ( m_mapName != heatProdMap )
         {
            oss << "Map name in project: " << heatProdMap << ", is different from parameter value: " << m_mapName;
            return oss.str();
         }
      }
      else
      {
         // check that map was generated by this parameter
         if ( heatProdMap.rfind( "_Case" ) == std::string::npos || heatProdMap.rfind( s_mapNameSuffix ) == std::string::npos )
         {
            oss << "Interpolated top crust heat production rate map in the project is differ from the parameter value";
         }

         // get map and check that scaling was done right
         mbapi::MapsManager & mpMgr = cldModel.mapsManager();

         mbapi::MapsManager::MapID minID = mpMgr.findID( m_minMapName );
         if ( UndefinedIDValue == minID ) { throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find map: " << m_minMapName; }
         mbapi::MapsManager::MapID maxID = mpMgr.findID( m_maxMapName );
         if ( UndefinedIDValue == maxID ) { throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find map: " << m_maxMapName; }
         mbapi::MapsManager::MapID mID = mpMgr.findID( heatProdMap ); // get map
         if ( UndefinedIDValue == mID   ) { throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find map: " << heatProdMap; }


         // get min/max values for min/max maps
         double minMinVal, minMaxVal, maxMinVal, maxMaxVal, minVal, maxVal;
         if ( ErrorHandler::NoError != mpMgr.mapValuesRange( minID, minMinVal, minMaxVal ) )
         {
            throw ErrorHandler::Exception( cldModel.errorCode() ) << cldModel.errorMessage();
         }

         if ( ErrorHandler::NoError != mpMgr.mapValuesRange( maxID, maxMinVal, maxMaxVal ) )
         {
            throw ErrorHandler::Exception( cldModel.errorCode() ) << cldModel.errorMessage();
         }
 
         if ( ErrorHandler::NoError != mpMgr.mapValuesRange( mID, minVal, maxVal ) )
         {
            throw ErrorHandler::Exception( cldModel.errorCode() ) << cldModel.errorMessage();
         }

         if ( minVal < minMinVal ) { oss << "Minimal value of the top crust production rate map is outside of the range"; }
         if ( maxVal > maxMaxVal ) { oss << "Maximal value of the top crust production rate map is outside of the range"; }
         // TODO scale the map again and compare with map from project file
      }
   }
   return oss.str();
}


// Are two parameters equal?
bool PrmTopCrustHeatProduction::operator == ( const Parameter & prm ) const
{
   const PrmTopCrustHeatProduction * pp = dynamic_cast<const PrmTopCrustHeatProduction *>( &prm );
   if ( !pp ) return false;
   
   if ( !NumericFunctions::isEqual( m_value, pp->m_value, 1.e-6 ) ) return false;
   if ( m_mapName != pp->m_mapName                                ) return false;

   return true;
}


// Save all object data to the given stream, that object could be later reconstructed from saved data
bool PrmTopCrustHeatProduction::save( CasaSerializer & sz, unsigned int /* version */ ) const
{
   bool hasParent = m_parent ? true : false;
   bool ok = sz.save( hasParent, "hasParent" );

   if ( hasParent )
   {
      CasaSerializer::ObjRefID parentID = sz.ptr2id( m_parent );
      ok = ok ? sz.save( parentID, "VarParameterID" ) : ok;
   }
   ok = ok ? sz.save( m_value, "heatProdRateValue" ) : ok;

   ok = ok ? sz.save( m_mapName,    "mapName"    ) : ok;
   ok = ok ? sz.save( m_minMapName, "minMapName" ) : ok;
   ok = ok ? sz.save( m_maxMapName, "maxMapName" ) : ok;
   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
PrmTopCrustHeatProduction::PrmTopCrustHeatProduction( CasaDeserializer & dz, unsigned int objVer )
{
   CasaDeserializer::ObjRefID parentID;

   bool hasParent;
   bool ok = dz.load( hasParent, "hasParent" );

   if ( hasParent )
   {
      bool ok = dz.load( parentID, "VarParameterID" );
      m_parent = ok ? dz.id2ptr<VarParameter>( parentID ) : 0;
   }

   ok = ok ? dz.load( m_value, "heatProdRateValue" ) : ok;

   if ( objVer > 0 )
   {
      ok = ok ? dz.load( m_mapName,    "mapName"    ) : ok;
      ok = ok ? dz.load( m_minMapName, "minMapName" ) : ok;
      ok = ok ? dz.load( m_maxMapName, "maxMapName" ) : ok;
   }

   if ( !ok )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "PrmTopCrustHeatProduction deserialization unknown error";
   }
}
}
