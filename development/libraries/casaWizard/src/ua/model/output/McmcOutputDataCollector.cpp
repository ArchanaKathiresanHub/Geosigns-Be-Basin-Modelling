//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "McmcOutputDataCollector.h"
#include "McmcTargetExportData.h"

#include "model/doeOption.h"
#include "model/input/dataFileParser.h"
#include "model/logger.h"
#include "model/monteCarloDataManager.h"
#include "model/percentileGenerator.h"
#include "model/script/runOptimalCaseScript.h"
#include "model/uaScenario.h"


#include <QMap>

#include <math.h>

namespace casaWizard
{

namespace ua
{

McmcOutputDataCollector::McmcOutputDataCollector(const UAScenario& scenario):
   m_scenario(scenario)
{}

McmcTargetExportData McmcOutputDataCollector::collectMcmcOutputData(const UAScenario& scenario)
{
   McmcOutputDataCollector collector(scenario);
   collector.collectMcmcOutputDataPrivate();
   return collector.m_mcmcTargetExportData;
}

void McmcOutputDataCollector::collectMcmcOutputDataPrivate()
{
   readBaseCase();
   readOptimalCase();
   collectPredictionTargetData();
}

void McmcOutputDataCollector::readBaseCase()
{
   const QVector<DoeOption*> selectedOptions = m_scenario.doeSelected();

   const QStringList optionsWithBaseCasePoint = {"Tornado", "BoxBehnken", "FullFactorial"};

   bool baseCaseDesignPointPresent(false);
   for (const DoeOption* option : selectedOptions)
   {
      if (!option->hasCalculatedDesignPoints())
      {
         continue;
      }

      if (optionsWithBaseCasePoint.contains(option->name()))
      {
         baseCaseDesignPointPresent = true;
         break;
      }
      else
      {
         break;
      }
   }

   if (!baseCaseDesignPointPresent)
   {
      Logger::log() << "Export MCMC data: First DoE option not guaranteed to contain base case point. Base case data is not exported." << Logger::endl();
      return;
   }

   const QVector<TargetQC>& targetQcs = m_scenario.targetQCs();
   if (targetQcs.size() > 0)
   {
      readValuesFromTargetQcs();
   }
   else
   {
      readValuesFromObservableFiles();
   }
}

void McmcOutputDataCollector::readValuesFromObservableFiles()
{
   QString fileName = m_scenario.workingDirectory() + "/" + m_scenario.runCasesObservablesTextFileName();
   m_baseCaseSimValues = readPredictionTargetsFirstRowObservables(fileName);

   fileName = m_scenario.workingDirectory() + "/" + m_scenario.proxyEvaluationObservablesTextFileName();
   m_baseCaseProxyValues = readPredictionTargetsFirstRowObservables(fileName);
}

void McmcOutputDataCollector::readValuesFromTargetQcs()
{
   const QVector<TargetQC>& targetQcs = m_scenario.targetQCs();

   const QVector<int> idxRange = m_scenario.predictionDataObservablesIndexRange();

   m_baseCaseSimValues.resize(0);
   m_baseCaseProxyValues.resize(0);

   for (int predictionTargetIdx : idxRange)
   {
       for(const TargetQC& qcTarget : targetQcs)
       {
          if (qcTarget.id() == predictionTargetIdx)
          {
             m_baseCaseSimValues.push_back(qcTarget.y().first());
             m_baseCaseProxyValues.push_back(qcTarget.yProxy().first());
             break;
          }
       }
   }
}

void McmcOutputDataCollector::readOptimalCase()
{
   const RunOptimalCaseScript optimal{m_scenario};
   const QString fileName = optimal.absoluteDirectory() + m_scenario.runCasesObservablesTextFileName();
   m_optimalCaseSimValues = readPredictionTargetsFirstRowObservables(fileName);
}

QVector<double> McmcOutputDataCollector::readPredictionTargetsFirstRowObservables(QString fileName) const
{
   QFile file{fileName};

   if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
   {
      return QVector<double>();
   }

   const DataFileParser<double> fileParser(fileName);
   const QVector<QVector<double>> mat = fileParser.rowDominantMatrix();

   if (mat.size() == 0)
   {
      Logger::log() << "Export MCMC data: Couldn't read target data from file: " << fileName << Logger::endl();
      return QVector<double>();
   }

   return getPredictionData(mat[0]);
}

QVector<double> McmcOutputDataCollector::getPredictionData(const QVector<double>& values) const
{
   const QVector<int> idxRange = m_scenario.predictionDataObservablesIndexRange();

   if (idxRange.back()+1 != values.size())
   {
      Logger::log() << "Export MCMC data: Failed to read prediction data, target indices do not match data size." << Logger::endl();
      return QVector<double>();
   }

   QVector<double> predictionTargetVec;
   for (int i : idxRange)
   {
      predictionTargetVec.push_back(values[i]);
   }
   return predictionTargetVec;
}

void McmcOutputDataCollector::collectPredictionTargetData()
{
   const PredictionTargetManager& predictionTargetManager = m_scenario.predictionTargetManager();
   const QVector<const PredictionTarget*> predictionTargets = predictionTargetManager.predictionTargetsIncludingTimeSeries();

   const MonteCarloDataManager& monteCarloData = m_scenario.monteCarloDataManager();
   const QVector<QVector<double>>& predictionTargetMatrix = monteCarloData.predictionTargetMatrix();
   const QVector<double>& optimalValuesProxy = m_scenario.predictionTargetDataBestMC();

   //Only write the data that could be collected (enables data writing before performing MCMC):
   const int numberOfMatrixRows = predictionTargetManager.amountOfPredictionTargetWithTimeSeriesAndProperties();
   const bool writeOptimalValuesProxy = optimalValuesProxy.size() == numberOfMatrixRows;
   const bool writePercentiles = predictionTargetMatrix.size() == numberOfMatrixRows;
   const bool writeValOptimalSim = m_optimalCaseSimValues.size() == numberOfMatrixRows;
   const bool writeValBaseSim = m_baseCaseSimValues.size() == numberOfMatrixRows;
   const bool writeValBaseProxy = m_baseCaseProxyValues.size() == numberOfMatrixRows;

   QMap<QString,McmcSingleTargetExportData>& targetToDataMap = m_mcmcTargetExportData.targetData;
   std::set<QString>& allPropNames = m_mcmcTargetExportData.allPropNames;
   int indexInPredictionTargetMatrix = 0;
   for (int i = 0; i < predictionTargets.size(); i++)
   {
      const PredictionTarget* target = predictionTargets.at(i);

      const QString targetHash = QString::number(target->x(), 'g',6)
            + QString::number(target->y(), 'g',6)
            + QString::number(target->age(), 'g',6)
            + target->variable();

      McmcSingleTargetExportData expData;
      if (targetToDataMap.contains(targetHash))
      {
         expData = targetToDataMap.value(targetHash);
      }
      else
      {
         expData.locationName = target->locationName();
         expData.age = target->age();
         expData.xCor = target->x();
         expData.yCor = target->y();
         expData.zCor = target->z();
      }

      const QString typeName = target->typeName();
      if(typeName == "XYPointLayerTopSurface")
      {
         expData.stratigraphicSurface = target->surfaceName();
      }

      for (const QString& propertyName : target->properties())
      {
         allPropNames.insert(propertyName);

         TargetDataSingleProperty targetData;
         if (expData.targetOutputs.contains(propertyName))
         {
            targetData = expData.targetOutputs.value(propertyName);
         }

         if (writePercentiles)
         {
            const QVector<double>& targetValues = predictionTargetMatrix.at(indexInPredictionTargetMatrix);
            const QVector<double> quartileTargets{0.1, 0.5, 0.9};
            const QVector<double> percentiles = percentileGenerator::getPercentileValues(targetValues, quartileTargets);
            targetData.p10 = *percentiles.begin();
            targetData.p50 = percentiles.at(1);
            targetData.p90 = percentiles.back();
         }

         if (writeOptimalValuesProxy)  targetData.optimalProxy = optimalValuesProxy.at(indexInPredictionTargetMatrix);
         if (writeValOptimalSim)       targetData.optimalSim = m_optimalCaseSimValues.at(indexInPredictionTargetMatrix);
         if (writeValBaseSim)          targetData.baseSim = m_baseCaseSimValues.at(indexInPredictionTargetMatrix);
         if (writeValBaseProxy)        targetData.baseProxy = m_baseCaseProxyValues.at(indexInPredictionTargetMatrix);

         expData.targetOutputs[propertyName] = targetData;
         targetToDataMap[targetHash] = expData;
         indexInPredictionTargetMatrix++;
      }
   }
}

} // namespace ua

} // namespace casaWizard
