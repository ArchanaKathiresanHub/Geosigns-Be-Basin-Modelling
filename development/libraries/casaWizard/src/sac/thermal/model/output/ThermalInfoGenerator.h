//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "model/output/SacInfoGenerator.h"
#include "model/ThermalMapManager.h"
#include "model/ThermalScenario.h"

namespace casaWizard
{

namespace sac
{

namespace thermal
{
class ThermalInfoGenerator : public SacInfoGenerator
{
public:
   ThermalInfoGenerator(const ThermalScenario& scenario,
                        std::unique_ptr<ProjectReader> uniqueProjectReader);
   virtual ~ThermalInfoGenerator() override = default;

protected:
   const ThermalScenario& scenario() const final;

private:
   void addOptimizationOptionsInfo() final;
   const ThermalMapManager& mapsManager() const final;

   const ThermalMapManager& m_mapManager;
   const ThermalScenario& m_scenario;

   std::unique_ptr<ProjectReader> m_projectReader;
};

} // namespace thermal

} // namespace sac

} // namespace casaWizard
