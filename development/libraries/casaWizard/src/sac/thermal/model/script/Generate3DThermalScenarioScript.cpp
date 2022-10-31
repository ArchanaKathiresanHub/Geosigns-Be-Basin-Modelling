//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Generate3DThermalScenarioScript.h"

#include "model/ThermalScenario.h"

namespace casaWizard
{

namespace sac
{

namespace thermal
{

Generate3DThermalScenarioScript::Generate3DThermalScenarioScript(const ThermalScenario& scenario):
   Generate3DScenarioScript(scenario),
   m_mapManager(scenario.mapsManager())
{}

const ThermalMapManager& Generate3DThermalScenarioScript::mapsManager() const
{
   return m_mapManager;
}

QString Generate3DThermalScenarioScript::setFilterOneDResults() const
{
   QString command("setFilterOneDResults none");
   addWellIndicesToFilter1DResults(command);
   return command;
}

} // namespace thermal

} // namespace sac

} // namespace casaWizard
