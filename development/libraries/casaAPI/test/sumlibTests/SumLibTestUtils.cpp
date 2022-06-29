//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SumLibTestUtils.h"

#include <stdlib.h>

//Eigen
#include <Eigen/Dense>

using namespace SUMlib;

namespace SumLibTestUtils
{

double randomNumber()
{
   return static_cast <double> (rand()) / (static_cast <double> (RAND_MAX/2.0)) - 1;
}

double monomialValue(const std::vector<double>& pars, const std::vector<unsigned int>& keys, double coefficient)
{
   double monomial = coefficient;
   for (const auto& k : keys)
   {
      monomial *= pars[k];
   }
   return monomial;
}

double polynomialValue(const std::vector<double>& pars, const CubicProxy::CoefficientsMap& coefficientMap)
{
   double vOut(0);
   for (const auto& cm : coefficientMap)
   {
      const std::vector<unsigned int>& monomialKeys = cm.first;
      const double coefficient = cm.second.first;
      vOut += monomialValue(pars,monomialKeys,coefficient);
   }
   return vOut;
}

std::vector<double> applyPolyFunction(const std::vector<Case>& caseCollection,
                                      const CubicProxy::CoefficientsMap& coefficientMap)
{
   std::vector<double> out;
   out.reserve(caseCollection.size());
   for (const auto& v : caseCollection)
   {
      out.push_back(polynomialValue(v.continuousPart(),coefficientMap));
   }
   return out;
}

void leastSquaresFit(CubicProxy::CoefficientsMap& cofMap, const std::vector<Case>& caseCollection, const TargetSet& targetSet)
{
   assert(caseCollection.size() == targetSet.size());

   //Set all coefficients to 1:
   for (auto& entry : cofMap)
   {
      entry.second.first = 1.0;
   }

   Eigen::MatrixXd A(caseCollection.size(),cofMap.size());

   for (size_t i = 0; i < caseCollection.size(); i++)
   {
      const std::vector<double>& pars = caseCollection[i].continuousPart();
      size_t j(0);
      for (const auto& cm : cofMap)
      {
         const std::vector<unsigned int>& monomialKeys = cm.first;
         const double coefficient = cm.second.first;
         A(i,j) = monomialValue(pars,monomialKeys,coefficient);
         j++;
      }
   }

   Eigen::VectorXd b(targetSet.size());
   for (size_t i = 0; i < targetSet.size(); i++)
   {
      b(i) = targetSet[i];
   }

   Eigen::VectorXd result = A.colPivHouseholderQr().solve(b);

   //Somehow, simply iterating over results doesn't work.
   std::vector<double> resOut;
   resOut.resize(result.size());
   Eigen::VectorXd::Map(&resOut[0], result.size()) = result;

   size_t idx = 0;
   for (auto& entry : cofMap)
   {
      entry.second.first = resOut[idx];
      idx++;
   }
}

std::vector<double> valuesSingleParameter(int nDataPointsPerDim)
{
   const double interval = 2.0 / double(nDataPointsPerDim - 1);
   double v = -1.0;

   std::vector<double> pars;
   pars.reserve(size_t(nDataPointsPerDim));
   for (int i = 0; i < nDataPointsPerDim; i++)
   {
      pars.push_back(v);
      v += interval;
   }
   return pars;
}

} //namespace SumLibTestUtils
