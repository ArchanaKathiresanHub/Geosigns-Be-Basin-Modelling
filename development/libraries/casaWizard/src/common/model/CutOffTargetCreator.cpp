//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "CutOffTargetCreator.h"

#include "model/functions/filterData.h"

#include <QMap>

namespace casaWizard
{

CutOffTargetCreator::CutOffTargetCreator(const QMap<QString, QPair<double, double> >& propertiesWithCutOffRanges) :
   m_numberOfTargetsCutOff{0},
   m_propertiesWithCutOffRanges{propertiesWithCutOffRanges}
{

}

QVector<CalibrationTarget> CutOffTargetCreator::createManipulatedTargets(QVector<const CalibrationTarget*> oldTargets, const QString& property)
{
   // Create a copy of the remaining targets
   QVector<CalibrationTarget> newTargets;
   for (const CalibrationTarget* target : oldTargets)
   {
      if (target->value() < m_propertiesWithCutOffRanges[property].first ||
          target->value() > m_propertiesWithCutOffRanges[property].second)
      {
         m_numberOfTargetsCutOff++;
      }
      else
      {
         newTargets.push_back(*target);//Copy
      }
   }
   return newTargets;
}

QString CutOffTargetCreator::metaDataMessage(const QString& property)
{
   if (m_numberOfTargetsCutOff > 0)
   {
      return "Cut off " + QString::number(m_numberOfTargetsCutOff) + " targets of property " + property + " outside the range [" +
            QString::number(m_propertiesWithCutOffRanges[property].first) + ", " +
            QString::number(m_propertiesWithCutOffRanges[property].second) + "]" ;
   }

   return "";
}

} // namespace casaWizard

