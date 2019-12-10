//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Genex0dProjectManager.h"

#include "CommonDefinitions.h"
#include "Genex0dDataExtractor.h"

// cmbAPI
#include "cmbAPI.h"
#include "ErrorHandler.h"

// DataAccess
#include "ProjectHandle.h"
#include "Snapshot.h"

//Genex6
#include "ConstantsGenex.h"

#include <cmath>

#include <iomanip>

namespace genex0d
{

namespace
{

const std::string s_dataMiningTblName = "DataMiningIoTbl";

} // namespace

Genex0dProjectManager::Genex0dProjectManager(const DataAccess::Interface::ProjectHandle & projectHandle, const std::string & projectFileName, const double xCoord,
                                             const double yCoord, const std::string & topSurfaceName, const std::string & formationName):
  m_projectFileName{projectFileName},
  m_projectHandle{projectHandle},
  m_xCoord{xCoord},
  m_yCoord{yCoord},
  m_mdl{nullptr},
  m_posData{0},
  m_posDataPrevious{0},
  m_propertyName{""},
  m_agesAll{},
  m_topSurfaceName{topSurfaceName},
  m_formationName{formationName}
{
  reloadModel();
  clearTable();
}

Genex0dProjectManager::~Genex0dProjectManager()
{
}

void Genex0dProjectManager::reloadModel()
{
  m_mdl.reset(new mbapi::Model());
  if (ErrorHandler::NoError != m_mdl->loadModelFromProjectFile(m_projectFileName.c_str()))
  {
    throw ErrorHandler::Exception(m_mdl->errorCode()) << "Could not initialize data mining table, " << m_mdl->errorMessage();
  }
}

void Genex0dProjectManager::clearTable()
{
  if (ErrorHandler::NoError != m_mdl->clearTable(s_dataMiningTblName))
  {
    throw ErrorHandler::Exception(m_mdl->errorCode()) << "Could not clear data mining table, " << m_mdl->errorMessage();
  }
}

void Genex0dProjectManager::computeAgesFromAllSnapShots(const double depositionTimeTopSurface)
{
  DataAccess::Interface::SnapshotList * snapshots = m_projectHandle.getSnapshots(DataAccess::Interface::MINOR | DataAccess::Interface::MAJOR);
  DataAccess::Interface::SnapshotList::reverse_iterator snapshotIter;

  if (snapshots->size() < 1)
  {
    return;
  }

  for (snapshotIter = snapshots->rbegin(); snapshotIter != snapshots->rend() - 1; ++ snapshotIter)
  {
    if (depositionTimeTopSurface < (*snapshotIter)->getTime())
    {
      continue;
    }
    m_agesAll.push_back((*snapshotIter)->getTime());
  }
}

void Genex0dProjectManager::getValues(std::vector<double> & values) const
{
  for (int i = m_posDataPrevious; i < m_posData; ++i)
  {
    values[i-m_posDataPrevious] = m_mdl->tableValueAsDouble(s_dataMiningTblName, i, "Value");
  }
}

void Genex0dProjectManager::requestPropertyInSnapshots()
{
  mbapi::PropertyManager & propMgr = m_mdl->propertyManager();

  if (ErrorHandler::NoError != propMgr.requestPropertyInSnapshots(m_propertyName))
  {
    throw ErrorHandler::Exception(propMgr.errorCode()) << "Could not initiate extracting property form mdl! "
                                                       << ", " << propMgr.errorMessage();
  }
}

void Genex0dProjectManager::saveModel()
{
  m_mdl->saveModelToProjectFile(m_projectFileName.c_str());
}

void Genex0dProjectManager::setInTable()
{
  m_posDataPrevious = m_posData;
  for (auto simTime : m_agesAll)
  {
    if (ErrorHandler::NoError != m_mdl->addRowToTable(s_dataMiningTblName) ||
        ErrorHandler::NoError != m_mdl->setTableValue(s_dataMiningTblName, m_posData, "Time", simTime) ||
        ErrorHandler::NoError != m_mdl->setTableValue(s_dataMiningTblName, m_posData, "XCoord", m_xCoord) ||
        ErrorHandler::NoError != m_mdl->setTableValue(s_dataMiningTblName, m_posData, "YCoord", m_yCoord) ||
        ErrorHandler::NoError != m_mdl->setTableValue(s_dataMiningTblName, m_posData, "ZCoord",
                                                      Utilities::Numerical::IbsNoDataValue) ||
        ErrorHandler::NoError != m_mdl->setTableValue(s_dataMiningTblName, m_posData, "PropertyName", m_propertyName) ||
        ErrorHandler::NoError != m_mdl->setTableValue(s_dataMiningTblName, m_posData, "Value",
                                                      Utilities::Numerical::IbsNoDataValue) ||
        ErrorHandler::NoError != m_mdl->setTableValue(s_dataMiningTblName, m_posData, "SurfaceName", m_topSurfaceName) ||
        ErrorHandler::NoError != m_mdl->setTableValue(s_dataMiningTblName, m_posData, "FormationName", m_formationName)
        )
    {
      throw ErrorHandler::Exception(m_mdl->errorCode()) << "Could not initialize data mining table! "
                                                        << "Error code: " << m_mdl->errorMessage();
    }
    ++m_posData;
  }
}

std::vector<double> Genex0dProjectManager::agesAll() const
{
  return m_agesAll;
}

void Genex0dProjectManager::setTopSurface(const std::string & topSurfaceName)
{
  m_topSurfaceName = topSurfaceName;
}

std::vector<double> Genex0dProjectManager::requestPropertyHistory(const std::string & propertyName)
{
  m_propertyName = propertyName;
  requestPropertyInSnapshots();
  setInTable();
  saveModel();

  genex0d::Genex0dDataExtractor::run(m_projectFileName);
  reloadModel();

  std::vector<double> values((m_posData - m_posDataPrevious), 0.0);
  getValues(values);
  return values;
}

} // namespace genex0d
