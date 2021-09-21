//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "dtToTwoWayTimeConverter.h"

#include "ConstantsMathematics.h"

#include <cmath>

namespace casaWizard
{

DTToTwoWayTimeConverter::DTToTwoWayTimeConverter()
{

}

std::vector<double> DTToTwoWayTimeConverter::convertToTWT(const std::vector<double>& SonicSlownessWellData, const std::vector<double>& depthWellData,
                                                          const std::vector<double>& TwoWayTimeModelData, const std::vector<double>& depthTrajectoryModelData) const
{
  // Calculate offset in TwoWayTime from the top of the model to the first data-point
  // using the supplementary model data
  double cumulativeTwoWayTime = 0;
  for (unsigned int i = 1; i < TwoWayTimeModelData.size(); i++)
  {
    if (depthTrajectoryModelData[i] >= depthWellData[0])
    {
      if (depthTrajectoryModelData[i-1] < depthWellData[0])
      {
        // Interpolate the TwoWayTime to the first well data point.
        cumulativeTwoWayTime = TwoWayTimeModelData[i-1] +
                (depthWellData[0] - depthTrajectoryModelData[i-1]) / (depthTrajectoryModelData[i] - depthTrajectoryModelData[i-1]) * (TwoWayTimeModelData[i] - TwoWayTimeModelData[i-1]);
      }
      else
      {
        cumulativeTwoWayTime = TwoWayTimeModelData[i-1];
      }
      break;
    }
  }
  std::vector<double> TWT;
  TWT.push_back(cumulativeTwoWayTime);
  for (unsigned int i = 1; i < SonicSlownessWellData.size(); i++)
  {
    cumulativeTwoWayTime += computeCurrentTwoWayTimeContribution(depthWellData[i-1], depthWellData[i], SonicSlownessWellData[i-1], SonicSlownessWellData[i]);
    TWT.push_back(cumulativeTwoWayTime);
  }

  return TWT;
}

double DTToTwoWayTimeConverter::computeCurrentTwoWayTimeContribution(const double previousDepth, const double currentDepth,
                                                                     const double previousSonicSlowness, const double currentSonicSlowness) const
{
  // Units:
  // DT : us/m
  // depth : m
  // TWT : ms

  // DT * depth -> us/m * m = us, so we need to multiply with 1e-3 to get ms for the TWT
  // Also note we need the factor 2 to calculate TwoWayTime
  return 2 * ((previousSonicSlowness + currentSonicSlowness) / 2 * std::fabs(previousDepth - currentDepth) * 1e-3);
}


} // namespace casaWizard
