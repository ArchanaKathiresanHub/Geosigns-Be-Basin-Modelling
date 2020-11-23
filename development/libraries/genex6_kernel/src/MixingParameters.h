// Copyright (C) 2012-2020 Shell International Exploration & Production.
// All rights reserved.

// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.

#pragma once

// utilities library
#include "ConstantsNumerical.h"
using Utilities::Numerical::CauldronNoDataValue;

namespace Genex6
{

struct MixingParameters
{
  double maximumTimeStepSize1;
  int numberOfTimeSteps1;
  double hcValue1;

  double maximumTimeStepSize2;
  int numberOfTimeSteps2;
  double hcValue2;

  MixingParameters() :
    maximumTimeStepSize1{CauldronNoDataValue},
    numberOfTimeSteps1{0},
    hcValue1{CauldronNoDataValue},
    maximumTimeStepSize2{CauldronNoDataValue},
    numberOfTimeSteps2{0},
    hcValue2{CauldronNoDataValue}
  {}
};
}
