//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SacMapsManager.h"

#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

namespace casaWizard
{

namespace sac
{

SacMapsManager::SacMapsManager() :
  m_interpolationMethod{0},
  m_smoothingOption{0},
  m_pIDW{3},
  m_radiusSmoothing{1000}
{

}

void SacMapsManager::clear()
{
   m_interpolationMethod = 0;
   m_smoothingOption = 0;
   m_pIDW = 3;
   m_radiusSmoothing = 1000;
}

void SacMapsManager::readFromFile(const ScenarioReader& reader)
{
   int version = reader.readInt("mapsManagerVersion");
   if (version < 1)
   {
      version = reader.readInt("SACScenarioVersion");
   }

   if (version > 0)
   {
      m_interpolationMethod = reader.readInt("interpolationMethod");
      m_smoothingOption = reader.readInt("smoothingOption");
      m_pIDW = reader.readInt("pIDW");
      m_radiusSmoothing = reader.readInt("radiusSmoothing");
   }
}

void SacMapsManager::writeToFile(ScenarioWriter& writer) const
{
   int mapsManagerVersion = 3;
   writer.writeValue("mapsManagerVersion", mapsManagerVersion);
   writer.writeValue("interpolationMethod", m_interpolationMethod);
   writer.writeValue("smoothingOption",m_smoothingOption);
   writer.writeValue("pIDW",m_pIDW);
   writer.writeValue("radiusSmoothing",m_radiusSmoothing);
}

int SacMapsManager::interpolationMethod() const
{
   return m_interpolationMethod;
}

void SacMapsManager::setInterpolationMethod(int interpolationMethod)
{
   m_interpolationMethod = interpolationMethod;
}

int SacMapsManager::smoothingOption() const
{
   return m_smoothingOption;
}

void SacMapsManager::setSmoothingOption(int smoothingOption)
{
   m_smoothingOption = smoothingOption;
}

int SacMapsManager::pIDW() const
{
   return m_pIDW;
}

void SacMapsManager::setPIDW(int pIDW)
{
   m_pIDW = pIDW;
}

int SacMapsManager::radiusSmoothing() const
{
   return m_radiusSmoothing;
}

void SacMapsManager::setRadiusSmoothing(int radiusSmoothing)
{
   m_radiusSmoothing = radiusSmoothing;
}

QVector<int> SacMapsManager::transformToActiveAndIncluded(const QVector<int>& selectedWellIndices, const QVector<int>& excludedWells)
{
   QVector<int> wellIndicesActiveIncluded;
   for (int wellIndex : selectedWellIndices)
   {
      int exclusionShift = 0;
      bool excluded = false;
      for (int excludedWellIndex : excludedWells)
      {
         if(wellIndex == excludedWellIndex) excluded = true;
         if(wellIndex >  excludedWellIndex) exclusionShift++;
      }

      if (!excluded)
      {
         wellIndicesActiveIncluded.push_back(wellIndex - exclusionShift);
      }
   }

   return wellIndicesActiveIncluded;
}

} // namespace sac

} // namespace casaWizard
