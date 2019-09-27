#include "caseDataCreator.h"

#include "model/calibrationTargetManager.h"
#include "model/sacScenario.h"
#include "model/functions/sortedByXWellIndices.h"

namespace casaWizard
{

namespace sac
{

CaseDataCreator::CaseDataCreator(const CalibrationTargetManager& calibrationTargetManager, const QString& iterationPath) :
  calibrationTargetManager_{calibrationTargetManager},
  iterationPath_{iterationPath}
{
}

void CaseDataCreator::read()
{
  int caseIndex{0};
  const QVector<int> sortedIndices = casaWizard::functions::sortedByXWellIndices(calibrationTargetManager_.wells());
  for (const int wellIndex : sortedIndices)
  {
    if (calibrationTargetManager_.well(wellIndex).isActive())
    {
      readCase(wellIndex, ++caseIndex);
    }
  }
}

const CalibrationTargetManager& CaseDataCreator::calibrationTargetManager() const
{
  return calibrationTargetManager_;
}

const QString& CaseDataCreator::iterationPath() const
{
  return iterationPath_;
}

} // namespace sac

} // namespace casaWizard
