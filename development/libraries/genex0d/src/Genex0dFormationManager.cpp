//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Genex0dFormationManager.h"

#include "CommonDefinitions.h"

// DataAccess
#include "Formation.h"
#include "Interface.h"
#include "ProjectHandle.h"

// utilities
#include "LogHandler.h"

#include <cmath>

namespace genex0d
{

Genex0dFormationManager::Genex0dFormationManager(DataAccess::Interface::ProjectHandle * projectHandle, const std::string & formationName) :
  m_projectHandle{projectHandle},
  m_formation{nullptr},
  m_gridLowResolution{nullptr},
  m_indI{0},
  m_indJ{0}
{
  if (m_projectHandle == nullptr)
  {
    throw Genex0dException() << "Fatal error, projectHandle could not be initiated!";
  }

  if (formationName.empty())
  {
    throw Genex0dException() << "Fatal error, empty formation name!";
  }
  m_formation = m_projectHandle->findFormation(formationName);

  if (!m_formation)
  {
    throw Genex0dException() << "Formation " << formationName << " not found";
  }

  m_gridLowResolution = (m_projectHandle->getLowResolutionOutputGrid());
}

unsigned int Genex0dFormationManager::indJ() const
{
  return m_indJ;
}

unsigned int Genex0dFormationManager::indI() const
{
  return m_indI;
}

const DataAccess::Interface::Formation * Genex0dFormationManager::formation() const
{
  return m_formation;
}

void Genex0dFormationManager::setProperties(const double x, const double y)
{
//  const DataAccess::Interface::Grid * gridLowResolution(m_projectHandle->getLowResolutionOutputGrid());

  if (!m_gridLowResolution->getGridPoint(x, y, m_indI, m_indJ))
  {
    throw Genex0dException() << "The input (X,Y) location is out of range! \n"
                             << "Valid X ranges: (" << m_gridLowResolution->minI() << ", " << m_gridLowResolution->maxI() << ") \n"
                             << "Valid Y ranges: (" << m_gridLowResolution->minJ() << ", " << m_gridLowResolution->maxJ() << ") \n";
  }
}

bool Genex0dFormationManager::isFormationSourceRock() const
{
  const bool tmp = m_formation->isSourceRock();
  return tmp;
}

} // namespace genex0d
