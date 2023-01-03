//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "LithoMapsInfoGenerator.h"

#include "model/LithologyScenario.h"
#include "model/input/cmbProjectReader.h"

namespace casaWizard
{

namespace sac
{

namespace lithology
{

LithoMapsInfoGenerator::LithoMapsInfoGenerator(const LithologyScenario& scenario, std::unique_ptr<ProjectReader> projectReader) :
   LithologyInfoGenerator(scenario, *projectReader),
   m_projectReader(std::move(projectReader))
{}

} // namespace lithology

} // namespace sac

} // namespace casaWizard
