//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "MapsManager.h"

#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

namespace casaWizard
{

namespace sac
{

MapsManager::MapsManager() :
  m_interpolationMethod{0},
  m_smoothingOption{0},
  m_pIDW{3},
  m_radiusSmoothing{1000}
{

}

void MapsManager::clear()
{
   m_interpolationMethod = 0;
   m_smoothingOption = 0;
   m_pIDW = 3;
   m_radiusSmoothing = 1000;
}

void MapsManager::readFromFile(const ScenarioReader& reader)
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

void MapsManager::writeToFile(ScenarioWriter& writer) const
{
   int mapsManagerVersion = 3;
   writer.writeValue("mapsManagerVersion", mapsManagerVersion);
   writer.writeValue("interpolationMethod", m_interpolationMethod);
   writer.writeValue("smoothingOption",m_smoothingOption);
   writer.writeValue("pIDW",m_pIDW);
   writer.writeValue("radiusSmoothing",m_radiusSmoothing);
}

int MapsManager::interpolationMethod() const
{
   return m_interpolationMethod;
}

void MapsManager::setInterpolationMethod(int interpolationMethod)
{
   m_interpolationMethod = interpolationMethod;
}

int MapsManager::smoothingOption() const
{
   return m_smoothingOption;
}

void MapsManager::setSmoothingOption(int smoothingOption)
{
   m_smoothingOption = smoothingOption;
}

int MapsManager::pIDW() const
{
   return m_pIDW;
}

void MapsManager::setPIDW(int pIDW)
{
   m_pIDW = pIDW;
}

int MapsManager::radiusSmoothing() const
{
   return m_radiusSmoothing;
}

void MapsManager::setRadiusSmoothing(int radiusSmoothing)
{
   m_radiusSmoothing = radiusSmoothing;
}

QVector<int> MapsManager::transformToActiveAndIncluded(const QVector<int>& selectedWellIndices, const QVector<int>& excludedWells)
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
