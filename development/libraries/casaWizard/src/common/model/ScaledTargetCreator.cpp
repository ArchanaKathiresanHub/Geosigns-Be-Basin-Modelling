//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ScaledTargetCreator.h"

#include "model/functions/filterData.h"

namespace casaWizard
{

ScaledTargetCreator::ScaledTargetCreator(const double scalingFactor) :
   m_scalingFactor{scalingFactor}
{

}

QVector<CalibrationTarget> ScaledTargetCreator::createManipulatedTargets(QVector<const CalibrationTarget*> oldTargets, const QString& /*property*/)
{
   QVector<double> scaledValues;
   QVector<double> scaledStandardDeviations;
   for (const CalibrationTarget* target : oldTargets)
   {
      scaledStandardDeviations.push_back(target->standardDeviation()*m_scalingFactor);
      scaledValues.push_back(target->value()*m_scalingFactor);
   }

   // Create new targets with the new data
   int i = 0;
   QVector<CalibrationTarget> newTargets;
   for (const CalibrationTarget* target : oldTargets)
   {
      CalibrationTarget newTarget(target->name(), target->propertyUserName(), target->z(), scaledValues.at(i), scaledStandardDeviations.at(i), target->uaWeight());
      newTargets.push_back(newTarget);
      ++i;
   }

   return newTargets;
}

QString ScaledTargetCreator::metaDataMessage(const QString& property)
{
   return "Scaling with scaling factor " + QString::number(m_scalingFactor) + " applied to " + property + " targets";
}

} // namespace casaWizard

