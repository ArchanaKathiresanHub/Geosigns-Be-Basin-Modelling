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

ModelPseudo1d::ModelPseudo1d(const ModelPseudo1dInputData & inputData) :
  m_projectMgr{inputData},
  m_mdl{nullptr},
  m_tablePropertyMapsLists{},
  m_outProjectFileName{inputData.outProjectFilename}
{
  m_mdl = m_projectMgr.mdl();
}

void ModelPseudo1d::initialize()
{
  LogHandler(LogHandler::INFO_SEVERITY) << "Initializing ModelPseudo1d...";

  m_projectMgr.checkSubSampling();
  m_projectMgr.getXYIndices();
  m_projectMgr.setReferredTablesSet();
  LogHandler(LogHandler::INFO_SEVERITY) << "Initialization Done! Starting the data extraction from the input maps...";
}

double ModelPseudo1d::extractScalarFromInputMaps(const ModelPseudo1dTableProperty & tableDefaultProperty, const std::string & tableName, const int row, const MapIDsHashTable& mapNameIDs)
{                                           
  mbapi::MapsManager & mapsMgr = m_mdl->mapsManager();
  mbapi::PropertyManager & propMgr = m_mdl->propertyManager();

  if (ErrorHandler::NoError != propMgr.requestPropertyInSnapshots(tableDefaultProperty.name))
  {
    LogHandler(LogHandler::WARNING_SEVERITY) << "Warning! Initialization of table property " << tableDefaultProperty.name << " failed!";
    return DataAccess::Interface::DefaultUndefinedScalarValue;
  }

  const std::string & myMapName = m_mdl->tableValueAsString(tableName, row, tableDefaultProperty.nameGridMap);
  if (myMapName.empty())
  {
    return DataAccess::Interface::DefaultUndefinedScalarValue;
  }

  return mapsMgr.mapGetValue(mapNameIDs.at(myMapName), m_projectMgr.indI(), m_projectMgr.indJ());
}

void ModelPseudo1d::extractScalarsFromInputMaps()
{
  mbapi::MapsManager & mapsMgr = m_mdl->mapsManager();
  const MapIDsHashTable & mapNameIDs = mapsMgr.mapNameIDs();

  ModelPseudo1dProjectManager::ConstIteratorReferredTableSet tableNameIterator = m_projectMgr.referredTableConstIteratorBegin();
  for ( ; tableNameIterator != m_projectMgr.referredTableConstIteratorend(); ++tableNameIterator)
  {
    const std::vector<ModelPseudo1dTableProperty> & tableDefaultProperties =
        ModelPseudo1dInputMapProperties::getInstance().tableMapPropertyNames(*tableNameIterator);
    if (tableDefaultProperties.empty())
    {
      LogHandler(LogHandler::WARNING_SEVERITY) << "Warning! Table name " << *tableNameIterator << " not found.";
      continue;
    }

    const int tableSize = m_mdl->tableSize(*tableNameIterator);
    TablePropertyMapList tablePropertyValuesMap;
    for (const ModelPseudo1dTableProperty & tableDefaultProperty : tableDefaultProperties)
    {
      LogHandler(LogHandler::INFO_SEVERITY) << " - Extracting scalars for " << tableDefaultProperty.name
                                            << " in " << *tableNameIterator;

      for (int row = 0; row < tableSize; ++row)
      {
        ModelPseudo1dTableProperty tableProperty = tableDefaultProperty;
        const double value = extractScalarFromInputMaps(tableDefaultProperty, *tableNameIterator, row, mapNameIDs);
        if (value == DataAccess::Interface::DefaultUndefinedScalarValue)
        {
          continue;
        }

        tableProperty.value = value;
        tablePropertyValuesMap[*tableNameIterator].push_back(make_pair(row, tableProperty));
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
        if (ErrorHandler::NoError != m_mdl->setTableValue(tablePropertyIterator->first, tablePropertyPair.first, tableProperty.name, tableProperty.value))
        {
          throw ErrorHandler::Exception(m_mdl->errorCode())
              << "Table " << tablePropertyIterator->first << " was not updated at row " << tablePropertyPair.first
              << " for table property " << tableProperty.name << " with scalar value " << tableProperty.value << ", " << m_mdl->errorMessage();
        }

        if (ErrorHandler::NoError != m_mdl->setTableValue(tablePropertyIterator->first, tablePropertyPair.first, tableProperty.nameGridMap, ""))
        {
          throw ErrorHandler::Exception(m_mdl->errorCode())
              << "Failed at removing map name in table " << tablePropertyIterator->first << " at row " << tablePropertyPair.first
              << " for table property " << tableProperty.name << ", " << m_mdl->errorMessage();
        }
      }
    }
  }
  LogHandler(LogHandler::INFO_SEVERITY) << "Successfully updated model!";
}

void ModelPseudo1d::finalize()
{
  LogHandler(LogHandler::INFO_SEVERITY) << "Finalizing and saving to output file ...";
  m_projectMgr.setSingleCellWindowXY();
  m_mdl->saveModelToProjectFile(m_outProjectFileName.c_str());
  LogHandler(LogHandler::INFO_SEVERITY) << "ModelPseudo1d was successfully saved to output project file: "
                                        << m_outProjectFileName;
}

} // namespace modelPseudo1d
