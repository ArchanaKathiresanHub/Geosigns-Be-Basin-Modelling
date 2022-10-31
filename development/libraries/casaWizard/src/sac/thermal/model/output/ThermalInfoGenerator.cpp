//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ThermalInfoGenerator.h"

namespace casaWizard
{

namespace sac
{

namespace thermal
{

ThermalInfoGenerator::ThermalInfoGenerator(const ThermalScenario& scenario,
                                           std::unique_ptr<ProjectReader> projectReader) :
   SacInfoGenerator(*projectReader),
   m_mapManager(scenario.mapsManager()),
   m_scenario(scenario),
   m_projectReader(std::move(projectReader))
{}

const ThermalMapManager& ThermalInfoGenerator::mapsManager() const
{
   return m_mapManager;
}

const ThermalScenario& ThermalInfoGenerator::scenario() const
{
   return m_scenario;
}

void ThermalInfoGenerator::addOptimizationOptionsInfo()
{
   addOption("Minimum Top Crust Heat Production ", doubleToFormattedString( m_scenario.TCHPmanager().minTCHP()) );
   addOption("Maximum Top Crust Heat Production ", doubleToFormattedString( m_scenario.TCHPmanager().maxTCHP()) );
}

} // namespace thermal

} // namespace sac

} // namespace casaWizard
