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
#include "GridMap.h"
#include "Interface.h"
#include "LithoType.h"
#include "ObjectFactory.h"
#include "ProjectHandle.h"
#include "Property.h"
#include "PropertyValue.h"

#include "LogHandler.h"

#include <cmath>

namespace genex0d
{

Genex0dFormationManager::Genex0dFormationManager(const std::string & projectFilename, const std::string & formationName, const double x, const double y) :
  m_factory{nullptr},
  m_projectHandle{nullptr},
  m_formation{nullptr},
  m_thickness{0.0},
  m_indI{0},
  m_indJ{0}
{
  try
  {
    m_factory = new DataAccess::Interface::ObjectFactory();

    if (projectFilename.empty())
    {
      throw Genex0dException() << "Fatal error, empty project name!";
    }
    m_projectHandle.reset(DataAccess::Interface::OpenCauldronProject(projectFilename, m_factory));

    if (formationName.empty())
    {
      throw Genex0dException() << "Fatal error, empty formation name!";
    }
    m_formation = m_projectHandle->findFormation(formationName);

    if (!m_formation)
    {
      throw genex0d::Genex0dException() << "Formation " << formationName << " not found";
    }
    setProperties(x, y);
  }
  catch (const Genex0dException & ex)
  {
    cleanup();
    throw;
  }
}

void Genex0dFormationManager::cleanup()
{
  delete m_factory;
}

Genex0dFormationManager::~Genex0dFormationManager()
{
  cleanup();
}

void Genex0dFormationManager::setThickness()
{
  const DataAccess::Interface::GridMap* inputThicknessGrid = m_formation->getInputThicknessMap();
  inputThicknessGrid->retrieveData();
  m_thickness = inputThicknessGrid->getValue(m_indI, m_indJ, inputThicknessGrid->getDepth()-1);
}

void Genex0dFormationManager::setProperties(const double x, const double y)
{
  const DataAccess::Interface::Grid * gridLowResolution(m_projectHandle->getLowResolutionOutputGrid());

  if (gridLowResolution->getGridPoint(x, y, m_indI, m_indJ))
  {
    setThickness();
    return;
  }

  throw Genex0dException() << "The input (X,Y) location is out of range! \n"
                           << "Valid X ranges: (" << gridLowResolution->minI() << ", " << gridLowResolution->maxI() << ") \n"
                           << "Valid Y ranges: (" << gridLowResolution->minJ() << ", " << gridLowResolution->maxJ() << ") \n";
}

double Genex0dFormationManager::getLithoDensity(const DataAccess::Interface::LithoType * theLitho)
{
  const DataAccess::Interface::AttributeValue theDensity = theLitho->getAttributeValue(DataAccess::Interface::Density);
  return theDensity.getDouble();
}

void Genex0dFormationManager::calculateInorganicDensityOfLithoType(const DataAccess::Interface::LithoType * lithoType,
                                                                   const DataAccess::Interface::GridMap * lithoTypePercentageMap,
                                                                   double & inorganicDensity)
{
  double lithoTypeDensity = getLithoDensity (lithoType);
  lithoTypePercentageMap->retrieveData ();
  const unsigned int depthlithoTypePercentageMap = lithoTypePercentageMap->getDepth ();
  inorganicDensity +=
      lithoTypeDensity * 0.01 * lithoTypePercentageMap->getValue (m_indI, m_indJ,
                                                                  depthlithoTypePercentageMap - 1);
}

double Genex0dFormationManager::getInorganicDensity()
{
  double inorganicDensity = 0;
  const DataAccess::Interface::LithoType * litho1 = m_formation->getLithoType1();
  const DataAccess::Interface::LithoType * litho2 = m_formation->getLithoType2();
  const DataAccess::Interface::LithoType * litho3 = m_formation->getLithoType3();

  const DataAccess::Interface::GridMap * litho1PercentageMap = m_formation->getLithoType1PercentageMap();
  const DataAccess::Interface::GridMap * litho2PercentageMap = m_formation->getLithoType2PercentageMap();
  const DataAccess::Interface::GridMap * litho3PercentageMap = m_formation->getLithoType3PercentageMap();

  if (litho1 && litho1PercentageMap)
  {
    calculateInorganicDensityOfLithoType(litho1, litho1PercentageMap, inorganicDensity);
  }

  if (litho2 && litho2PercentageMap)
  {
    calculateInorganicDensityOfLithoType(litho2, litho2PercentageMap, inorganicDensity);
  }

  if (litho3 && litho3PercentageMap)
  {
    calculateInorganicDensityOfLithoType(litho3, litho3PercentageMap, inorganicDensity);
  }

  return inorganicDensity;
}

std::string Genex0dFormationManager::getTopSurfaceName() const
{
  return m_formation->getTopSurfaceName();
}

bool Genex0dFormationManager::isFormationSourceRock() const
{
  const bool tmp = m_formation->isSourceRock();
  return tmp;
}

double Genex0dFormationManager::getThickness() const
{
  return m_thickness;
}

} // namespace genex0d
