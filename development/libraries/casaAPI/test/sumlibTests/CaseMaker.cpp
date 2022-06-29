//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "CaseMaker.h"

#include "SumLibTestUtils.h"
#include <algorithm>

const std::vector<SUMlib::Case>& GridCaseMaker::createCaseCollection(int nDataPointsPerDim, int nDims)
{
   const std::vector<double> pv = SumLibTestUtils::valuesSingleParameter(nDataPointsPerDim);
   m_caseCollection.resize(0);
   makeCases({}, pv, nDims, m_caseCollection);

   m_minCase = m_caseCollection[0];
   m_maxCase = m_caseCollection.back();
   return m_caseCollection;
}

void GridCaseMaker::makeCases(const std::vector<double>& pars, const std::vector<double>& parSingleValue, int nDims, std::vector<SUMlib::Case>& caseCollection)
{
   if (pars.size() == size_t(nDims))
   {
      caseCollection.push_back(SUMlib::Case(pars));
      return;
   }

   for (const auto& p : parSingleValue)
   {
      std::vector<double> tmpPars;
      std::copy(pars.begin(), pars.end(),
                std::back_inserter(tmpPars));
      tmpPars.push_back(p);
      makeCases(tmpPars, parSingleValue, nDims, caseCollection);
   }
   return;
}

const std::vector<SUMlib::Case>& RandomCaseMaker::createCaseCollection(int nDataPointsPerDim, int nDims)
{
   srand(0);

   const size_t numCases = size_t(nDataPointsPerDim*nDims);
   m_caseCollection.resize(numCases);
   for (SUMlib::Case& c : m_caseCollection)
   {
      std::vector<double> values(nDims);
      std::generate(values.begin(),values.end(),SumLibTestUtils::randomNumber);
      c.setContinuousPart(values);
   }

   m_minCase = SUMlib::Case(std::vector<double>(size_t(nDims),-1));
   m_maxCase = SUMlib::Case(std::vector<double>(size_t(nDims),1));

   return m_caseCollection;
}
