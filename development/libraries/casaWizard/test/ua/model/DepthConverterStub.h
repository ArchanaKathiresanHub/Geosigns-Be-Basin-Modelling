//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "model/ToDepthConverter.h"

namespace casaWizard
{
namespace ua
{
class DepthConverterStub : public ToDepthConverter
{
public:
   double getDepth(double /*x*/, double /*y*/, QString /*surfaceName*/) const
   {
      return 12.0;
   }
};
}
}
