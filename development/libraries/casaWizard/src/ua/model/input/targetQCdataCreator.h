// Function to populate target QC data.
#pragma once

#include <QVector>

namespace casaWizard
{

namespace ua
{

class UAScenario;

namespace targetQCdataCreator
{

  void readTargetQCs(ua::UAScenario& scenario);
  void removeObservablesFailedSimulations(QVector<QVector<double>>& observables, const QVector<int>& simulationStates );

} // namespace targetQCdataCreator

} // namespace ua

} // namespace casaWizard
