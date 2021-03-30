//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "CaseStorageManager.h"
#include "CommonDefinitions.h"

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/filesystem.hpp"

#include "ErrorHandler.h"

#include "LogHandler.h"

#include "propinterface.h"

namespace fastDepthConversion
{

namespace
{

static const std::string s_finalResultsFolder = "T2Zcal_";
static const std::string s_masterResultsFile = "MasterInputs.HDF";

} // namespace

CaseStorageManager::CaseStorageManager(std::shared_ptr<mbapi::Model> & mdl, const int rank) :
  m_resultsMapFileName("CalibratedInputs.HDF"),
  m_rank(rank),
  m_date(""),
  m_mdl(mdl),
  m_projectFileName(m_mdl->projectFileName()),
  m_fullMasterDirPath(FolderPath( "." ).fullPath()),
  m_masterResultsFilePath(FolderPath( "." ).fullPath( ) << s_masterResultsFile),
  m_casePath(""),
  m_casePathResultsHDFFile(""),
  m_caseProjectFilePath("")
{
  setOriginalCaseProjectFilePath();
  removeMasterResultsFile();
}

void CaseStorageManager::setOriginalCaseProjectFilePath()
{
  m_caseProjectFilePath = ibs::FolderPath(m_fullMasterDirPath) << m_projectFileName;
}

std::string CaseStorageManager::resultsMapFileName() const
{
  return m_resultsMapFileName;
}

void CaseStorageManager::removeMasterResultsFile()
{
  if ( m_rank == 0 && m_masterResultsFilePath.exists() )
  {
    LogHandler( LogHandler::WARNING_SEVERITY ) << "Removing existing " << m_masterResultsFilePath.fileName();
    m_masterResultsFilePath.remove();
  }
}

// create a case folder with the input files
void CaseStorageManager::createCase(ibs::FilePath & casePathResults)
{
  int exception_rank0 = createAndSetupCase(casePathResults);

  MPI_Bcast(&exception_rank0, 1, MPI_INT, 0, PETSC_COMM_WORLD);
  MPI_Barrier(PETSC_COMM_WORLD);

  if (exception_rank0 == 1) { throw ErrorHandler::Exception(m_mdl->errorCode()) << m_mdl->errorMessage(); }
  else if (exception_rank0 == 2) { throw T2Zexception() << "Failed to copy file: " << casePathResults.path(); }
}

int CaseStorageManager::createAndSetupCase(ibs::FilePath & casePathResults)
{
  if (m_rank != 0)
  {
    return 0;
  }

  if (m_casePath.exists())
  {
    LogHandler(LogHandler::WARNING_SEVERITY) << "Folder " << m_casePath.fullPath().path() << " will be deleted";
    m_casePath.remove();
  }
  m_casePath.create();

  if (ErrorHandler::NoError != m_mdl->saveModelToProjectFile(m_caseProjectFilePath.path().c_str(), true)) { return 1; }
  if (!casePathResults.exists() && !m_masterResultsFilePath.copyFile(casePathResults)) { return 2; }
  return 0;
}

std::string CaseStorageManager::projectFileName() const
{
  return m_projectFileName;
}

std::string CaseStorageManager::finalProjectFilePath() const
{
  return m_caseProjectFilePath.path();
}

void CaseStorageManager::createTemporaryCase(const mbapi::StratigraphyManager::SurfaceID surface)
{
  std::string mapNamePath = "Surface_" + std::to_string(surface);
  m_casePath = ibs::FilePath(".") << mapNamePath;
  m_caseProjectFilePath = ibs::FilePath(m_casePath) << m_projectFileName;
  m_casePathResultsHDFFile = ibs::FilePath(m_casePath) << m_resultsMapFileName;
  createCase(m_casePathResultsHDFFile);
}

void CaseStorageManager::setDate()
{
  m_date.clear();
  std::vector<char> buffer(200);
  if ( m_rank == 0)
  {
    std::string bufferDate = boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time());
    if (bufferDate.size() > buffer.size())
    {
      LogHandler(LogHandler::WARNING_SEVERITY) << "Final case folder name charaters exceeded the limit!";
    }

    for (int i = 0; i < bufferDate.size(); ++i)
    {
      buffer[i] = bufferDate[i];
    }
  }
  MPI_Bcast(&buffer[0], buffer.size(), MPI_CHAR, 0, PETSC_COMM_WORLD);

