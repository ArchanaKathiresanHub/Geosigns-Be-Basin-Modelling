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

namespace fastDepthConversion
{

FinalCaseManager::FinalCaseManager(FDCProjectManager  & fdcProjectManager,
                                   FDCMapFieldProperties & fdcMapFieldProperties,
                                   CaseStorageManager & caseStorageManager,
                                   int rank) :
  m_fdcProjectManager{fdcProjectManager},
  m_fdcMapFieldProperties{fdcMapFieldProperties},
  m_caseStorageManager{caseStorageManager},
  m_rank{rank}
{  
}

FinalCaseManager::~FinalCaseManager()
{}

void FinalCaseManager::runModifyTables(const bool noCalculatedTWToutput)
{
  try
  {
    m_fdcProjectManager.modifyTables(m_fdcProjectManager.surfacesIDs().size() - 1, m_fdcMapFieldProperties.hiatusAll(), m_fdcMapFieldProperties.correctedMapsNames(),
                                     m_fdcMapFieldProperties.correctedMapsSequenceNbr(), m_fdcMapFieldProperties.addedTwtmapsequenceNbr(),
                                     m_caseStorageManager.resultsMapFileName(), noCalculatedTWToutput);
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

void FinalCaseManager::updateIsoPackMapsInStratIoTbl(const mbapi::StratigraphyManager::SurfaceID s, const std::string & mapName)
{
  mbapi::MapsManager & mapsMgrFinal = m_fdcProjectManager.getMapsManager();

  size_t mapsSequenceNbr = Utilities::Numerical::NoDataIDValue;
  if (IsValueUndefined(mapsMgrFinal.generateMap("StratIoTbl", mapName, m_fdcMapFieldProperties.isoPacks()[s], mapsSequenceNbr, m_caseStorageManager.resultsMapFileName())))
  { throw T2Zexception() << " Cannot generate the map for the iso surface " << s; }
  m_fdcMapFieldProperties.setCorrectedMapsSequenceNbr(s, mapsSequenceNbr);
}

void FinalCaseManager::appendIsopacks(const mbapi::StratigraphyManager::SurfaceID endSurface, const std::vector<double> & depthsEndSurface)
{  
  mbapi::StratigraphyManager & strMgrFinal  = m_fdcProjectManager.getStratManager();
  for (mbapi::StratigraphyManager::SurfaceID s = endSurface + 1; s < m_fdcProjectManager.surfacesIDs().size(); ++s)
  {
    const std::string surfaceName = strMgrFinal.surfaceName(s);
    const std::string & mapName = m_fdcProjectManager.t2ZIsoPackMapName(surfaceName);
    m_fdcMapFieldProperties.setCorrectedMapsNames(s, mapName);

    if (m_rank == 0) { LogHandler(LogHandler::INFO_SEVERITY) << " Appending isopack for surface " << surfaceName; }

    appendCurrentSurfaceIsoPacks(s, depthsEndSurface);
    updateIsoPackMapsInStratIoTbl(s, mapName);
    m_fdcProjectManager.setCurrentSurfaceMapNameInStratIoTbl(s, mapName);
  }
}

void FinalCaseManager::saveProjectAndFinalize()
{    
  if (m_rank == 0)
  {    
    m_fdcProjectManager.saveModelToProjectFile(m_caseStorageManager.caseProjectFilePath(), true);
    m_caseStorageManager.removeFinalProjectCauldronOutputDir();
    m_caseStorageManager.removeMasterResultsFile();
  }
}

void FinalCaseManager::writeFinalProject(const mbapi::StratigraphyManager::SurfaceID endSurface, const std::vector<double> & depthsEndSurface, const bool noCalculatedTWToutput)
{
  MPI_Barrier(PETSC_COMM_WORLD);
  runModifyTables(noCalculatedTWToutput);
  MPI_Barrier(PETSC_COMM_WORLD);
  fcmCreateFinalCase();
  MPI_Barrier(PETSC_COMM_WORLD);
  m_fdcProjectManager.reloadModel(m_caseStorageManager.caseProjectFilePath());
  MPI_Barrier(PETSC_COMM_WORLD);
  appendIsopacks(endSurface, depthsEndSurface);
  MPI_Barrier(PETSC_COMM_WORLD);
  saveProjectAndFinalize();
  MPI_Barrier(PETSC_COMM_WORLD);
}

} // fastDepthConversion
