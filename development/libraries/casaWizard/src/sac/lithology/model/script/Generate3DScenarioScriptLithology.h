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
#include "model/MapsManagerLithology.h"

namespace casaWizard
{

namespace sac
{

class SacLithologyScenario;

class Generate3DScenarioScriptLithology : public Generate3DScenarioScript
{
public:
   explicit Generate3DScenarioScriptLithology(const SacLithologyScenario& scenario);

private:
   const MapsManagerLithology& mapsManager() const override;
   QString setFilterOneDResults() const;

   const MapsManagerLithology& m_mapsManager;
};

} // namespace sac

} // namespace casaWizard
