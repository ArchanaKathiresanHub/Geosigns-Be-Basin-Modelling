//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "MapsManagerLithology.h"
#include "model/input/mapReader.h"
#include "model/input/projectReader.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"
#include "model/output/zycorWriter.h"

namespace casaWizard
{

namespace sac
{

MapsManagerLithology::MapsManagerLithology():
   m_smartGridding{true}
{}

void MapsManagerLithology::clear()
{
   MapsManager::clear();
   m_smartGridding = true;
}

void MapsManagerLithology::readFromFile(const ScenarioReader& reader)
{
   MapsManager::readFromFile(reader);
   int version = reader.readInt("mapsManagerVersion");
   if (version > 2)
   {
      m_smartGridding = reader.readBool("smartGridding");
   }
}

void MapsManagerLithology::writeToFile(ScenarioWriter& writer) const
{
   MapsManager::writeToFile(writer);
   writer.writeValue("smartGridding", m_smartGridding);
}

bool MapsManagerLithology::smartGridding() const
{
   return m_smartGridding;
}

void MapsManagerLithology::setSmartGridding(bool smartGridding)
{
   m_smartGridding = smartGridding;
}

void MapsManagerLithology::exportOptimizedMapsToZycor(const ProjectReader& projectReader, MapReader& mapReader, const QString& targetPath)
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

} // namespace sac

} // namespace casaWizard
