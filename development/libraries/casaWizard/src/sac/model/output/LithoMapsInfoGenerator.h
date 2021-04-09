//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "model/output/SACInfoGenerator.h"

namespace casaWizard
{

namespace sac
{

class SACScenario;

class LithoMapsInfoGenerator : public SACInfoGenerator
{
public:
  LithoMapsInfoGenerator(const SACScenario& scenario, ProjectReader &projectReader);
  void generateInfoTextFile() override;
};

} // namespace sac

} // namespace casaWizard
