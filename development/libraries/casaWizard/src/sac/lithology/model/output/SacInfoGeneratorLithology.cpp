//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SacInfoGeneratorLithology.h"

namespace casaWizard
{

namespace sac
{

SacInfoGeneratorLithology::SacInfoGeneratorLithology(const SacLithologyScenario& scenario, ProjectReader& projectReader) :
   SacInfoGenerator(projectReader),
   m_mapsManager(scenario.mapsManager()),
   m_scenario(scenario)
{}

void SacInfoGeneratorLithology::addMapsGenerationSection()
{
   SacInfoGenerator::addMapsGenerationSection();
   addSmartGridding();
}

const MapsManagerLithology& SacInfoGeneratorLithology::mapsManager() const
{
   return m_mapsManager;
}

const SacLithologyScenario& SacInfoGeneratorLithology::scenario() const
{
   return m_scenario;
}

void SacInfoGeneratorLithology::addSmartGridding()
{
   addOption("Smart gridding", m_mapsManager.smartGridding() ? "Enabled" : "Disabled");
}

} // namespace sac

} // namespace casaWizard
