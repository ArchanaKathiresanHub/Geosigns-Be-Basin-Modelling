//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.

// CaseStorageManager class manages the storage of the temporary cases and the final case
#pragma once

#include "cmbAPI.h"

#include "FilePath.h"
#include "FolderPath.h"

#include <string>

namespace fastDepthConversion
{

class FDCProjectManager;

class CaseStorageManager
{
public:
  explicit CaseStorageManager(FDCProjectManager& m_fdcProjectManager, const std::string& projectFileName, const int rank);

  void setOriginalMasterPath();
  void createTemporaryCase(const mbapi::StratigraphyManager::SurfaceID surface);
  void createFinalCase();
  void changeToTemporaryCaseDirectoryPath();
  void copyTemporaryToMasterHDFMaps();
  void changeToMasterDirectoryPath();
  void removeFinalProjectCauldronOutputDir();
  void removeMasterResultsFile();
  void saveModelToCaseProjectFile();

  std::string masterResultsFilePath() const;  
  std::string resultsMapFileName() const;  
  std::string caseProjectFilePath() const;

private:
  /// @brief Create a new folder casePath for the caseProject with casePathResults (the input maps BEFORE the calibration)
  /// @param casePath the ibs::FolderPath for the new case
  /// @param caseProject the ibs::FilePath for the new case filename
  /// @param casePathResults the ibs::FilePath destination of the input maps
  void createCase();

  int createAndSetupCase(const ibs::FilePath & casePathResults);
  std::string getDate() const;

  ibs::FilePath   casePathResultsMapFile() const;

  int                m_rank;
  FDCProjectManager& m_fdcProjectManager;
  std::string        m_projectFileName;
  ibs::FolderPath    m_fullMasterDirPath;
  ibs::FilePath      m_masterResultsFilePath;
  ibs::FolderPath    m_casePath;
};

} // namespace fastDepthConversion
