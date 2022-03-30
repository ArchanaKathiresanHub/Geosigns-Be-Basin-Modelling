//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SubsampledTargetCreator.h"

#include "model/functions/filterData.h"

namespace casaWizard
{

SubsampledTargetCreator::SubsampledTargetCreator(const double length) :
   m_length{length}
{

}

QVector<CalibrationTarget> SubsampledTargetCreator::createManipulatedTargets(QVector<const CalibrationTarget*> oldTargets, const QString& /*property*/)
{
   // Extract vectors of depth
   QVector<double> depths;
   for (const CalibrationTarget* target : oldTargets)
   {
      depths.push_back(target->z());
   }

   // Subsampling this data
   const QVector<int> subsampledIndicesRemaining = functions::subsampleData(depths, m_length);

   // Create a copy of the remaining targets
   QVector<CalibrationTarget> newTargets;
   for (const int index : subsampledIndicesRemaining)
   {
      const CalibrationTarget* target = oldTargets[index];
      const CalibrationTarget newTarget = *target; //Copy
      newTargets.push_back(newTarget);
   }

   return newTargets;
}

QString SubsampledTargetCreator::metaDataMessage(const QString& property)
{
   return "Subsampling with length " + QString::number(m_length) + " applied to " + property + " targets";
}

} // namespace casaWizard

