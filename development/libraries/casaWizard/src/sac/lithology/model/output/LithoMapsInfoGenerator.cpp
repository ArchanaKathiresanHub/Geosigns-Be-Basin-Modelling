//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "LithoMapsInfoGenerator.h"

#include "model/sacLithologyScenario.h"
#include "model/input/cmbProjectReader.h"

namespace casaWizard
{

namespace sac
{

LithoMapsInfoGenerator::LithoMapsInfoGenerator(const SacLithologyScenario& scenario, std::unique_ptr<ProjectReader> projectReader) :
   SacInfoGeneratorLithology(scenario, *projectReader),
   m_projectReader(std::move(projectReader))
{}

LithoMapsInfoGenerator::~LithoMapsInfoGenerator() = default;

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
