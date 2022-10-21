//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file PrmTopCrustHeatProductionGrid.h
/// @brief This file keeps API implementation for handling Top Crust Heat Production Rate basement parameter.

// CASA API
#include "PrmTopCrustHeatProductionGrid.h"

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
PrmTopCrustHeatProductionGrid::PrmTopCrustHeatProductionGrid( const VarParameter * parent,
                                                              const PrmTopCrustHeatProductionGrid& param,
                                                              double value) :
  PrmTopCrustHeatProductionGrid( param )
{
  setParent( parent );
  m_value = value;

  if (value == 1.0)
  {
    m_mapName = param.m_maxMapName;
    m_minMapName = "";
    m_maxMapName = "";
  }
  else if (value == -1.0)
  {
    m_mapName = param.m_minMapName;
    m_minMapName = "";
    m_maxMapName = "";
  }
  else if (value > 0.0 && value < 1.0)
  {
    m_minMapName = param.m_mapName;
  }
  else if (value < 0.0 && value > -1.0)
  {
    m_maxMapName = param.m_mapName;
  }
}

// This constructor extracts top crust production rate defined as a map, constructor extracts map maximum value and use it to
// initialize class with this value and stores map name also.
PrmTopCrustHeatProductionGrid::PrmTopCrustHeatProductionGrid( mbapi::Model & mdl, const std::vector<std::string>& /*parameters*/ ) : Parameter( )
{
  try
  {
    const std::string mapName = mdl.tableValueAsString( s_basementTblName, 0, s_topCrustHeatProdGrid );
    if ( ErrorHandler::NoError != mdl.errorCode() )
    {
      throw ErrorHandler::Exception( mdl.errorCode() ) << mdl.errorMessage();
    }

    if ( mapName.empty() )
    {
      throw ErrorHandler::Exception( ErrorHandler::ValidationError ) << "No map name provided for using with top crust heat production rate.";
    }

    mbapi::MapsManager & mpMgr = mdl.mapsManager();
    mbapi::MapsManager::MapID mID = mpMgr.findID( mapName );

    if ( Utilities::isValueUndefined( mID ) )
    {
      throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find the map: " << mapName
                                                                   << " defined for top crust heat production rate in maps catalog";
    }

    double minVal, maxVal;
    if ( ErrorHandler::NoError != mpMgr.mapValuesRange( mID, minVal, maxVal ) )
    {
      throw ErrorHandler::Exception( mpMgr.errorCode() ) << mpMgr.errorMessage();
    }

    m_value = 0.0;// maxVal;
    m_mapName = mapName;
  }
  catch ( const ErrorHandler::Exception& e)
  {
    mdl.reportError( e.errorCode(), e.what() );
  }
}

// Destructor
PrmTopCrustHeatProductionGrid::~PrmTopCrustHeatProductionGrid()
{
}

