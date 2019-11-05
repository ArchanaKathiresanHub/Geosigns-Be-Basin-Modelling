//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FinalCaseManager.h"
#include "CommonDefinitions.h"
#include "CaseStorageManager.h"
#include "FDCMapFieldProperties.h"
#include "FDCProjectManager.h"

#include "LogHandler.h"

#include "propinterface.h"

#include <algorithm>

namespace fastDepthCalibration
{

FinalCaseManager::FinalCaseManager(FDCProjectManager  & fdcProjectManager,
                                   FDCMapFieldProperties & fdcMapFieldProperties,
                                   CaseStorageManager & caseStorageManager) :
  m_fdcProjectManager{fdcProjectManager},
  m_fdcMapFieldProperties{fdcMapFieldProperties},
  m_caseStorageManager{caseStorageManager},
  m_rank{0}
{
  MPI_Comm_rank( PETSC_COMM_WORLD, &m_rank );
}

FinalCaseManager::~FinalCaseManager()
{}

std::vector<mbapi::StratigraphyManager::SurfaceID> FinalCaseManager::getSurfacesID() const
{
  std::shared_ptr<mbapi::Model> mdl = m_fdcProjectManager.getModel();
  const mbapi::StratigraphyManager & strMgrMaster = mdl->stratigraphyManager();
  return strMgrMaster.surfacesIDs();
}

void FinalCaseManager::runModifyTables(const bool noCalculatedTWToutput, const std::vector<mbapi::StratigraphyManager::SurfaceID> & surfacesIDs)
{
  try
  {
    m_fdcProjectManager.modifyTables(getSurfacesID().size() - 1, m_fdcMapFieldProperties.hiatusAll(), m_fdcMapFieldProperties.correctedMapsNames(),
                                     m_fdcMapFieldProperties.correctedMapsSequenceNbr(), m_fdcMapFieldProperties.addedTwtmapsequenceNbr(),
                                     m_caseStorageManager.resultsMapFileName(), surfacesIDs, noCalculatedTWToutput);
  }
  catch (const T2Zexception & ex)
  { throw T2Zexception() << "Tables could not be modified for the final project, error code: " << ex.what(); }
}

void FinalCaseManager::fcmCreateFinalCase()
{
  try
  { m_caseStorageManager.createFinalCase(); }
  catch (const T2Zexception & ex)
  { throw T2Zexception() << "Case could not be created for the final project, error code: " << ex.what(); }
}

void FinalCaseManager::appendCurrentSurfaceIsoPacks(const mbapi::StratigraphyManager::SurfaceID s, const std::vector<double> & depthsEndSurface)
{
  std::vector<double> isoPacks = m_fdcMapFieldProperties.isoPacks()[s];
  for (size_t i = 0; i != depthsEndSurface.size(); ++i)
  {
    isoPacks[i] += depthsEndSurface[i];
  }

  m_fdcMapFieldProperties.setIsoPacks(s, isoPacks);
}

void FinalCaseManager::updateIsoPackMapsInStratIoTbl(const mbapi::StratigraphyManager::SurfaceID s, const std::string & mapName, mbapi::MapsManager & mapsMgrFinal)
{
  size_t mapsSequenceNbr = Utilities::Numerical::NoDataIDValue;
  if (IsValueUndefined(mapsMgrFinal.generateMap("StratIoTbl", mapName, m_fdcMapFieldProperties.isoPacks()[s], mapsSequenceNbr, m_caseStorageManager.resultsMapFileName())))
  { throw T2Zexception() << " Cannot generate the map for the iso surface " << s; }
  m_fdcMapFieldProperties.setCorrectedMapsSequenceNbr(s, mapsSequenceNbr);
}

void FinalCaseManager::appendIsopacks(const mbapi::StratigraphyManager::SurfaceID endSurface, const std::vector<double> & depthsEndSurface)
{
  std::shared_ptr<mbapi::Model> mdl = m_fdcProjectManager.getModel();
  mbapi::MapsManager         & mapsMgrFinal = mdl->mapsManager();
  mbapi::StratigraphyManager & strMgrFinal  = mdl->stratigraphyManager();
  for (mbapi::StratigraphyManager::SurfaceID s = endSurface + 1; s < getSurfacesID().size(); ++s)
  {
    const std::string surfaceName = strMgrFinal.surfaceName(s);
    const std::string & mapName = m_fdcProjectManager.t2ZIsoPackMapName(surfaceName, m_caseStorageManager.date());
    m_fdcMapFieldProperties.setCorrectedMapsNames(s, mapName);

    if (m_rank == 0) { LogHandler(LogHandler::INFO_SEVERITY) << " Appending isopack for surface " << surfaceName; }

    appendCurrentSurfaceIsoPacks(s, depthsEndSurface);
    updateIsoPackMapsInStratIoTbl(s, mapName, mapsMgrFinal);
    m_fdcProjectManager.setCurrentSurfaceMapNameInStratIoTbl(s, mapName);
  }
}

void FinalCaseManager::checkForCauldronAlternativeNameSetting()
{
  std::shared_ptr<mbapi::Model> mdl = m_fdcProjectManager.getModel();
  if (mdl->tableSize("BPANameMapping") > 3)
  { m_fdcProjectManager.setAlternativeTableNames(m_caseStorageManager.date()); }
}

void FinalCaseManager::saveProjectAndFinalize()
{
  std::shared_ptr<mbapi::Model> mdl = m_fdcProjectManager.getModel();
  if (m_rank == 0)
  {
    mdl->saveModelToProjectFile(m_caseStorageManager.finalProjectFilePath().c_str(), true);
    m_caseStorageManager.removeFinalProjectCauldronOutputDir();
    m_caseStorageManager.removeMasterResultsFile();
  }
}

void FinalCaseManager::writeFinalProject(const mbapi::StratigraphyManager::SurfaceID endSurface, const std::vector<double> & depthsEndSurface, const std::vector<mbapi::StratigraphyManager::SurfaceID> & surfacesIDs, const bool noCalculatedTWToutput)
{
  runModifyTables(noCalculatedTWToutput, surfacesIDs);
  fcmCreateFinalCase();
  m_fdcProjectManager.reloadModel(m_caseStorageManager.caseProjectFilePath());
  appendIsopacks(endSurface, depthsEndSurface);
  checkForCauldronAlternativeNameSetting();
  saveProjectAndFinalize();
}

} // fastDepthCalibration
