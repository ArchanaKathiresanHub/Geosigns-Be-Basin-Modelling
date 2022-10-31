//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Class for creating the optimized 3D case
#pragma once

#include "model/script/Generate3DScenarioScript.h"
#include "model/LithologyMapsManager.h"

namespace casaWizard
{

namespace sac
{

class LithologyScenario;

class Generate3DScenarioScriptLithology : public Generate3DScenarioScript
{
public:
   explicit Generate3DScenarioScriptLithology(const LithologyScenario& scenario);

private:
   const LithologyMapsManager& mapsManager() const final;
   QString setFilterOneDResults() const final;

   const LithologyMapsManager& m_mapsManager;
};

} // namespace sac

} // namespace casaWizard
