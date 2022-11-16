#include "targetQCdataCreator.h"

#include "model/input/dataFileParser.h"
#include "model/logger.h"
#include "model/targetParameterMapCreator.h"
#include "model/uaScenario.h"

#include "Qt_Utils.h"

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

   QString obsFileName = scenario.workingDirectory() + "/" + scenario.runCasesObservablesTextFileName();
   QString doeIndicesFileName = scenario.workingDirectory() + "/" + scenario.doeIndicesTextFileName();
   QString proxyEvalFileName = scenario.workingDirectory() + "/" + scenario.proxyEvaluationObservablesTextFileName();
   QString proxyQualityFileName = scenario.workingDirectory() + "/" + scenario.proxyQualityEvaluationTextFileName();

   QVector<QString> obsIdentifiersRunCaseObservables;
   const QVector<QVector<double>> runCasesObservables = DataFileParser<double>::parseMatrixFileWithHeaderColDominant(obsFileName,obsIdentifiersRunCaseObservables);
   const QVector<QVector<int>> doeIndices = DataFileParser<int>::readFile(doeIndicesFileName);

   QVector<QString> obsIdentifiersEvalObservables;
   QVector<QVector<double>> proxyEvaluationObservables = DataFileParser<double>::parseMatrixFileWithHeaderColDominant(proxyEvalFileName,obsIdentifiersEvalObservables);

   QVector<QString> obsIdentifiersProxyQuality;
   const QVector<QVector<double>> proxyQualityEvaluation = DataFileParser<double>::parseMatrixFileWithHeaderColDominant(proxyQualityFileName,obsIdentifiersProxyQuality);

   if (!qtutils::isEqual(obsIdentifiersRunCaseObservables,obsIdentifiersEvalObservables)
       || !qtutils::isEqual(obsIdentifiersEvalObservables, obsIdentifiersProxyQuality))
   {
      Logger::log() << "Incompatible header names in observables, proxy observables, or proxyQuality files." << Logger::endl();
      return;
   }

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
      if (iDoe >= doeIndices.size())
      {
         Logger::log() << "Incompatible size of DoE indices returned from casa and the DoEs selected in the wizard" << Logger::endl();
         return;
      }
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

   QString stateFileName = scenario.workingDirectory() + "/" + scenario.simStatesTextFileName();
   const QVector<int> simStates = DataFileParser<int>::colDominantMatrix(stateFileName).at(0);
   QVector<int> simStatesOfTargetQC;
   for (const int& indSetRQC : setOfindicesOfRunCasesForQC)
   {
      simStatesOfTargetQC.push_back(simStates[indSetRQC]);
   }

   removeObservablesFailedSimulations(runCasesObservablesOfTargetQC,simStatesOfTargetQC);
   removeObservablesFailedSimulations(proxyEvaluationObservables,simStatesOfTargetQC);

   const int nTargets{runCasesObservables.size()};

   const CalibrationTargetManager& calibrationTargetManager = scenario.calibrationTargetManager();
   const PredictionTargetManager& predictionTargetManager = scenario.predictionTargetManager();

   const int nActiveCalibrationTargets = calibrationTargetManager.activeCalibrationTargets().size();
   const int nRequired = nActiveCalibrationTargets + predictionTargetManager.amountOfPredictionTargetWithTimeSeriesAndProperties();
   if (nTargets != nRequired)
   {
      Logger::log() << "Incompatible size of observable imported (" << nTargets << ") and specified (" << nRequired << ")." << Logger::endl();
      return;
   }

   int targetIndex = 0;
   QVector<TargetQC> targetQCs;

   auto tryAddBaseCase = [&](TargetQC& target, int targetIndex)
   {
      const int idx = scenario.qcDoeOptionSelectedNames().indexOf("BaseCase");
      if (idx < 0)
      {
         return;
      }
      const int caseIdx = doeIndices.at(idx).at(0);
      target.setValBaseSim(runCasesObservablesOfTargetQC[targetIndex][caseIdx]);
      target.setValBaseProxy(proxyEvaluationObservables[targetIndex][caseIdx]);
   };

   for (const CalibrationTarget* calibrationTarget : calibrationTargetManager.activeCalibrationTargets())
   {
      TargetQC targetQC(targetIndex,
                        calibrationTargetManager.getCauldronPropertyName(calibrationTarget->propertyUserName()),
                        calibrationTarget->name(),
                        obsIdentifiersRunCaseObservables[targetIndex],
                        true, // is calibration
                        calibrationTarget->value(),
                        calibrationTarget->standardDeviation(),
                        proxyQualityEvaluation[targetIndex][0], // R2
                        proxyQualityEvaluation[targetIndex][1], // R2Adj
                        proxyQualityEvaluation[targetIndex][2], // Q2
                        runCasesObservablesOfTargetQC[targetIndex],
                        proxyEvaluationObservables[targetIndex]);

      tryAddBaseCase(targetQC, targetIndex);

      targetQCs.push_back(targetQC);
      targetIndex++;
   }

   for (const PredictionTarget* const predictionTarget : predictionTargetManager.predictionTargetsIncludingTimeSeries())
   {
      for (const QString& property : predictionTarget->properties())
      {
         QString targetId = predictionTarget->identifier(property);
         QString obsId = obsIdentifiersEvalObservables[targetIndex];
         if (targetId == obsId)
         {
            TargetQC targetQC(targetIndex,
                              property,
                              predictionTarget->name(property),
                              targetId,
                              false, // is not calibration
                              0.0, // value
                              0.0, // standard deviation
                              proxyQualityEvaluation[targetIndex][0], // R2
                              proxyQualityEvaluation[targetIndex][1], // R2Adj
                              proxyQualityEvaluation[targetIndex][2], // Q2
                              runCasesObservablesOfTargetQC[targetIndex],
                              proxyEvaluationObservables[targetIndex]);

            tryAddBaseCase(targetQC, targetIndex);

            targetQCs.push_back(targetQC);
         }
         else
         {
            Logger::log() << "Warning: Mismatch target identifier: " <<  targetId << " and observable Id: " << obsId << " Qc target is not created." << Logger::endl();
         }

         targetIndex++;
      }
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

void removeObservablesFailedSimulations(QVector<QVector<double>>& observables, const QVector<int>& simulationStates )
{
   for (int i = 0; i < observables.size(); i++)
   {
      const QVector<double>& obsSingleTarget = observables[i];

      if (obsSingleTarget.size() != simulationStates.size())
      {
         throw std::runtime_error("Obsevables data does not match with simulation states data.");
      }

      QVector<double> validObs;
      for (int j = 0; j < simulationStates.size(); j++)
      {
         if (simulationStates[j] == 1) //valid
         {
            validObs.push_back(obsSingleTarget[j]);
         }
      }
      observables[i] = validObs;
   }
}

} // namespace targetQCdataCreator

} // namespace ua

} // namespace casaWizard
