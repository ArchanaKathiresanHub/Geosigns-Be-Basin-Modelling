//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "caseDataCreator.h"

#include "model/calibrationTargetManager.h"
#include "model/functions/sortedByXWellIndices.h"

#include <QVector>

namespace casaWizard
{

namespace sac
{

CaseDataCreator::CaseDataCreator(const CalibrationTargetManager& calibrationTargetManager, const QString& iterationPath) :
  m_calibrationTargetManager{calibrationTargetManager},
  m_iterationPath{iterationPath}
{
}

void CaseDataCreator::read()
{
  int caseIndex{0};
  const QVector<int> sortedIndices = casaWizard::functions::sortedByXYWellIndices(m_calibrationTargetManager.wells());
  for (const int wellIndex : sortedIndices)
  {
    if ( m_calibrationTargetManager.well(wellIndex).isIncludedInOptimization() )
    {
      readCase(wellIndex, ++caseIndex);
    }
  }
}

const CalibrationTargetManager& CaseDataCreator::calibrationTargetManager() const
{
  return m_calibrationTargetManager;
}

const QString& CaseDataCreator::iterationPath() const
{
  return m_iterationPath;
}

} // namespace sac

} // namespace casaWizard
