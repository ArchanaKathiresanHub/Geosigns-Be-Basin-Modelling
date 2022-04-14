//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once
#include "model/uaScenario.h"
#include "stubProjectReader.h"

#include <memory>
namespace casaWizard
{

namespace ua
{

class StubUAScenario : public UAScenario
{
public:
  explicit StubUAScenario() :
    UAScenario(new StubProjectReader())
  {
  }
};

} // sac

} // casaWizard
