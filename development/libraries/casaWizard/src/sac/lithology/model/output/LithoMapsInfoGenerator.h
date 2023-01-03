//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "LithologyInfoGenerator.h"

namespace casaWizard
{

class CMBProjectReader;

namespace sac
{

namespace lithology
{

class LithologyScenario;

class LithoMapsInfoGenerator : public LithologyInfoGenerator
{
public:
   LithoMapsInfoGenerator(const LithologyScenario& scenario, std::unique_ptr<ProjectReader> projectReader);
   virtual ~LithoMapsInfoGenerator() override = default;

private:
   std::unique_ptr<ProjectReader> m_projectReader;
};

} // namespace lithology

} // namespace sac

} // namespace casaWizard
