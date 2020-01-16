//
// Copyright (C) 2019 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file PrmTopCrustHeatProductionGridScaling.cpp
/// @brief Implementation for the parameter for scaling the grid

#include "PrmTopCrustHeatProductionGridScaling.h"

#include "cmbAPI.h"
#include "NumericFunctions.h"

namespace casa
{

namespace
{
  const std::string tableName = "BasementIoTbl";
  const std::string prodColumn = "TopCrustHeatProd";
  const std::string mapColumn = "TopCrustHeatProdGrid";
  const std::string mapNameSuffix = "_RHPR";
  const std::string mapFileSuffix = "_VarRHPR.HDF";
}

PrmTopCrustHeatProductionGridScaling::PrmTopCrustHeatProductionGridScaling(mbapi::Model& mdl, const std::vector<std::string>& parameters)
  : Parameter(),
    m_factor{1}
{
  try
  {
    const double production = mdl.tableValueAsDouble(tableName, 0, prodColumn);

    if ( !IsValueUndefined(production) )
    {
      throw ErrorHandler::Exception ( ErrorHandler::ValidationError )
          << tableName << ":" << prodColumn << " was defined in this model. Grid scaling is not possible";
    }

    const std::string mapName = mdl.tableValueAsString(tableName, 0, mapColumn);
    if ( mapName.empty() )
    {
      throw ErrorHandler::Exception ( ErrorHandler::ValidationError )
          << "No mapname defined in " << tableName << ":" << mapColumn;
    }

    mbapi::MapsManager& mapManager = mdl.mapsManager();
    mbapi::MapsManager::MapID mapID = mapManager.findID( mapName );
    if ( IsValueUndefined( mapID ) )
    {
      throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find the map: " << mapName
                                                                   << " defined for top crust heat production rate in maps catalog";
    }

    double minVal, maxVal;
    if ( ErrorHandler::NoError != mapManager.mapValuesRange( mapID, minVal, maxVal ) )
    {
       throw ErrorHandler::Exception( mapManager.errorCode() ) << mapManager.errorMessage();
    }

    m_mapName = mapName;
  }
  catch ( const ErrorHandler::Exception& e)
  {
    mdl.reportError( e.errorCode(), e.what() );
  }
}

PrmTopCrustHeatProductionGridScaling::PrmTopCrustHeatProductionGridScaling(const VarParameter* parent, const PrmTopCrustHeatProductionGridScaling& param, const double val)
  : PrmTopCrustHeatProductionGridScaling(param)
{
  setParent(parent);
  m_factor = val;
}

PrmTopCrustHeatProductionGridScaling::~PrmTopCrustHeatProductionGridScaling()
{
}

ErrorHandler::ReturnCode PrmTopCrustHeatProductionGridScaling::setInModel(mbapi::Model& caldModel, size_t caseID)
{
  mbapi::MapsManager & mpMgr = caldModel.mapsManager();
  mbapi::MapsManager::MapID mID = mpMgr.findID( m_mapName );

  if ( IsValueUndefined( mID ) )
  {
    throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find the map: " << m_mapName
                                                                 << " defined for top crust heat production rate in maps catalog";
  }

  // copy map to avoid influence on other project parts
  std::string newMapName = m_mapName + "_Case_" + std::to_string( caseID + 1 ) + mapNameSuffix;
  mbapi::MapsManager::MapID cmID = mpMgr.copyMap( mID, newMapName );
  if ( IsValueUndefined( cmID ) )
  {
    throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Copy radiogenic heat production rate map " << m_mapName << " failed";
  }

  // extract min/max values from the map
  double minVal, maxVal;

  if ( ErrorHandler::NoError != mpMgr.mapValuesRange( mID, minVal, maxVal ) )
  {
    throw ErrorHandler::Exception( mpMgr.errorCode() ) << mpMgr.errorMessage();
  }

  //double scaleCoeff = NumericFunctions::isEqual( 0.0, maxVal, 1e-10 ) ? 0.0 : ( m_value / maxVal );

  // scale map with new maximum value
  if ( ErrorHandler::NoError != mpMgr.scaleMap( cmID, m_factor ) )
  {
    throw ErrorHandler::Exception( mpMgr.errorCode() ) << mpMgr.errorMessage();
  }

  // save map to separate HDF file
  if ( ErrorHandler::NoError != mpMgr.saveMapToHDF( cmID, m_mapName + mapFileSuffix ) )
  {
    throw ErrorHandler::Exception( mpMgr.errorCode() ) << mpMgr.errorMessage();
  }

  // update project with new map name
  if ( ErrorHandler::NoError != caldModel.setTableValue( tableName, 0, mapColumn, newMapName ) )
  {
    throw ErrorHandler::Exception( mpMgr.errorCode() ) << mpMgr.errorMessage();
  }

  return ErrorHandler::NoError;
}

std::string PrmTopCrustHeatProductionGridScaling::validate(mbapi::Model& caldModel)
{
  std::ostringstream oss;

  const double productionRate = caldModel.tableValueAsDouble(tableName, 0, prodColumn);

  if ( !IsValueUndefined(productionRate) )
  {
    oss << tableName << ":" << prodColumn << " was defined in this model. Grid scaling is not possible";
  }

  const std::string mapName = caldModel.tableValueAsString(tableName, 0, mapColumn);
  if ( mapName.empty() )
  {
    oss << "No mapname defined in " << tableName << ":" << mapColumn;
  }

  mbapi::MapsManager& mapManager = caldModel.mapsManager();
  mbapi::MapsManager::MapID mapID = mapManager.findID( mapName );
  if ( IsValueUndefined( mapID ) )
  {
    oss << "Can't find the map: " << mapName
        << " defined for top crust heat production rate in maps catalog";
  }

  return oss.str();
}

int PrmTopCrustHeatProductionGridScaling::asInteger() const
{
   assert(0);
   return Utilities::Numerical::NoDataIntValue;
}

bool PrmTopCrustHeatProductionGridScaling::operator ==(const Parameter& prm) const
{
  const PrmTopCrustHeatProductionGridScaling* pp = dynamic_cast<const PrmTopCrustHeatProductionGridScaling*>(&prm);
  if (!pp)
  {
    return false;
  }

  return NumericFunctions::isEqual(m_factor, pp->m_factor, 1e-6);
}

bool PrmTopCrustHeatProductionGridScaling::save(CasaSerializer& sz) const
{
  bool ok = saveCommonPart(sz);
  ok = ok && sz.save( m_factor, "HeatGridFactor");
  ok = ok && sz.save( m_mapName, "HeatGridMapName");
  return ok;
}

PrmTopCrustHeatProductionGridScaling::PrmTopCrustHeatProductionGridScaling(CasaDeserializer& dz, unsigned int objVer)
  : Parameter(dz, objVer)
{
  bool ok = dz.load(m_factor, "HeatGridFactor");
  ok = ok && dz.load(m_mapName, "HeatGridMapName");

  if ( !ok )
  {
     throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
        << "PrmTopCrustHeatProductionGridScaling deserialization unknown error";
  }
}

void PrmTopCrustHeatProductionGridScaling::loadVarPrm(CasaDeserializer& dz, const unsigned int objVer)
{
}

std::size_t PrmTopCrustHeatProductionGridScaling::expectedParametersNumber()
{
  return 0;
}

std::size_t PrmTopCrustHeatProductionGridScaling::optionalParametersNumber()
{
  return 0;
}

std::string PrmTopCrustHeatProductionGridScaling::key()
{
  return "TopCrustHeatProductionGridScaling";
}

std::string PrmTopCrustHeatProductionGridScaling::varprmName()
{
  return "VarPrmTopCrustHeatProductionGridScaling";
}

std::string PrmTopCrustHeatProductionGridScaling::description()
{
  return "Scaling factor for the map of the surface radiogenic heat production rate of the basement";
}

std::string PrmTopCrustHeatProductionGridScaling::fullDescription()
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

std::string PrmTopCrustHeatProductionGridScaling::helpExample(const char* cmdName)
{
  std::ostringstream oss;
  oss << "    #                                      type               minVal  maxVal prmPDF\n";
  oss << "    " << cmdName << " \"Radiogenic heat rate\"  \"" << key() << "\"    0.5     1.5  \"Block\"\n";
  oss << "\n";
  return oss.str();
}

}
