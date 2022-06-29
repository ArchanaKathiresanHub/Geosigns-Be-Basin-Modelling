//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "SumLibTestUtils.h"
#include "CaseMaker.h"
#include "TestInfo.h"

#include <gtest/gtest.h>

// SUMlib includes
#include <CubicProxy.h>
#include <CompoundProxyCollection.h>

#include <memory>

//Support class for unit tests.
class ProxyFitTester
{
public:

   //Initialization of m_proxyCollection can be a time-consuming for a high number of cases. In its initialization,
   //the case covariance matrix is formed and its pseudo-inverse is calculated. These are only needed when applying Kriging.
   //This process takes more time than I would expect.
   ProxyFitTester(int nDataPointsPerDim, int nDimensions, CaseMaker* caseMaker = new GridCaseMaker);

   ~ProxyFitTester();

   void makeTestFit(const SUMlib::CubicProxy::CoefficientsMap& stubModel, const TestInfo& info);

   double meanOfTargets(const SUMlib::CubicProxy::CoefficientsMap& stubModel) const;

   //Performs a least-squares fit for the coefficients of the monomials defined by cofMap
   void leastSquaresFit(SUMlib::CubicProxy::CoefficientsMap& cofMap) const;

   //Performs a least-squares fit for the coefficients of the monomials found by SUMLib
   SUMlib::CubicProxy::CoefficientsMap leastSquaresComparisonFit() const;

   //Compares the R2Adj value calculated by sumlib against the R2Adj value calculated by Casa:
   bool compareCasaSumlibR2Adj() const;

   double r2Adj() const;
   const SUMlib::CompoundProxyCollection& proxyCollection() const;
   SUMlib::CubicProxy::CoefficientsMap fittedCoefficients() const;
   const std::vector<SUMlib::Case>& caseCollection() const;
   SUMlib::TargetSet targetSet() const;

private:
   std::unique_ptr<CaseMaker> m_caseMaker;

   SUMlib::TargetSet m_targetSet;

   //Collection of proxies for a single caseset. One proxy for each targetSet in the targetCollection. This test class supports a single targetset.
   SUMlib::CompoundProxyCollection m_proxyCollection;
   double m_r2Adj; //R2 adj quality measure of the fitted proxy
   SUMlib::CubicProxy::CoefficientsMap m_fittedCoefficients; //Coefficient map defining the fitted proxy
};
