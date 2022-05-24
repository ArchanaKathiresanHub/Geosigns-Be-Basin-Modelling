//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "MapsManager.h"

#include "model/input/cmbMapReader.h"
#include "model/input/projectReader.h"
#include "model/output/zycorWriter.h"
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
  m_radiusSmoothing{1000},
  m_smartGridding{true}
{

}

void MapsManager::clear()
{
   m_interpolationMethod = 0;
   m_smoothingOption = 0;
   m_pIDW = 3;
   m_radiusSmoothing = 1000;
   m_smartGridding = true;
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

   if (version > 2)
   {
      m_smartGridding = reader.readBool("smartGridding");
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
   writer.writeValue("smartGridding", m_smartGridding);
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

bool MapsManager::smartGridding() const
{
   return m_smartGridding;
}

void MapsManager::setSmartGridding(bool smartGridding)
{
   m_smartGridding = smartGridding;
}

void MapsManager::exportOptimizedLithofractionMapsToZycor(const ProjectReader& projectReader, MapReader& mapReader, const QString& targetPath)
{
   ZycorWriter writer;
   double xMin = 0.0;
   double xMax = 0.0;
   double yMin = 0.0;
   double yMax = 0.0;
   long numI = 0;
   long numJ = 0;
   mapReader.getMapDimensions(xMin, xMax, yMin, yMax, numI, numJ);
   const casaWizard::MapMetaData metaData(xMin, xMax, yMin, yMax, numI, numJ);

   for (int i = 0; i < projectReader.layerNames().size(); i++)
   {
      if (!mapReader.mapExists(std::to_string(i) + "_percent_1"))
      {
         continue;
      }
      std::vector<VectorVectorMap> lithoMaps = mapReader.getOptimizedLithoMapsInLayer(i);
      QStringList lithoNames = projectReader.lithologyTypesForLayer(i);
      int iLithoName = 0;
      for (const VectorVectorMap& lithoMap : lithoMaps)
      {
         writer.writeToFile(targetPath.toStdString() + "/" +
                            projectReader.layerNames()[i].toStdString() + "_" +
                            lithoNames[iLithoName].toStdString() + ".zyc",
                            lithoMap.getData(),
                            metaData);

         iLithoName++;
      }
   }
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