  int i = 0;
  while (buffer[i])
  {
    m_date += buffer[i];
    ++i;
  }
}

void CaseStorageManager::createFinalCase()
{
  setDate();
  MPI_Barrier(PETSC_COMM_WORLD);

  std::string correct_myDate = m_date;
  std::replace_if(correct_myDate.begin(), correct_myDate.end(), ::ispunct, '_');
  std::replace_if(correct_myDate.begin(), correct_myDate.end(), ::isspace, '_');
  m_casePath = std::string("./") + s_finalResultsFolder + correct_myDate;
  m_caseProjectFilePath = ibs::FilePath(m_casePath) << m_projectFileName;
  ibs::FilePath finalResultsFilePath(m_casePath.path() + "/" + m_resultsMapFileName);
  createCase(finalResultsFilePath);
}

void CaseStorageManager::removeFinalProjectCauldronOutputDir()
{
  if (m_rank == 0)
  {
    ibs::FolderPath finalProjectCauldronOutputDir = ibs::FolderPath(m_casePath.path() + "/" + m_caseProjectFilePath.fileNameNoExtension() + mbapi::Model::s_ResultsFolderSuffix);
    finalProjectCauldronOutputDir.remove();
  }
}

void CaseStorageManager::changeToTemporaryCaseDirectoryPath()
{
  if (!m_casePath.setPath())
  {
    throw T2Zexception() << "Cannot change to the case directory " << m_casePath.fullPath().path();
  }
  MPI_Barrier(PETSC_COMM_WORLD);
}

void CaseStorageManager::CopyTemporaryToMasterHDFMaps()
{
  int exception_rank0 = 0;
  if (m_rank == 0)
  {
    m_masterResultsFilePath.remove();
    if (!m_casePathResultsHDFFile.copyFile(m_masterResultsFilePath))
    {
      exception_rank0 == 1;
    }
    m_casePath.remove();
    if (!m_fullMasterDirPath.setPath())
    {
      exception_rank0 == 2;
    }
    setOriginalCaseProjectFilePath();
  }

  MPI_Bcast(&exception_rank0, 1, MPI_INT, 0, PETSC_COMM_WORLD);
  MPI_Barrier(PETSC_COMM_WORLD);

  if(exception_rank0 == 1)
  {
    throw T2Zexception() << "Cannot copy the result file to " << m_masterResultsFilePath.fullPath().path();
  }
  if(exception_rank0 == 2)
  {
    throw T2Zexception() << " Cannot change to the master directory " << m_fullMasterDirPath.path();
  }
}

void CaseStorageManager::saveModelToCaseProjectFile()
{
  if (m_rank == 0)
  {
    m_mdl->saveModelToProjectFile(caseProjectFilePath().c_str(), true);
  }
  MPI_Barrier(PETSC_COMM_WORLD);
}

void CaseStorageManager::changeToMasterDirectoryPath()
{
  if (!m_fullMasterDirPath.setPath())
  {
    throw T2Zexception() << " Cannot change to the master directory " << m_fullMasterDirPath.path();
  }
  MPI_Barrier(PETSC_COMM_WORLD);
}

std::string CaseStorageManager::masterResultsFilePath() const
{
  return m_masterResultsFilePath.path();
}

std::string CaseStorageManager::caseProjectFilePath() const
{
  return m_caseProjectFilePath.path();
}

std::string CaseStorageManager::date() const
{
  return m_date;
}

void CaseStorageManager::setModel(std::shared_ptr<mbapi::Model> & mdl)
{
  m_mdl.reset();
  m_mdl = mdl;
}

} // namespace fastDepthConversion
