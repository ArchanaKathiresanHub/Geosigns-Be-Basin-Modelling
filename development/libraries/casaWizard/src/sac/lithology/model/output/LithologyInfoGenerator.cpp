//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "LithologyInfoGenerator.h"

namespace casaWizard
{

namespace sac
{

namespace lithology
{

LithologyInfoGenerator::LithologyInfoGenerator(const LithologyScenario& scenario, ProjectReader& projectReader) :
   SacInfoGenerator(projectReader),
   m_mapsManager(scenario.mapsManager()),
   m_scenario(scenario)
{}

void LithologyInfoGenerator::addMapsGenerationSection()
{
   SacInfoGenerator::addMapsGenerationSection();
   addSmartGridding();
}

const LithologyMapsManager& LithologyInfoGenerator::mapsManager() const
{
   return m_mapsManager;
}

const LithologyScenario& LithologyInfoGenerator::scenario() const
{
   return m_scenario;
}

void LithologyInfoGenerator::addOptimizationOptionsInfo()
{
  const QStringList layerNames = projectReader().layerNames();
  const QStringList mapNames = projectReader().mapNames();

  int layerID = 0;
  for (const QString& layerName: layerNames)
  {
    bool includedInMapOptimization = false;
    for (const QString& mapName : mapNames)
    {

      QString test = QString::fromStdString(std::to_string(layerID)) + "_percent";
      if (mapName.startsWith(test))
      {
        includedInMapOptimization = true;
        break;
      }
    }

    addOption("Formation " + layerName.toStdString(), includedInMapOptimization ? "INCLUDED" : "EXCLUDED");

    layerID++;
  }
}

void LithologyInfoGenerator::addSmartGridding()
{
   addOption("Smart gridding", m_mapsManager.smartGridding() ? "Enabled" : "Disabled");
}

} // namespace lithology

} // namespace sac

} // namespace casaWizard
