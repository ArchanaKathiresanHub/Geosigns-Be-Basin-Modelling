//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once
#include "model/sacLithologyScenario.h"
#include "model/ThermalScenario.h"
#include "stubProjectReader.h"

#include <memory>
namespace casaWizard
{

namespace sac
{

class StubSacLithologyScenario : public SacLithologyScenario
{
public:
  explicit StubSacLithologyScenario() :
    SacLithologyScenario(new StubProjectReader())
  {
  }
};

class StubThermalScenario : public thermal::ThermalScenario
{
public:
  explicit StubThermalScenario() :
    ThermalScenario(new StubProjectReader())
  {
  }
};


} // sac

} // casaWizard
