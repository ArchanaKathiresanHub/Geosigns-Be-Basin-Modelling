//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "vpToDTConverter.h"

#include "ConstantsMathematics.h"
#include "ConstantsNumerical.h"

#include <cmath>

namespace casaWizard
{


VPToDTConverter::VPToDTConverter()
{

}

std::vector<double> VPToDTConverter::convertToDT(const std::vector<double>& velocityVector) const
{
  std::vector<double> sonicSlowness;
  for (const double velocity : velocityVector)
  {
    if (velocity <= 0 || std::fabs(velocity - Utilities::Numerical::CauldronNoDataValue) < 1e-5)
    {
      sonicSlowness.push_back(Utilities::Numerical::CauldronNoDataValue);
    }
    else
    {
      sonicSlowness.push_back(Utilities::Maths::SecondToMicroSecond * (1.0 / velocity));
    }
  }

  return sonicSlowness;
}

} // namespace casaWizard
