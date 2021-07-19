//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "wellValidator.h"

#include "ConstantsNumerical.h"
#include "model/input/cmbMapReader.h"
#include "model/well.h"

#include <cmath>

namespace casaWizard
{

WellValidator::WellValidator(CMBMapReader& mapReader) :
  mapReader_{mapReader}
{
}

WellValidator::~WellValidator()
{
}

WellState WellValidator::wellState(const casaWizard::Well& well, const std::string& depthGridName) const
{
  const double valueAtWellLocation = mapReader_.getValue(well.x(), well.y(), depthGridName);

  if (std::fabs(valueAtWellLocation - Utilities::Numerical::CauldronNoDataValue) <= 1e-5 )
  {
    return invalidLocation;
  }
  else if (well.calibrationTargets().size() == 0)
  {
    return invalidData;
  }

  return valid;
}

} // namespace casaWizard
