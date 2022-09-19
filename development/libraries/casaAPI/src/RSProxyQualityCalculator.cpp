///
// Copyright (C) Shell. All rights reserved.
//

/// @file RSProxyQualityCalculatorImpl.cpp
/// @brief This file keeps API implementation of RSProxyQualityCalculatorImpl

#include "RSProxyQualityCalculator.h"
#include "RunCase.h"
#include "ScenarioAnalysis.h"

#include "LogHandler.h"

#include <cmath>
#include <limits>

namespace casa
{

namespace
{

// Auxiliary functions and variables

const double epsilon = std::numeric_limits<double>::epsilon();

void extractObservableValues(std::vector<double>& result, const casa::RunCase* cs)
{
   // go over all observables and calculate PrxVal for each observable
   for (size_t j = 0; j < cs->observablesNumber(); ++j)
   {
      const casa::ObsValue* obv = cs->obsValue(j);

      if (!obv->isDouble())
      {
         continue;
      }

      const std::vector<double>& vals = obv->asDoubleArray();
      result.insert(result.end(), vals.begin(), vals.end());
   }
}

const std::vector<double> averagesOfMatrixColumns(const std::vector<std::vector<double>>& matrix)
{
   const int nRows    = matrix.size();
   const int nColumns = matrix[0].size();
   std::vector<double> yAvg(nColumns, 0.0);

   for (int j = 0; j < nColumns; ++j)
   {
      for (int i = 0; i < nRows; ++i)
      {
         yAvg[j] += matrix[i][j];
      }
      yAvg[j] /= nRows;
   }

   return yAvg;
}

double calculateSSEOverSSyy(const int j, const std::vector<std::vector<double>>& runCasesObservables, const std::vector<std::vector<double>>& proxyEvaluationObservables, const double yAverage, const int nDoEs)
{
   double SSe  = 0.0;
   double SSyy = 0.0;

   for (int i = 0; i < nDoEs; ++i)
   {
      double e  = runCasesObservables[i][j] - proxyEvaluationObservables[i][j];
      double yy = runCasesObservables[i][j] - yAverage;
      SSe  += e*e;
      SSyy += yy*yy;
   }

   return (std::fabs(SSyy) > epsilon) ? SSe/SSyy : -1.0;
}

} // namespace

RSProxyQualityCalculator::RSProxyQualityCalculator(ScenarioAnalysis& scenario) :
   m_scenario{scenario}
{
}

std::vector<std::vector<double>> RSProxyQualityCalculator::calculateR2AndR2adj(const std::string& proxyName, const std::vector<string>& experimentList) const
{  
   casa::RunCaseSet & doeCaseSet = m_scenario.doeCaseSet();
   std::vector<std::string> doeList = experimentList.empty() ? doeCaseSet.experimentNames() : experimentList;

   casa::RSProxy* proxy = m_scenario.rsProxySet().rsProxy(proxyName.c_str());
   if (!proxy)
   {
      throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Unknown proxy name:" << proxyName;
   }

   size_t nRunCases = doeCaseSet.size();

   std::vector<std::vector<double>> proxyEvaluationObservables;
   std::vector<std::vector<double>> runCasesObservables;

   doeCaseSet.filterByDoeList(doeList);

   for (size_t iRunCase = 0; iRunCase < nRunCases; ++iRunCase)
   {
      const casa::RunCase* runCase = doeCaseSet.runCase(iRunCase);
      if (runCase->runStatus() != casa::RunCase::Completed)
      {
         LogHandler( LogHandler::WARNING_SEVERITY ) << "Skipping run case " << iRunCase << " because it is not completed.";
         continue;
      }

      // Extract run case observables
      runCasesObservables.push_back({});
      extractObservableValues(runCasesObservables.back(), runCase);

      // Extract proxy evaluation observables
      std::shared_ptr<casa::RunCase> proxyRunCase = runCase->shallowCopy();
      if (ErrorHandler::NoError != proxy->evaluateRSProxy(*proxyRunCase))
      {
         throw ErrorHandler::Exception(proxy->errorCode()) << proxy->errorMessage();
      }

      proxyEvaluationObservables.push_back({});
      extractObservableValues(proxyEvaluationObservables.back(), proxyRunCase.get());
   }

   const casa::RSProxy::CoefficientsMapList & cml = proxy->getCoefficientsMapList();

   std::vector<int> nCoefficients;
   for (const auto& cofMap : cml)
   {
      nCoefficients.push_back(int(cofMap.size()));
   }

   return calculateR2AndR2adjFromObservables(runCasesObservables, proxyEvaluationObservables, nCoefficients);
}

std::vector<double> RSProxyQualityCalculator::calculateQ2(const string& proxyName, const std::vector<string>& experimentList) const
{
   casa::RunCaseSet & doeCaseSet = m_scenario.doeCaseSet();
   std::vector<std::string> doeList = experimentList.empty() ? doeCaseSet.experimentNames() : experimentList;

   const casa::RSProxy* proxy = m_scenario.rsProxySet().rsProxy(proxyName.c_str());

   if (!proxy)
   {
      throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Unknown proxy name:" << proxyName;
   }

   const int proxyOrder = proxy->polynomialOrder();
   const casa::RSProxy::RSKrigingType proxyKrigingType = proxy->kriging();

   doeCaseSet.filterByDoeList(doeList);
   const size_t nRunCases = doeCaseSet.size();
   std::vector<std::vector<double>> proxyEvaluationObservables;
   std::vector<std::vector<double>> runCasesObservables;

   int iGlobalRunCase = 0;

   for (size_t iRunCase = 0; iRunCase < nRunCases; ++iRunCase)
   {
      const casa::RunCase* runCase = doeCaseSet.runCase(iRunCase);
      if (runCase->runStatus() != casa::RunCase::Completed)
      {
         LogHandler( LogHandler::WARNING_SEVERITY ) << "Skipping run case " << iRunCase; // << " of DoE " << doe;
         continue;
      }

      std::shared_ptr<casa::RSProxy> proxyQ2(m_scenario.createRSProxy("proxyQ2", proxyOrder, proxyKrigingType));

      calculateRSProxyQ2(*proxyQ2, doeList, iGlobalRunCase);

      LogHandler (LogHandler::DEBUG_SEVERITY)  << "Evaluating proxy No. " << iGlobalRunCase << "for Q2 calculation ...";

      // Extract run case observables
      runCasesObservables.push_back({});
      extractObservableValues(runCasesObservables.back(), runCase);

      // Extract proxy evaluation observables
      std::shared_ptr<RunCase> proxyObservableRunCaseQ2 = runCase->shallowCopy();

      if (ErrorHandler::NoError != proxyQ2->evaluateRSProxy(*proxyObservableRunCaseQ2))
      {
         throw ErrorHandler::Exception(proxyQ2->errorCode()) << proxyQ2->errorMessage();
      }

      proxyEvaluationObservables.push_back({});
      extractObservableValues(proxyEvaluationObservables.back(), proxyObservableRunCaseQ2.get());

      ++iGlobalRunCase;
   }

   return calculateQ2FromObservables(runCasesObservables, proxyEvaluationObservables);
}

const std::vector<std::vector<double>> RSProxyQualityCalculator::calculateR2AndR2adjFromObservables(const std::vector<std::vector<double>>& runCaseObservables,
                                                                                                    const std::vector<std::vector<double>>& proxyObservables,
                                                                                                    const std::vector<int>& nCoefficients)
{
   const int nTargets{ static_cast<int>(runCaseObservables[0].size())};

   std::vector<std::vector<double>> R2AndR2adj(2, std::vector<double>(nTargets, 0.0));
   std::vector<double> yAvg = averagesOfMatrixColumns(runCaseObservables);

   const int nDoEs = runCaseObservables.size();
   for (int j = 0; j < nTargets; ++j)
   {
      const double SSEOverSSyy = calculateSSEOverSSyy(j, runCaseObservables, proxyObservables, yAvg[j], nDoEs);
      double R2Current    = 0.0;
      double R2adjCurrent = 0.0;

      if (SSEOverSSyy >= 0)
      {
         R2Current    = 1.0 - SSEOverSSyy;
         R2adjCurrent = 1.0 - SSEOverSSyy*(nDoEs - 1)/(nDoEs - nCoefficients.at(size_t(j)));
      }

      R2AndR2adj[0][j] = R2Current;
      R2AndR2adj[1][j] = R2adjCurrent;
   }

   return R2AndR2adj;
}

const std::vector<double> RSProxyQualityCalculator::calculateQ2FromObservables(const std::vector<std::vector<double>>& runCasesObservables, const std::vector<std::vector<double>>& proxyEvaluationObservables) const
{
   const int nDoEs    = runCasesObservables.size();
   const int nTargets = runCasesObservables[0].size();

   std::vector<double> Q2(nTargets, 0.0);
   std::vector<double> yAvg = averagesOfMatrixColumns(runCasesObservables);

   for (int j = 0; j < nTargets; ++j)
   {
      const double SSEOverSSyy = calculateSSEOverSSyy(j, runCasesObservables, proxyEvaluationObservables, yAvg[j], nDoEs);
      double Q2Current = 0;

      if (SSEOverSSyy >= 0)
      {
         Q2Current = 1.0 - SSEOverSSyy;
      }

      Q2[j] = Q2Current;
   }

   return Q2;
}

void RSProxyQualityCalculator::calculateRSProxyQ2(RSProxy& proxy, const std::vector<string>& doeList, const int iGlobalDoEPointRemove) const
{
   if ( doeList.empty() )
   {
      return;
   }

   const std::vector<const casa::RunCase *> & runCases = m_scenario.doeCaseSet().collectCompletedCases( doeList );

   if ( iGlobalDoEPointRemove >= runCases.size() )
   {
      throw Exception( OutOfRangeValue ) << "calculateRSProxyQ2(): index greater than size of run cases";
   }

   std::vector<const casa::RunCase *> runCasesQ2 = runCases;
   if ( runCasesQ2.size() < 2  )
   {
      return;
   }

   runCasesQ2.erase( runCasesQ2.begin() + iGlobalDoEPointRemove );

   if (ErrorHandler::NoError != proxy.calculateRSProxy(runCasesQ2))
   {
      throw Exception(proxy.errorCode() ) << proxy.errorMessage();
   }
}

} // namespace casa
