//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "model/output/SacInfoGenerator.h"
#include "model/MapsManagerLithology.h"
#include "model/sacLithologyScenario.h"

namespace casaWizard
{

namespace sac
{

class SacInfoGeneratorLithology : public SacInfoGenerator
{
public:
   SacInfoGeneratorLithology(const SacLithologyScenario& scenario, ProjectReader &projectReader);
   virtual ~SacInfoGeneratorLithology() override = default;
   void addMapsGenerationSection() override;

protected:
   const SacLithologyScenario& scenario() const final;

private:
   const MapsManagerLithology& mapsManager() const final;

   void addSmartGridding();

   const MapsManagerLithology& m_mapsManager;
   const SacLithologyScenario& m_scenario;
};

} // namespace sac

} // namespace casaWizard
