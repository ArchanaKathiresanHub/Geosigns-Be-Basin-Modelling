//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Class for creating the optimized 3D case
#pragma once

#include "model/script/Generate3DScenarioScript.h"
#include "model/ThermalMapManager.h"

namespace casaWizard
{

namespace sac
{

namespace thermal
{

class ThermalScenario;

class Generate3DThermalScenarioScript : public Generate3DScenarioScript
{
public:
   explicit Generate3DThermalScenarioScript(const ThermalScenario& scenario);

private:
   const ThermalMapManager& mapsManager() const final;
   QString setFilterOneDResults() const final;

   const ThermalMapManager& m_mapManager;
};

} // namespace thermal

} // namespace sac

} // namespace casaWizard
