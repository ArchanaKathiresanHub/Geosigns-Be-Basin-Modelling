//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ProxyFitTester.h"
#include "RSProxyQualityCalculator.h"

//Std
#include <numeric>

using namespace SUMlib;
using namespace SumLibTestUtils;

ProxyFitTester::ProxyFitTester(int nDataPointsPerDim, int nDimensions, CaseMaker* caseMaker):
   m_caseMaker(caseMaker),
   m_proxyCollection(
      caseMaker->createCaseCollection(nDataPointsPerDim, nDimensions),
      caseMaker->minCase(),
      caseMaker->maxCase()),
   m_r2Adj(0)
{}

ProxyFitTester::~ProxyFitTester() = default;

void ProxyFitTester::makeTestFit(const CubicProxy::CoefficientsMap& stubModel, const TestInfo& info)
{
   //Dummy DoE values for a single target.
   m_targetSet = applyPolyFunction(m_caseMaker->caseCollection(), stubModel);

   //case2Obs2Valid   indicates (in)valid "case(row)-obs(column)" combinations
   const std::vector< std::vector< bool > > case2Obs2Valid(size_t(m_caseMaker->caseCollection().size()), { true });

   // Partition allows for excluding certain influential parameters from the fitting procedure.
   // As stated in CompaoundProxyCollection.h:
   // list of flags, one for each parameter, to indicate whether the element should be included.
   const Partition partition(m_caseMaker->minCase().size(), true);

   //The last optional input const std::vector< ParameterTransformTypeVector >& not used. The transform transforms the input parameter space to a unit hypercube.
   //Default transforms just applies scaling. Other transforms can also be chosen, but since the transforms are applied to the influential parameters and not to the target set,
   //its usage is not straightforward. This option is currently not used in casa.
   m_proxyCollection.calculate(TargetCollection{m_targetSet}, case2Obs2Valid, info.order, info.modelSearch, info.targetR2, info.confLevel, partition /*,parTransformsDef*/);

   std::vector<CubicProxy::CoefficientsMap> fittedCoefficientsVec;
   m_proxyCollection.getCoefficientsMapList(fittedCoefficientsVec);
   m_fittedCoefficients = fittedCoefficientsVec[0];
   const CompoundProxy* proxy = m_proxyCollection.getProxyList()[0];
   m_r2Adj = proxy->adjustedR2();
}

double ProxyFitTester::meanOfTargets(const CubicProxy::CoefficientsMap& stubModel) const
{
   const TargetSet targetSet = applyPolyFunction(m_caseMaker->caseCollection(), stubModel);
   const double sum = std::accumulate(targetSet.begin(), targetSet.end(), 0.0);
   return sum / targetSet.size();
}

void ProxyFitTester::leastSquaresFit(SUMlib::CubicProxy::CoefficientsMap& cofMap) const
{
   SumLibTestUtils::leastSquaresFit(cofMap, m_caseMaker->caseCollection(), m_targetSet);
}

bool ProxyFitTester::compareCasaSumlibR2Adj() const
{
   std::vector<std::vector<double>> casaTargets;
   for (auto t : m_targetSet)
   {
      casaTargets.push_back({t});
   }

   std::vector<double> proxyValues = applyPolyFunction(m_caseMaker->caseCollection(), m_fittedCoefficients);
   std::vector<std::vector<double>> casaProxyVals;
   for (auto p : proxyValues)
   {
      casaProxyVals.push_back({p});
   }

   const int nCoefficients = int(m_fittedCoefficients.size());
   std::vector<std::vector<double>> r2Andr2Adj = casa::RSProxyQualityCalculator::calculateR2AndR2adjFromObservables(casaTargets,
                                                                                                                    casaProxyVals,
                                                                                                                    nCoefficients);
   double r2AdjCasa = r2Andr2Adj[1][0];
   return std::abs(r2AdjCasa-m_r2Adj) < 1e-6;
}

SUMlib::CubicProxy::CoefficientsMap ProxyFitTester::leastSquaresComparisonFit() const
{
   SUMlib::CubicProxy::CoefficientsMap tmpCoefficients = m_fittedCoefficients;
   SumLibTestUtils::leastSquaresFit(tmpCoefficients, m_caseMaker->caseCollection(), m_targetSet);
   return tmpCoefficients;
}

double ProxyFitTester::r2Adj() const
{
   return m_r2Adj;
}

const CompoundProxyCollection& ProxyFitTester::proxyCollection() const
{
   return m_proxyCollection;
}

CubicProxy::CoefficientsMap ProxyFitTester::fittedCoefficients() const
{
   return m_fittedCoefficients;
}

const std::vector<Case>& ProxyFitTester::caseCollection() const
{
   return m_caseMaker->caseCollection();
}

TargetSet ProxyFitTester::targetSet() const
{
   return m_targetSet;
}
