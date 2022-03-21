//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "DTToTwoWayTimeConverter.h"

#include "ConstantsMathematics.h"

#include <cmath>

namespace casaWizard
{

std::vector<double> DTToTwoWayTimeConverter::convertToTWT(const std::vector<double>& sonicSlownessWellData, const std::vector<double>& depthWellData, const std::vector<double>& TwoWayTimeModelData, const std::vector<double>& depthTrajectoryModelData)
{
   if (sonicSlownessWellData.size() < 2 || depthWellData.size() < 2)
   {
      return {};
   }

   DTToTwoWayTimeConverter converter(sonicSlownessWellData, depthWellData, TwoWayTimeModelData, depthTrajectoryModelData);
   return converter.convertToTWT();
}

DTToTwoWayTimeConverter::DTToTwoWayTimeConverter(const std::vector<double>& sonicSlownessWellData, const std::vector<double>& depthWellData,
                                                 const std::vector<double>& TwoWayTimeModelData, const std::vector<double>& depthTrajectoryModelData) :
   m_sonicSlownessWellData{sonicSlownessWellData},
   m_depthWellData{depthWellData},
   m_twoWayTimeModelData{TwoWayTimeModelData},
   m_depthTrajectoryModelData{depthTrajectoryModelData}
{
}

std::vector<double> DTToTwoWayTimeConverter::convertToTWT() const
{
   std::vector<double> TWT = calculateTwoWayTime();
   alignTwoWayTimeToModelData(TWT);

   return TWT;
}

std::vector<double> DTToTwoWayTimeConverter::calculateTwoWayTime() const
{
   std::vector<double> TWT;

   double cumulativeTwoWayTime = 0;
   TWT.push_back(cumulativeTwoWayTime);
   for (unsigned int i = 1; i < m_sonicSlownessWellData.size(); i++)
   {
      cumulativeTwoWayTime += computeCurrentTwoWayTimeContribution(m_depthWellData[i-1], m_depthWellData[i], m_sonicSlownessWellData[i-1], m_sonicSlownessWellData[i]);
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

void DTToTwoWayTimeConverter::alignTwoWayTimeToModelData(std::vector<double>& outputTwoWayTime) const
{
   if (m_depthTrajectoryModelData.size() < 2 || m_twoWayTimeModelData.size() < 2)
   {
      return;
   }

   double alignmentShift = 0;
   if (topOfModelDataIsHigherThanTopOfWellData())
   {
      alignmentShift = shiftTopOfWellDataToModelData();
   }
   else
   {
      alignmentShift = shiftAlignTwoWayTimeWellDataToTopOfModelData(outputTwoWayTime);
   }

   for (double& TWTPoint : outputTwoWayTime)
   {
      TWTPoint += alignmentShift;
   }
}

bool DTToTwoWayTimeConverter::topOfModelDataIsHigherThanTopOfWellData() const
{
   return m_depthTrajectoryModelData[0] <= m_depthWellData[0];
}

double DTToTwoWayTimeConverter::shiftTopOfWellDataToModelData() const
{
   return interpolateVectorToDepth(m_twoWayTimeModelData, m_depthTrajectoryModelData, m_depthWellData[0]);
}

double DTToTwoWayTimeConverter::interpolateVectorToDepth(const std::vector<double>& values, const std::vector<double> depths, const double interpolatedDepth)
{
   for (unsigned int i = 1; i < values.size(); i++)
   {
      if (depths[i] >= interpolatedDepth)
      {
         if (depths[i-1] <= interpolatedDepth)
         {
            // Interpolate the value to the specified depth (interpolatedDepth).
            return values[i-1] +
                  (interpolatedDepth - depths[i-1]) / (depths[i] - depths[i-1]) * (values[i] - values[i-1]);
         }
         break;
      }
   }

   return 0.0;
}

double DTToTwoWayTimeConverter::shiftAlignTwoWayTimeWellDataToTopOfModelData(const std::vector<double>& outputTwoWayTime) const
{
   double TwoWayTimeWellAtTopOfModel = interpolateVectorToDepth(outputTwoWayTime, m_depthWellData, m_depthTrajectoryModelData[0]);
   return m_twoWayTimeModelData[0] - TwoWayTimeWellAtTopOfModel;
}

} // namespace casaWizard
