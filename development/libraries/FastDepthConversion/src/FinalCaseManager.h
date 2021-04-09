//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// FinalCaseManager class manages the writing of the final case
#pragma once

#include "cmbAPI.h"

#include <string>
#include <vector>

namespace fastDepthConversion
{

class CaseStorageManager;
class FDCMapFieldProperties;
class FDCProjectManager;

class FinalCaseManager
{
public:
  explicit FinalCaseManager(FDCProjectManager  & fdcProjectManager,
                            FDCMapFieldProperties & fdcMapFieldProperties,
                            CaseStorageManager & caseStorageManager);
  ~FinalCaseManager();

  /// @brief Write the final project and calibrated input files into the s_finalResultsFolder
  void writeFinalProject(const mbapi::StratigraphyManager::SurfaceID endSurface, const std::vector<double> & depthsEndSurface, const std::vector<mbapi::StratigraphyManager::SurfaceID> & surfacesIDs, const bool noCalculatedTWToutput);

private:
  std::vector<mbapi::StratigraphyManager::SurfaceID> getSurfacesID() const;
  void runModifyTables(const bool noCalculatedTWToutput, const std::vector<mbapi::StratigraphyManager::SurfaceID>& surfacesIDs);
  void fcmCreateFinalCase();
  void appendCurrentSurfaceIsoPacks(const mbapi::StratigraphyManager::SurfaceID s, const std::vector<double> & depthsEndSurface);
  void updateIsoPackMapsInStratIoTbl(const mbapi::StratigraphyManager::SurfaceID s, const string & mapName, mbapi::MapsManager & mapsMgrFinal);
  void appendIsopacks(const mbapi::StratigraphyManager::SurfaceID endSurface, const std::vector<double> & depthsEndSurface);
  void saveProjectAndFinalize();

  FDCProjectManager  & m_fdcProjectManager;
  FDCMapFieldProperties  & m_fdcMapFieldProperties;
  CaseStorageManager & m_caseStorageManager;
  int m_rank;
};

} // namespace fastDepthConversion
