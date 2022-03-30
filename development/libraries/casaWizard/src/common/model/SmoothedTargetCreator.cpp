//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SmoothedTargetCreator.h"

#include "model/functions/filterData.h"

namespace casaWizard
{

SmoothedTargetCreator::SmoothedTargetCreator(const double radius) :
   m_radius{radius}
{

}

QVector<CalibrationTarget> SmoothedTargetCreator::createManipulatedTargets(QVector<const CalibrationTarget*> oldTargets, const QString&)
{
   // Extract vectors of depth and the corresponding values
   QVector<double> depths;
   QVector<double> values;
   for (const CalibrationTarget* target : oldTargets)
   {
      depths.push_back(target->z());
      values.push_back(target->value());
   }

   // Smoothing this data
   const QVector<double> smoothenedData = functions::smoothenData(depths, values, m_radius);

   // Create new targets with the new data
   int i = 0;
   QVector<CalibrationTarget> newTargets;
   for (const CalibrationTarget* target : oldTargets)
   {
      CalibrationTarget newTarget(target->name(), target->propertyUserName(), target->z(), smoothenedData[i], target->standardDeviation(), target->uaWeight());
      newTargets.push_back(newTarget);
      ++i;
   }

   return newTargets;
}

QString SmoothedTargetCreator::metaDataMessage(const QString& property)
{
   return "Gaussian smoothing with radius " + QString::number(m_radius) + " applied to " + property + " targets";
}

} // namespace casaWizard

