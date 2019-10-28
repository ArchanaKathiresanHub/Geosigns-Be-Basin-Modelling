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

namespace fastDepthCalibration
{

class CaseStorageManager
{
public:
  explicit CaseStorageManager(std::shared_ptr<mbapi::Model> & mdl, const int rank);

  void setOriginalCaseProjectFilePath();
  void createTemporaryCase(const mbapi::StratigraphyManager::SurfaceID surface);
  void createFinalCase();
  void changeToTemporaryCaseDirectoryPath();
  void CopyTemporaryToMasterHDFMaps();
  void changeToMasterDirectoryPath();
  void removeFinalProjectCauldronOutputDir();

  void saveModelToCaseProjectFile();

  std::string masterResultsFilePath() const;
  std::string caseProjectFilePath() const;
  std::string resultsMapFileName() const;
  std::string date() const;

  std::string finalProjectFilePath() const;
  void setModel(std::shared_ptr<mbapi::Model> & mdl);

  std::string projectFileName() const;
  void removeMasterResultsFile();

private:
  /// @brief Create a new folder casePath for the caseProject with casePathResults (the input maps BEFORE the calibration)
  /// @param casePath the ibs::FolderPath for the new case
  /// @param caseProject the ibs::FilePath for the new case filename
  /// @param casePathResults the ibs::FilePath destination of the input maps
  void createCase(ibs::FilePath & casePathResults);

  int createAndSetupCase(ibs::FilePath & casePathResults);
  void setDate();

  const std::string m_resultsMapFileName;
  int             m_rank;
  std::string     m_date;
  std::shared_ptr<mbapi::Model> m_mdl;
  std::string     m_projectFileName;
  ibs::FolderPath m_fullMasterDirPath;
  ibs::FilePath   m_masterResultsFilePath;
  ibs::FolderPath m_casePath;
  ibs::FilePath   m_casePathResultsHDFFile;
  ibs::FilePath   m_caseProjectFilePath;
};

} // namespace fastDepthCalibration
