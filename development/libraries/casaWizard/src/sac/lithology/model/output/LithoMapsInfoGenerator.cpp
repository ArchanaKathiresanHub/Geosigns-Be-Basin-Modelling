//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "LithoMapsInfoGenerator.h"

#include "model/sacLithologyScenario.h"

namespace casaWizard
{

namespace sac
{

LithoMapsInfoGenerator::LithoMapsInfoGenerator(const SacLithologyScenario& scenario, ProjectReader& projectReader) :
  SACInfoGenerator(scenario, projectReader)
{

}

void LithoMapsInfoGenerator::generateInfoTextFile()
{
  addGeneralSettingsSection();
  addInputTabSection();
  addMapsGenerationSection();
  addWellsSection();
  addSectionSeparator();

  writeTextToFile();
}

} // namespace sac

} // namespace casaWizard
