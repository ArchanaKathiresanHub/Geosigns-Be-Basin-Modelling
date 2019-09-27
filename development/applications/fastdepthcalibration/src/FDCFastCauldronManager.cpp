//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FDCFastCauldronManager.h"
#include "CommonDefinitions.h"

#include "Formation.h"
#include "Grid.h"
#include "GridMap.h"

namespace fastDepthCalibration
{

FDCFastCauldronManager::FDCFastCauldronManager(int argc,
                                               char** argv,
                                               const long scX,
                                               const long scY,
                                               const bool noExtrapolationFlag) :
  m_fastcauldronStartup{argc, argv},
  m_XScalingFactor{scX},
  m_YScalingFactor{scY},
  m_noExtrapolationFlag{noExtrapolationFlag}
{}

FDCFastCauldronManager::~FDCFastCauldronManager()
{}

void FDCFastCauldronManager::prepareAndRunFastCauldron()
{
  if (!m_fastcauldronStartup.getPrepareStatus()) { throw T2Zexception() << "Failed to prepare fastcauldron run "; }
  if (!m_fastcauldronStartup.getStartUpStatus()) { throw T2Zexception() << "Failed to startup fastcauldron "; }

  m_fastcauldronStartup.run();
  if (!m_fastcauldronStartup.getRunStatus()) { throw T2Zexception() << "Failed to run fastcauldron "; }
}

const DataAccess::Interface::PropertyValueList * FDCFastCauldronManager::getPropertyValues(const std::string & propertyName, const std::string & layerName) const
{
  FastcauldronSimulator & fastcauldronApp = FastcauldronSimulator::getInstance();
  const DataAccess::Interface::Property * propertyType = fastcauldronApp.findProperty(propertyName);
  const DataAccess::Interface::Formation * formation = fastcauldronApp.findFormation(layerName);
  const DataAccess::Interface::Snapshot * presentDaySnapshot = fastcauldronApp.findOrCreateSnapshot(0.0);
  if (presentDaySnapshot == nullptr) { throw T2Zexception() << "No present day snapshot found"; }
  return fastcauldronApp.getPropertyValues(Interface::FORMATION, propertyType, presentDaySnapshot, 0, formation, 0, VOLUME);
}

DataAccess::Interface::GridMap * FDCFastCauldronManager::getPropertyGridMap(const std::string & propertyName, const std::string & layerName)
{
  DataAccess::Interface::GridMap * propertyGridMap;

  FastcauldronSimulator & fastcauldronApp = FastcauldronSimulator::getInstance();
  std::unique_ptr<const DataAccess::Interface::PropertyValueList> propertyValues(getPropertyValues(propertyName, layerName));
  if (propertyValues->size() != 1) { throw T2Zexception() << "Invalid size of the property value"; }

  if ( m_XScalingFactor < 2 || m_YScalingFactor < 2)
  {
    propertyGridMap = propertyValues->at(0)->getGridMap();
    return propertyGridMap;
  }

  std::unique_ptr<const DataAccess::Interface::GridMap> propertyGridMapLowres(propertyValues->at(0)->getGridMap());
  propertyGridMap = fastcauldronApp.getFactory()->produceGridMap (nullptr,
                                                                      1,
                                                                      fastcauldronApp.getHighResolutionOutputGrid(),
                                                                      propertyGridMapLowres->getUndefinedValue(),
                                                                      propertyGridMapLowres->getDepth());

  if (!propertyGridMapLowres->transformLowRes2HighRes(propertyGridMap, !m_noExtrapolationFlag))
  {
    throw T2Zexception() << "Could not transform low res property gridmap to high res!";
  }

  return propertyGridMap;
}

void FDCFastCauldronManager::finalizeFastCauldronSturtup()
{
  m_fastcauldronStartup.finalize();
}

} // namespace fastDepthCalibration
