//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ModelPseudo1d.h"

#include "ModelPseudo1dCommonDefinitions.h"
#include "ModelPseudo1dInputData.h"
#include "ModelPseudo1dInputMapProperties.h"

#include "cmbAPI.h"
#include "ErrorHandler.h"

#include "LogHandler.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace modelPseudo1d
{

ModelPseudo1d::ModelPseudo1d( mbapi::Model& model, const double xCoord , const double yCoord ):
  m_mdl{model},
  m_xCoord{xCoord},
  m_yCoord{yCoord},
  m_indI{0},
  m_indJ{0},
  m_tablePropertyMapsLists{}
{
}

void ModelPseudo1d::initialize()
{
  LogHandler(LogHandler::INFO_SEVERITY) << "Initializing ModelPseudo1d...";

  checkSubSampling();
  getXYIndices();
  setReferredTablesSet();
  LogHandler(LogHandler::INFO_SEVERITY) << "Initialization Done! Starting the data extraction from the input maps...";
}

void ModelPseudo1d::checkSubSampling()
{
  long dXSubsampling = 1;
  long dYSubsampling = 1;
  if (ErrorHandler::NoError != m_mdl.subsampling(dXSubsampling, dYSubsampling))
  {
    throw ErrorHandler::Exception(m_mdl.errorCode())
        << "Could not get subsampling values.  " << m_mdl.errorMessage();
  }

  if (dXSubsampling == 1 && dYSubsampling == 1 )
  {
    return;
  }

  LogHandler(LogHandler::WARNING_SEVERITY) << "\n The model is subsampled! Subsampling values will be reset to 1.0. \n";
  if (ErrorHandler::NoError != m_mdl.setSubsampling( 1, 1 ))
  {
    throw  ErrorHandler::Exception(m_mdl.errorCode()) << "Resetting of the subsampling was not successful, " << m_mdl.errorMessage();
  }
}

void ModelPseudo1d::getXYIndices()
{
  int    minI, maxI, minJ, maxJ;
  double centreX, centreY;

  if (ErrorHandler::NoError != m_mdl.windowSize( m_xCoord, m_yCoord, minI, maxI, minJ, maxJ, centreX, centreY ))
  {
    double originX = 0.0;
    double originY = 0.0;
    m_mdl.origin(originX, originY);
    double lengthX = 0.0;
    double lengthY = 0.0;
    m_mdl.arealSize(lengthX, lengthY);
    throw modelPseudo1d::ModelPseudo1dException() << "The input (X,Y) location is out of range! \n"
                                                  << "Valid X ranges: (" << originX << ", " << originX + lengthX << ") \n"
                                                  << "Valid Y ranges: (" << originY << ", " << originY + lengthY << ") \n";
  }

  m_indI = minI;
  m_indJ = minJ;
}

void ModelPseudo1d::setReferredTablesSet()
{
  for (int row = 0; row < m_mdl.tableSize("GridMapIoTbl"); ++row)
  {
    m_referredTablesSet.insert(m_mdl.tableValueAsString("GridMapIoTbl", row, "ReferredBy"));
  }
}

double ModelPseudo1d::extractScalarFromInputMaps(const ModelPseudo1dTableProperty & tableDefaultProperty, const std::string & tableName, const int row, const MapIDsHashTable& mapNameIDs)
{                                           
  mbapi::MapsManager & mapsMgr = m_mdl.mapsManager();
  mbapi::PropertyManager & propMgr = m_mdl.propertyManager();

  if (ErrorHandler::NoError != propMgr.requestPropertyInSnapshots(tableDefaultProperty.name))
  {
    LogHandler(LogHandler::WARNING_SEVERITY) << "Warning! Initialization of table property " << tableDefaultProperty.name << " failed!";
    return DataAccess::Interface::DefaultUndefinedScalarValue;
  }

  const std::string & myMapName = m_mdl.tableValueAsString(tableName, row, tableDefaultProperty.nameGridMap);
  if (myMapName.empty())
  {
    return DataAccess::Interface::DefaultUndefinedScalarValue;
  }

  return mapsMgr.mapGetValue(mapNameIDs.at(myMapName), m_indI, m_indJ);
}

void ModelPseudo1d::extractScalarsFromInputMaps()
{
  mbapi::MapsManager & mapsMgr = m_mdl.mapsManager();
  const MapIDsHashTable & mapNameIDs = mapsMgr.mapNameIDs();

  for ( auto tableName : m_referredTablesSet )
  {
    const std::vector<ModelPseudo1dTableProperty> & tableDefaultProperties =
        ModelPseudo1dInputMapProperties::getInstance().tableMapPropertyNames(tableName);
    if (tableDefaultProperties.empty())
    {
      LogHandler(LogHandler::WARNING_SEVERITY) << "Warning! Table name " << tableName << " not found.";
      continue;
    }

    const int tableSize = m_mdl.tableSize(tableName);
    TablePropertyMapList tablePropertyValuesMap;
    for (const ModelPseudo1dTableProperty & tableDefaultProperty : tableDefaultProperties)
    {
      LogHandler(LogHandler::INFO_SEVERITY) << " - Extracting scalars for " << tableDefaultProperty.name
                                            << " in " << tableName;

      for (int row = 0; row < tableSize; ++row)
      {
        ModelPseudo1dTableProperty tableProperty = tableDefaultProperty;
        const double value = extractScalarFromInputMaps(tableDefaultProperty, tableName, row, mapNameIDs);
        if (value == DataAccess::Interface::DefaultUndefinedScalarValue)
        {
          continue;
        }

        tableProperty.value = value;
        tablePropertyValuesMap[tableName].push_back(make_pair(row, tableProperty));
      }
    }
    m_tablePropertyMapsLists.push_back(tablePropertyValuesMap);
  }

  LogHandler(LogHandler::INFO_SEVERITY) << "Data extraction step completed successfully!";
}

void ModelPseudo1d::setScalarsInModel()
{
  LogHandler(LogHandler::INFO_SEVERITY) << "Setting extracted values in model ...";
  for (const TablePropertyMapList & tablePropertyValuesMap : m_tablePropertyMapsLists)
  {
    TablePropertyMapList::const_iterator tablePropertyIterator = tablePropertyValuesMap.begin();
    for ( ; tablePropertyIterator != tablePropertyValuesMap.end(); ++tablePropertyIterator)
    {
      for (const PairModelPseudo1dTableProperty & tablePropertyPair : tablePropertyIterator->second)
      {
        const ModelPseudo1dTableProperty & tableProperty = tablePropertyPair.second;
        if (ErrorHandler::NoError != m_mdl.setTableValue(tablePropertyIterator->first, tablePropertyPair.first, tableProperty.name, tableProperty.value))
        {
          throw ErrorHandler::Exception(m_mdl.errorCode())
              << "Table " << tablePropertyIterator->first << " was not updated at row " << tablePropertyPair.first
              << " for table property " << tableProperty.name << " with scalar value " << tableProperty.value << ", " << m_mdl.errorMessage();
        }

        if (ErrorHandler::NoError != m_mdl.setTableValue(tablePropertyIterator->first, tablePropertyPair.first, tableProperty.nameGridMap, ""))
        {
          throw ErrorHandler::Exception(m_mdl.errorCode())
              << "Failed at removing map name in table " << tablePropertyIterator->first << " at row " << tablePropertyPair.first
              << " for table property " << tableProperty.name << ", " << m_mdl.errorMessage();
        }
      }
    }
  }
  LogHandler(LogHandler::INFO_SEVERITY) << "Successfully updated model!";
}

void ModelPseudo1d::setSingleCellWindowXY()
{
  if (ErrorHandler::NoError != m_mdl.setWindow(m_indI, m_indI + 1, m_indJ, m_indJ + 1)) // TODO: this is 2X2 grid, to be fixed to 1X1 grid after fastcauldron has capability to runs with 1X1 grid.
  {
    throw ErrorHandler::Exception(m_mdl.errorCode())
        << "Windowing around the specified location failed, " << m_mdl.errorMessage();
  }
}

unsigned int ModelPseudo1d::indI() const
{
  return m_indI;
}

unsigned int ModelPseudo1d::indJ() const
{
  return m_indJ;
}
} // namespace modelPseudo1d
