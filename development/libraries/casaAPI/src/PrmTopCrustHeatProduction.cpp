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

// Constructor
PrmTopCrustHeatProduction::PrmTopCrustHeatProduction( const VarPrmTopCrustHeatProduction * parent, double val, std::string mapName )
   : m_parent( parent )
   , m_value( val )
   , m_mapName( mapName )
{
}

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
ErrorHandler::ReturnCode PrmTopCrustHeatProduction::setInModel( mbapi::Model & caldModel, size_t caseID )
{
   try
   {
      const std::string & modelName = caldModel.tableValueAsString( s_basementTblName, 0, s_bottomBoundaryModel );
      if ( ErrorHandler::NoError != caldModel.errorCode() ) { throw ErrorHandler::Exception( caldModel.errorCode() ) << caldModel.errorMessage(); }

      if ( modelName != "Fixed Temperature" )
      {
         throw ErrorHandler::Exception( ErrorHandler::ValidationError ) <<  
               "Unsupported bottom boundary model: '" << modelName << "' for using with top crust heat production rate.";
      }

      if ( m_mapName.empty() ) // value or map scaler case
      {
         const std::string & mapName = caldModel.tableValueAsString( s_basementTblName, 0, s_topCrustHeatProdGrid );
         if ( ErrorHandler::NoError != caldModel.errorCode() ) throw ErrorHandler::Exception( caldModel.errorCode() ) << caldModel.errorMessage();

         // check if in base project heat production rate is defined as a map
         if ( ! mapName.empty() ) // if yes rescale map according to the new maximum value
         {
            mbapi::MapsManager & mpMgr = caldModel.mapsManager();
            mbapi::MapsManager::MapID mID = mpMgr.findID( mapName ); // get map

            if ( UndefinedIDValue == mID )
            {
               throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find the map: " << mapName
                  << " defined for top crust heat production rate in maps catalog"; 
            }

            // copy map to avoid influence on other project parts
            std::string newMapName = mapName + "_Case_" + ibs::to_string( caseID + 1 ) + "_RHPR";
            mbapi::MapsManager::MapID cmID = mpMgr.copyMap( mID, newMapName );
            if ( UndefinedIDValue == cmID )
            {
               ErrorHandler::Exception( ErrorHandler::IoError ) << "Copy radiogenic heat production rate map " << mapName << " failed";
            }

            // extract min/max values from the map
            double minVal, maxVal;

            bool ok = ErrorHandler::NoError == mpMgr.mapValuesRange( mID, minVal, maxVal ) ? true : false;
            double scaleCoeff = NumericFunctions::isEqual(0.0, maxVal, 1e-10) ? 0.0 : (m_value / maxVal);

            // scale map with new maximum value
            ok = ErrorHandler::NoError == mpMgr.scaleMap( cmID, scaleCoeff ) ? true : ok;

            // save map to separate HDF file
            ok = ErrorHandler::NoError == mpMgr.saveMapToHDF( cmID, mapName + "_VarRHPR.HDF" ) ? true : ok;
            if ( !ok ) { throw ErrorHandler::Exception( caldModel.errorCode() ) << caldModel.errorMessage(); }
           
            // update project with new map name 
            if ( ErrorHandler::NoError != caldModel.setTableValue( s_basementTblName, 0, s_topCrustHeatProdGrid, newMapName ) )
            {
               throw ErrorHandler::Exception( caldModel.errorCode() ) << caldModel.errorMessage();
            }
            if ( ErrorHandler::NoError != caldModel.setTableValue( s_basementTblName, 0, s_topCrustHeatProd, UndefinedDoubleValue ) )
            {
               throw ErrorHandler::Exception( caldModel.errorCode() ) << caldModel.errorMessage();
            }
         }
         else if ( ErrorHandler::NoError != caldModel.setTableValue( s_basementTblName, 0, s_topCrustHeatProd, m_value ) )
         {
            throw ErrorHandler::Exception( caldModel.errorCode() ) << caldModel.errorMessage();
         }
      }
      else // interpolation between maps case
      {
         if ( m_minMapName.empty() && m_maxMapName.empty() ) // no interpolation needed, just set map name
         {
            if ( ErrorHandler::NoError != caldModel.setTableValue( s_basementTblName, 0, s_topCrustHeatProdGrid, m_mapName ) )
            {
               throw ErrorHandler::Exception( caldModel.errorCode() ) << caldModel.errorMessage();
            }
         }
         else // interpolation is needed
         {
            mbapi::MapsManager & mpMgr = caldModel.mapsManager();

            mbapi::MapsManager::MapID bsID = mpMgr.findID( m_mapName );
            if ( UndefinedIDValue == bsID ) { throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find map: " << m_mapName; }

            mbapi::MapsManager::MapID minID = mpMgr.findID( m_minMapName );
            if ( UndefinedIDValue == minID ) { throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find map: " << m_minMapName; }

            mbapi::MapsManager::MapID maxID = mpMgr.findID( m_maxMapName );
            if ( UndefinedIDValue == maxID ) { throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find map: " << m_maxMapName; }

            // copy map and oweright it wieh interpolated between min/max maps values
            std::string newMapName = m_mapName + "_Case_" + ibs::to_string( caseID + 1 ) + "_RHPR";
            mbapi::MapsManager::MapID cmID = mpMgr.copyMap( bsID, newMapName );

            if ( UndefinedIDValue == cmID )
            {
               ErrorHandler::Exception( ErrorHandler::IoError ) << "Copy radiogenic heat production rate map " << m_mapName << " failed";
            }

            // value interval for maps range case is [-1:0:1] but min/max maps are set for [-1:0] or [0:1] interval
            if ( ErrorHandler::NoError != mpMgr.interpolateMap( cmID, minID, maxID, m_value < 0.0 ? m_value + 1.0 : m_value ) )
            {
               throw ErrorHandler::Exception( mpMgr.errorCode() ) << mpMgr.errorMessage();
            }
            
            // Save new map to file
            if ( ErrorHandler::NoError != mpMgr.saveMapToHDF( cmID, m_mapName + "_VarRHPR.HDF" ) )
            {
               throw ErrorHandler::Exception( caldModel.errorCode() ) << caldModel.errorMessage();
            }
           
            // Set this map to table
            if ( ErrorHandler::NoError != caldModel.setTableValue( s_basementTblName, 0, s_topCrustHeatProdGrid, newMapName ) )
            {
               throw ErrorHandler::Exception( caldModel.errorCode() ) << caldModel.errorMessage();
            }
         }

         if ( ErrorHandler::NoError != caldModel.setTableValue( s_basementTblName, 0, s_topCrustHeatProd, UndefinedDoubleValue ) )
         {
            throw ErrorHandler::Exception( caldModel.errorCode() ) << caldModel.errorMessage();
         }
      }
   }
   catch ( const ErrorHandler::Exception & e ) { return caldModel.reportError( e.errorCode(), e.what() ); }

   return ErrorHandler::NoError;
}

// Validate top crust heat production rate value if it is in positive range\n
// also it checks does the given model has the same value for this parameter.
std::string PrmTopCrustHeatProduction::validate( mbapi::Model & caldModel )
{
   std::ostringstream oss;

   if ( m_value < 0.0 )
   {
      oss << "Top crust heat production rate value  can not be negative: " << m_value << std::endl;
   }

   const std::string & modelName = caldModel.tableValueAsString( s_basementTblName, 0, s_bottomBoundaryModel );
   if ( ErrorHandler::NoError != caldModel.errorCode() )
   {
      oss << caldModel.errorMessage() << std::endl;
      return oss.str();
   }

   if ( modelName == "Fixed HeatFlow" )
   {
      oss << "Unsupported bottom boundary model: 'Fixed HeatFlow' for using top crust heat production rate." << std::endl;
      return oss.str(); // another model, no reason to check further
   }

   const std::string & heatProdMap = caldModel.tableValueAsString( s_basementTblName, 0, s_topCrustHeatProdGrid );
   if ( ErrorHandler::NoError != caldModel.errorCode() )
   {
      oss << caldModel.errorMessage() << std::endl;
      return oss.str();
   }

   if ( !heatProdMap.empty() )
   {
      oss << "Project has a map defined for top crust heat production rate.";
      oss << " Map and one rate value can not be used together" << std::endl;
      return oss.str();
   }
      
   double valInModel = caldModel.tableValueAsDouble( s_basementTblName, 0, s_topCrustHeatProd );

   if ( ErrorHandler::NoError != caldModel.errorCode() )
   {
      oss << caldModel.errorMessage() << std::endl;
      return oss.str();
   }

   if ( !NumericFunctions::isEqual( valInModel, m_value, 1e-6 ) )
   {
      oss << "Top crust heat production rate parameter value in the model (" << valInModel; 
      oss << ") is differ from a parameter value (" << m_value << ")" << std::endl;
   }

   return oss.str();
}


// Are two parameters equal?
bool PrmTopCrustHeatProduction::operator == ( const Parameter & prm ) const
{
   const PrmTopCrustHeatProduction * pp = dynamic_cast<const PrmTopCrustHeatProduction *>( &prm );
   if ( !pp ) return false;
   
   return NumericFunctions::isEqual( m_value, pp->m_value, 1.e-6 ) ? true : false;
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
