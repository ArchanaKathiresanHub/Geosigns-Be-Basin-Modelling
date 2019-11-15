//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Genex0dProjectManager.h"

#include "CommonDefinitions.h"

// cmbAPI
#include "ErrorHandler.h"

namespace genex0d
{

Genex0dProjectManager::Genex0dProjectManager(const std::string & projectFileName, const std::string & outProjectFileName, const double xCoord, const double yCoord):
  m_projectFileName{projectFileName},
  m_xCoord{xCoord},
  m_yCoord{yCoord},
  m_mdl{nullptr},
  m_posData{0},
  m_posDataPrevious{0},
  m_outProjectFileName{outProjectFileName}
{
  reloadModel(m_projectFileName);
}

void Genex0dProjectManager::saveModel(const std::string & name)
{
  if (ErrorHandler::NoError != m_mdl->saveModelToProjectFile(name.c_str()))
  {
    throw ErrorHandler::Exception(m_mdl->errorCode())
        << "The model could not be saved to output file " << name << ", " << m_mdl->errorMessage();
  }
}

void Genex0dProjectManager::resetWindowingAndSampling(const unsigned int indI, const unsigned int indJ)
{
  if (ErrorHandler::NoError != m_mdl->setWindow(indI, indI, indJ, indJ))
  {
    throw ErrorHandler::Exception(m_mdl->errorCode())
        << "Windowing around the specified location failed, " << m_mdl->errorMessage();
  }

  if (ErrorHandler::NoError != m_mdl->setSubsampling(1, 1))
  {
    throw ErrorHandler::Exception(m_mdl->errorCode())
        << "Resetting of the subsampling values failed, " << m_mdl->errorMessage();
  }

  saveModel(m_outProjectFileName);
}

void Genex0dProjectManager::reloadModel(const std::string & projectFileName)
{
  m_mdl.reset(new mbapi::Model());
  if (ErrorHandler::NoError != m_mdl->loadModelFromProjectFile(projectFileName.c_str()))
  {
    throw ErrorHandler::Exception(m_mdl->errorCode()) << "Could not initialize data mining table, " << m_mdl->errorMessage();
  }
}

} // namespace genex0d