// Update given model with the parameter value
ErrorHandler::ReturnCode PrmTopCrustHeatProductionGrid::setInModel( mbapi::Model & cldModel, size_t caseID )
{
  try
  {
    if ( ErrorHandler::NoError != cldModel.errorCode() )
    {
      throw ErrorHandler::Exception( cldModel.errorCode() ) << cldModel.errorMessage();
    }

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
      if ( Utilities::isValueUndefined( bsID ) ) { throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find map: " << m_mapName; }

      mbapi::MapsManager::MapID minID = mpMgr.findID( m_minMapName );
      if ( Utilities::isValueUndefined( minID ) ) { throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find map: " << m_minMapName; }

      mbapi::MapsManager::MapID maxID = mpMgr.findID( m_maxMapName );
      if ( Utilities::isValueUndefined( maxID ) ) { throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find map: " << m_maxMapName; }

      // copy map and overwrite it when interpolating between min/max maps values
      std::string newMapName = m_mapName + "_Case_" + std::to_string( caseID + 1 ) + s_mapNameSuffix;
      mbapi::MapsManager::MapID cmID = mpMgr.copyMap( bsID, newMapName );

      if ( Utilities::isValueUndefined( cmID ) )
      {
        throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Copy radiogenic heat production rate map " << m_mapName << " failed";
      }

      // value interval for maps range case is [-1:0:1] but min/max maps are set for [-1:0] or [0:1] interval
      if ( ErrorHandler::NoError != mpMgr.interpolateMap( cmID, minID, maxID, m_value < 0.0 ? m_value + 1.0 : m_value ) )
      {
        throw ErrorHandler::Exception( mpMgr.errorCode() ) << mpMgr.errorMessage();
      }

      // Save new map to file
      if ( ErrorHandler::NoError != mpMgr.saveMapToHDF( cmID, m_mapName + s_mapFileSuffix ) )
      {
        throw ErrorHandler::Exception( cldModel.errorCode() ) << cldModel.errorMessage();
      }

      // Set this map to table
      if ( ErrorHandler::NoError != cldModel.setTableValue( s_basementTblName, 0, s_topCrustHeatProdGrid, newMapName ) )
      {
        throw ErrorHandler::Exception( cldModel.errorCode() ) << cldModel.errorMessage();
      }
    }

    if ( ErrorHandler::NoError != cldModel.setTableValue( s_basementTblName, 0, s_topCrustHeatProd, Utilities::Numerical::IbsNoDataValue ) )
    {
      throw ErrorHandler::Exception( cldModel.errorCode() ) << cldModel.errorMessage();
    }

  }
  catch ( const ErrorHandler::Exception & e )
  {
    return cldModel.reportError( e.errorCode(), e.what() );
  }
  return ErrorHandler::NoError;
}

// Validate top crust heat production rate value if it is in positive range\n
// also it checks does the given model has the same value for this parameter.
std::string PrmTopCrustHeatProductionGrid::validate( mbapi::Model & cldModel )
{
  std::ostringstream oss;

  // get value from the project
  const std::string & heatProdMap = cldModel.tableValueAsString( s_basementTblName, 0, s_topCrustHeatProdGrid );
  if ( ErrorHandler::NoError != cldModel.errorCode() )
  {
    oss << cldModel.errorMessage() << std::endl;
    return oss.str();
  }

  if ( ErrorHandler::NoError != cldModel.errorCode() )
  {
    oss << cldModel.errorMessage() << std::endl;
    return oss.str();
  }


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
    if ( Utilities::isValueUndefined( minID ) ) { throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find map: " << m_minMapName; }
    mbapi::MapsManager::MapID maxID = mpMgr.findID( m_maxMapName );
    if ( Utilities::isValueUndefined( maxID ) ) { throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find map: " << m_maxMapName; }
    mbapi::MapsManager::MapID mID = mpMgr.findID( heatProdMap ); // get map
    if ( Utilities::isValueUndefined( mID   ) ) { throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find map: " << heatProdMap; }

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
  return oss.str();
}


// Are two parameters equal?
bool PrmTopCrustHeatProductionGrid::operator == ( const Parameter & prm ) const
{
   const PrmTopCrustHeatProductionGrid * pp = dynamic_cast<const PrmTopCrustHeatProductionGrid *>( &prm );
   if ( !pp ) return false;

   if ( !NumericFunctions::isEqual( m_value, pp->m_value, 1.e-6 ) ) return false;
   if ( m_mapName != pp->m_mapName                                ) return false;

   return true;
}


// Save all object data to the given stream, that object could be later reconstructed from saved data
bool PrmTopCrustHeatProductionGrid::save( CasaSerializer & sz ) const
{
   bool hasParent = parent();
   bool ok = sz.save( hasParent, "hasParent" );

   if ( hasParent )
   {
      CasaSerializer::ObjRefID parentID = sz.ptr2id( parent() );
      ok = ok ? sz.save( parentID, "VarParameterID" ) : ok;
   }
   ok = ok ? sz.save( m_value, "heatProdRateValue" ) : ok;

   ok = ok ? sz.save( m_mapName,    "mapName"    ) : ok;
   ok = ok ? sz.save( m_minMapName, "minMapName" ) : ok;
   ok = ok ? sz.save( m_maxMapName, "maxMapName" ) : ok;
   return ok;
}

// Create a new var.parameter instance by deserializing it from the given stream
PrmTopCrustHeatProductionGrid::PrmTopCrustHeatProductionGrid( CasaDeserializer & dz, unsigned int objVer )
{
   CasaDeserializer::ObjRefID parentID;

   bool hasParent;
   bool ok = dz.load( hasParent, "hasParent" );

   if ( hasParent )
   {
      bool ok = dz.load( parentID, "VarParameterID" );
      setParent( ok ? dz.id2ptr<VarParameter>( parentID ) : 0 );
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
         << "PrmTopCrustHeatProductionGrid deserialization unknown error";
   }
}

void PrmTopCrustHeatProductionGrid::loadVarPrm(CasaDeserializer& dz, const unsigned int objVer)
{
}

std::size_t PrmTopCrustHeatProductionGrid::expectedParametersNumber()
{
  return 0;
}

std::size_t PrmTopCrustHeatProductionGrid::optionalParametersNumber()
{
  return 0;
}

std::string PrmTopCrustHeatProductionGrid::key()
{
  return "BasementIoTbl:TopCrustHeatProdGrid";
}

std::string PrmTopCrustHeatProductionGrid::varprmName()
{
  return "VarPrmTopCrustHeatProductionGrid";
}

std::string PrmTopCrustHeatProductionGrid::description()
{
  return "surface radiogenic heat production rate of the basement [ uW/m^3]";
}

std::string PrmTopCrustHeatProductionGrid::fullDescription()
{
  std::ostringstream oss;

  oss << "    \"" << key() << "\" <minVal> <maxVal> <prmPDF>\n";
  oss << "    Where:\n";
  oss << "       minVal     - the parameter minimal range scale factor value\n";
  oss << "       maxVal     - the parameter maximal range scale factor value\n";
  oss << "       prmPDF     - the parameter probability density function type, the value could be one of the following:\n";
  oss << "                \"Block\"    - uniform probability between min and max values,\n";
  oss << "                \"Triangle\" - triangle shape probability function. The top triangle value is taken from the base case\n";
  oss << "                \"Normal\"   - normal (or Gaussian) probability function. The position of highest value is taken from the base case\n";

  return oss.str();
}

std::string PrmTopCrustHeatProductionGrid::helpExample(const char* cmdName)
{
  std::ostringstream oss;
  oss << "    #                                      type               minVal  maxVal prmPDF\n";
  oss << "    " << cmdName << " \"Radiogenic heat rate\"  \"" << key() << "Grid\"  \"MinMapName\" \"MaxMapName\"  \"Block\"\n";
  oss << "\n";
  return oss.str();
}



}
