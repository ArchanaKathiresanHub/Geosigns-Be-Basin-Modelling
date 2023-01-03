//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Generate3DScenarioScriptLithology.h"

#include "model/LithologyScenario.h"

namespace casaWizard
{

namespace sac
{

namespace lithology
{

Generate3DScenarioScriptLithology::Generate3DScenarioScriptLithology(const LithologyScenario& scenario):
   Generate3DScenarioScript(scenario),
   m_mapsManager(scenario.mapsManager())
{}

const LithologyMapsManager& Generate3DScenarioScriptLithology::mapsManager() const
{
   return m_mapsManager;
}

QString Generate3DScenarioScriptLithology::setFilterOneDResults() const
{
   QString command("setFilterOneDResults ");
   command += m_mapsManager.smartGridding() ? "smartLithoFractionGridding" : "none";
   addWellIndicesToFilter1DResults(command);
   return command;
}

} // namespace lithology

} // namespace sac

} // namespace casaWizard
