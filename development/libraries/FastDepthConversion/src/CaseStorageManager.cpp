//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "CaseStorageManager.h"
#include "CommonDefinitions.h"
#include "FDCProjectManager.h"

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
static const std::string s_resultsMapFile = "CalibratedInputs.HDF";

} // namespace

CaseStorageManager::CaseStorageManager(FDCProjectManager& fdcProjectManager, const std::string& projectFileName, const int rank) :
  m_rank(rank),  
  m_fdcProjectManager(fdcProjectManager),
  m_projectFileName(projectFileName),
  m_fullMasterDirPath(FolderPath( "." ).fullPath()),
  m_masterResultsFilePath(FolderPath( "." ).fullPath( ) << s_masterResultsFile),
  m_casePath(m_fullMasterDirPath)
{  
  MPI_Barrier(PETSC_COMM_WORLD);
  if ( m_rank == 0)
  {
    removeMasterResultsFile();
  }
  MPI_Barrier(PETSC_COMM_WORLD);
}

void CaseStorageManager::setOriginalMasterPath()
{
  m_casePath = m_fullMasterDirPath;
}

std::string CaseStorageManager::resultsMapFileName() const
{
  return s_resultsMapFile;
}

void CaseStorageManager::removeMasterResultsFile()
{  
  assert( m_rank == 0 );

  if ( m_masterResultsFilePath.exists() )
  {
    LogHandler( LogHandler::WARNING_SEVERITY ) << "Removing existing " << m_masterResultsFilePath.fileName();
    m_masterResultsFilePath.remove();
  }  
}

// create a case folder with the input files
void CaseStorageManager::createCase()
{
  const ibs::FilePath casePathResults = casePathResultsMapFile();

  MPI_Barrier(PETSC_COMM_WORLD);
  int exception_rank0 = (m_rank == 0) ? createAndSetupCase(casePathResults) : 0;

  MPI_Bcast(&exception_rank0, 1, MPI_INT, 0, PETSC_COMM_WORLD);  

  if (exception_rank0 == 1) { throw T2Zexception() << "Failed to save project to: " <<  caseProjectFilePath() ; }
  else if (exception_rank0 == 2) { throw T2Zexception() << "Failed to copy file: " << casePathResults.path(); }

  MPI_Barrier(PETSC_COMM_WORLD);
}

int CaseStorageManager::createAndSetupCase(const ibs::FilePath& casePathResults)
{
  assert(m_rank == 0); // Only rank 0 can do this operation

  if (m_casePath.exists())
  {
    LogHandler(LogHandler::WARNING_SEVERITY) << "Folder " << m_casePath.fullPath().path() << " will be deleted";
    m_casePath.remove();
  }
  m_casePath.create();

  if (ErrorHandler::NoError != m_fdcProjectManager.saveModelToProjectFile(caseProjectFilePath(), true)) { return 1; }
  if (!casePathResults.exists() && !m_masterResultsFilePath.copyFile(casePathResults)) { return 2; }
  return 0;
}

std::string CaseStorageManager::caseProjectFilePath() const
{
  return (ibs::FilePath(m_casePath) << m_projectFileName).path();
}

void CaseStorageManager::createTemporaryCase(const mbapi::StratigraphyManager::SurfaceID surface)
{  
  m_casePath = ibs::FilePath(".") << "Surface_" + std::to_string(surface);
  createCase();
}

std::string CaseStorageManager::getDate() const
{
  std::string date;
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
    date += buffer[i];
    ++i;
  }

  std::replace_if(date.begin(), date.end(), ::ispunct, '_');
  std::replace_if(date.begin(), date.end(), ::isspace, '_');

  return date;
}

FilePath CaseStorageManager::casePathResultsMapFile() const
{
  return ibs::FilePath(m_casePath) << s_resultsMapFile;
}

void CaseStorageManager::createFinalCase()
{  
  m_casePath = std::string("./") + s_finalResultsFolder + getDate();
  createCase();
}

void CaseStorageManager::removeFinalProjectCauldronOutputDir()
{
  assert( m_rank == 0);

  ibs::FolderPath finalProjectCauldronOutputDir = ibs::FolderPath(m_casePath.path() + "/" + ibs::FilePath(caseProjectFilePath()).fileNameNoExtension() + mbapi::Model::s_ResultsFolderSuffix);
  finalProjectCauldronOutputDir.remove();
}

void CaseStorageManager::changeToTemporaryCaseDirectoryPath()
{  
  if (!m_casePath.setPath())
  {
    throw T2Zexception() << "Cannot change to the case directory " << m_casePath.fullPath().path();
  }
}

void CaseStorageManager::copyTemporaryToMasterHDFMaps()
{
  MPI_Barrier(PETSC_COMM_WORLD);
  int exception_rank0 = 0;
  if (m_rank == 0)
  {
    m_masterResultsFilePath.remove();
    if (!casePathResultsMapFile().copyFile(m_masterResultsFilePath))
    {
      exception_rank0 == 1;
    }
    m_casePath.remove();
  }

  MPI_Bcast(&exception_rank0, 1, MPI_INT, 0, PETSC_COMM_WORLD);

  if(exception_rank0 == 1)
  {
    throw T2Zexception() << "Cannot copy the result file to " << m_masterResultsFilePath.fullPath().path();
  }
  MPI_Barrier(PETSC_COMM_WORLD);
}

void CaseStorageManager::saveModelToCaseProjectFile()
{
  MPI_Barrier(PETSC_COMM_WORLD);
  if (m_rank == 0)
  {
    m_fdcProjectManager.saveModelToProjectFile(caseProjectFilePath(), true);
  }
  MPI_Barrier(PETSC_COMM_WORLD);
}

void CaseStorageManager::changeToMasterDirectoryPath()
{
  if (!m_fullMasterDirPath.setPath())
  {
    throw T2Zexception() << " Cannot change to the master directory " << m_fullMasterDirPath.path();
  }
}

std::string CaseStorageManager::masterResultsFilePath() const
{
  return m_masterResultsFilePath.path();
}

} // namespace fastDepthConversion
