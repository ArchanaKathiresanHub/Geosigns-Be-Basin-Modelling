//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <vector>

namespace casaWizard
{

class DTToTwoWayTimeConverter
{
public:
  DTToTwoWayTimeConverter();
  std::vector<double> convertToTWT(const std::vector<double>& SonicSlownessWellData, const std::vector<double>& depthWellData,
                                   const std::vector<double>& TwoWayTimeModelData, const std::vector<double>& depthTrajectoryModelData) const;
private:
  double computeCurrentTwoWayTimeContribution(const double previousDepth, const double currentDepth,
                                              const double previousSonicSlowness, const double currentSonicSlowness) const;
};

} // namespace casaWizard
