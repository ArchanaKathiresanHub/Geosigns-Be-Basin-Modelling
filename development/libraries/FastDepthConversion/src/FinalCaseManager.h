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
                            CaseStorageManager & caseStorageManager, int rank);
  ~FinalCaseManager();

  /// @brief Write the final project and calibrated input files into the s_finalResultsFolder
  void writeFinalProject(const mbapi::StratigraphyManager::SurfaceID endSurface, const std::vector<double> & depthsEndSurface, const bool noCalculatedTWToutput);

private:  
  void runModifyTables(const bool noCalculatedTWToutput);
  void fcmCreateFinalCase();
  void appendCurrentSurfaceIsoPacks(const mbapi::StratigraphyManager::SurfaceID s, const std::vector<double> & depthsEndSurface);
  void updateIsoPackMapsInStratIoTbl(const mbapi::StratigraphyManager::SurfaceID s, const string & mapName);
  void appendIsopacks(const mbapi::StratigraphyManager::SurfaceID endSurface, const std::vector<double> & depthsEndSurface);
  void saveProjectAndFinalize();

  FDCProjectManager&     m_fdcProjectManager;
  FDCMapFieldProperties& m_fdcMapFieldProperties;
  CaseStorageManager&    m_caseStorageManager;
  int m_rank;
};

} // namespace fastDepthConversion
