//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "model/output/SacInfoGenerator.h"
#include "model/LithologyMapsManager.h"
#include "model/LithologyScenario.h"

namespace casaWizard
{

namespace sac
{

namespace lithology
{

class LithologyInfoGenerator : public SacInfoGenerator
{
public:
   LithologyInfoGenerator(const LithologyScenario& scenario, ProjectReader &projectReader);
   virtual ~LithologyInfoGenerator() override = default;
   void addMapsGenerationSection() override;

protected:
   const LithologyScenario& scenario() const final;

private:
   const LithologyMapsManager& mapsManager() const final;

   void addOptimizationOptionsInfo() final;
   void addSmartGridding();

   const LithologyMapsManager& m_mapsManager;
   const LithologyScenario& m_scenario;
};

} // namespace lithology

} // namespace sac

} // namespace casaWizard
