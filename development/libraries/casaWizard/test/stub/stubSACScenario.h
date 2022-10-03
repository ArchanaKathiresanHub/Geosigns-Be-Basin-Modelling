//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once
#include "model/sacLithologyScenario.h"
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

} // sac

} // casaWizard
