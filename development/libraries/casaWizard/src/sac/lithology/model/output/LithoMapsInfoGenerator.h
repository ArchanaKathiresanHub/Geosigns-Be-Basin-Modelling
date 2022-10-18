//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "SacInfoGeneratorLithology.h"

namespace casaWizard
{

class CMBProjectReader;

namespace sac
{

class SacLithologyScenario;

class LithoMapsInfoGenerator : public SacInfoGeneratorLithology
{
public:
   LithoMapsInfoGenerator(const SacLithologyScenario& scenario, std::unique_ptr<ProjectReader> projectReader);
   virtual ~LithoMapsInfoGenerator();
   void generateInfoTextFile() override;

private:
   std::unique_ptr<ProjectReader> m_projectReader;
};

} // namespace sac

} // namespace casaWizard
