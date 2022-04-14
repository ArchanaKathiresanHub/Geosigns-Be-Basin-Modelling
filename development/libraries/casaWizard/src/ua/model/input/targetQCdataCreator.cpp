#include "targetQCdataCreator.h"

#include "model/input/dataFileParser.h"
#include "model/logger.h"
#include "model/targetParameterMapCreator.h"
#include "model/uaScenario.h"

#include <set>

#include <cmath>

namespace casaWizard
{

namespace ua
{

namespace targetQCdataCreator
{

const double epsilon{1e-16};

void readTargetQCs(UAScenario& scenario)
{
  if (scenario.workingDirectory().isEmpty())
  {
    Logger::log() << "No working directory present. Try to select a proper project3d file first." << Logger::endl();
    return;
  }

  const DataFileParser<double> parserRunCases(scenario.workingDirectory() + "/" + scenario.runCasesObservablesTextFileName());
  const DataFileParser<int> parserDoeIndices(scenario.workingDirectory() + "/" + scenario.doeIndicesTextFileName());
  const DataFileParser<double> parserProxyEvaluation(scenario.workingDirectory() + "/" + scenario.proxyEvaluationObservablesTextFileName());
  const DataFileParser<double> parserProxyQualityEvaluation(scenario.workingDirectory() + "/" + scenario.proxyQualityEvaluationTextFileName());

  const QVector<QVector<double>> runCasesObservables = parserRunCases.matrixData();
  const QVector<QVector<int>> doeIndices = parserDoeIndices.rowDominantMatrix();
  const QVector<QVector<double>> proxyEvaluationObservables = parserProxyEvaluation.matrixData();
  const QVector<QVector<double>> proxyQualityEvaluation = parserProxyQualityEvaluation.matrixData();

  if (runCasesObservables.size() == 0
   || runCasesObservables.size() != proxyEvaluationObservables.size()
   || runCasesObservables[0].size() == 0
   || proxyEvaluationObservables[0].size() == 0)
  {
    Logger::log() << "Incompatible size of run case and proxy evaluated observables" << Logger::endl();
    return;
  }

  const QVector<int> indicesOfQcDoeOptionSelInDoeOptionTable = scenario.indicesOfSelectedQCDoeOptionsInSelectedDoeOptions();
  QVector<QVector<int>> tmpIndicesOfRunCasesForQC;
  for (const int& iDoe : indicesOfQcDoeOptionSelInDoeOptionTable)
  {
    tmpIndicesOfRunCasesForQC.push_back(doeIndices[iDoe]);
  }
  std::set<int> setOfindicesOfRunCasesForQC;
  for (const QVector<int>& indRQCVec : tmpIndicesOfRunCasesForQC)
  {
    for (const int& indRQC : indRQCVec)
    {
      setOfindicesOfRunCasesForQC.insert(indRQC);
    }
  }

  QVector<QVector<double>> runCasesObservablesOfTargetQC;
  for (int i = 0; i < runCasesObservables.size(); ++i)
  {
    QVector<double> rowrunCase;
    for (const int& indSetRQC : setOfindicesOfRunCasesForQC)
    {
      rowrunCase.push_back(runCasesObservables[i][indSetRQC]);
    }
    runCasesObservablesOfTargetQC.push_back(rowrunCase);
  }

  const int nTargets{runCasesObservables.size()};

  const CalibrationTargetManager& calibrationTargetManager = scenario.calibrationTargetManager();
  const PredictionTargetManager& predictionTargetManager = scenario.predictionTargetManager();

  const int nActiveCalibrationTargets = calibrationTargetManager.activeCalibrationTargets().size();
  const int nRequired = nActiveCalibrationTargets + predictionTargetManager.amountIncludingTimeSeries();
  if (nTargets != nRequired)
  {
    Logger::log() << "Incompatible size of observable imported (" << nTargets << ") and specified (" << nRequired << ")." << Logger::endl();
    return;
  }

  int targetIndex = 0;
  QVector<TargetQC> targetQCs;

  for (const CalibrationTarget* calibrationTarget : calibrationTargetManager.activeCalibrationTargets())
  {
    TargetQC targetQC(targetIndex,
                      calibrationTargetManager.getCauldronPropertyName(calibrationTarget->propertyUserName()),
                      calibrationTarget->name(),
                      true, // is calibration
                      calibrationTarget->value(),
                      calibrationTarget->standardDeviation(),
                      proxyQualityEvaluation[targetIndex][0], // R2
                      proxyQualityEvaluation[targetIndex][1], // R2Adj
                      proxyQualityEvaluation[targetIndex][2], // Q2
                      runCasesObservablesOfTargetQC[targetIndex],
                      proxyEvaluationObservables[targetIndex]);
    targetQCs.push_back(targetQC);
    targetIndex++;
  }

  for (const PredictionTarget* const predictionTarget : predictionTargetManager.predictionTargetsIncludingTimeSeries())
  {
    TargetQC targetQC(targetIndex,
                      predictionTarget->property(),
                      predictionTarget->name(),
                      false, // is not calibration
                      0.0, // value
                      0.0, // standard deviation
                      proxyQualityEvaluation[targetIndex][0], // R2
                      proxyQualityEvaluation[targetIndex][1], // R2Adj
                      proxyQualityEvaluation[targetIndex][2], // Q2
                      runCasesObservablesOfTargetQC[targetIndex],
                      proxyEvaluationObservables[targetIndex]);
    targetQCs.push_back(targetQC);
    targetIndex++;
  }

  std::sort(targetQCs.begin(), targetQCs.end(), [](const TargetQC& a, const TargetQC& b)
  {
    if (std::fabs(a.R2()) < epsilon)
    {
      return false;
    }
    if (std::fabs(b.R2()) < epsilon)
    {
      return true;
    }
    return a.R2() < b.R2() ;
  });

  scenario.setTargetQCs(targetQCs);
}

} // namespace targetQCdataCreator

} // namespace ua

} // namespace casaWizard
