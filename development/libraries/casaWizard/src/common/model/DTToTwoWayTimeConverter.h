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
   static std::vector<double> convertToTWT(const std::vector<double>& sonicSlownessWellData, const std::vector<double>& depthWellData,
                                           const std::vector<double>& TwoWayTimeModelData, const std::vector<double>& depthTrajectoryModelData);

private:
   DTToTwoWayTimeConverter(const std::vector<double>& sonicSlownessWellData, const std::vector<double>& depthWellData, const std::vector<double>& TwoWayTimeModelData,
                           const std::vector<double>& depthTrajectoryModelData);

   std::vector<double> convertToTWT() const;
   double computeCurrentTwoWayTimeContribution(const double previousDepth, const double currentDepth,
                                               const double previousSonicSlowness, const double currentSonicSlowness) const;
   std::vector<double> calculateTwoWayTime() const;
   void alignTwoWayTimeToModelData(std::vector<double>& TWT) const;
   double shiftTopOfWellDataToModelData() const;
   double shiftAlignTwoWayTimeWellDataToTopOfModelData(const std::vector<double>& outputTwoWayTime) const;
   bool topOfModelDataIsHigherThanTopOfWellData() const;

   static double interpolateVectorToDepth(const std::vector<double>& values, const std::vector<double> depths, const double interpolatedDepth);

   const std::vector<double>& m_sonicSlownessWellData;
   const std::vector<double>& m_depthWellData;
   const std::vector<double>& m_twoWayTimeModelData;
   const std::vector<double>& m_depthTrajectoryModelData;
};

} // namespace casaWizard
