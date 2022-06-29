//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

// SUMlib includes
#include <CubicProxy.h>
#include <Case.h>

//std
#include <vector>

namespace SumLibTestUtils
{

//Random number between -1 and 1
double randomNumber();


double monomialValue(const std::vector<double>& pars, const std::vector<unsigned int>& keys, double coefficient);

//Returns a single value of the polynomial funtion defined by coefficientmap, for inputs pars.
double polynomialValue(const std::vector<double>& pars, const SUMlib::CubicProxy::CoefficientsMap& coefficientMap);

//Returns polynomial values of the polynomial funtion defined by coefficientmap, for a range of input values defined by caseCollection.
std::vector<double> applyPolyFunction(const std::vector<SUMlib::Case>& caseCollection,
                                      const SUMlib::CubicProxy::CoefficientsMap& coefficientMap);

//Fits the coefficients of the monomial terms defined by cofMap to the data defined by caseCollection and targetSet.
void leastSquaresFit(SUMlib::CubicProxy::CoefficientsMap& cofMap,
                                    const std::vector<SUMlib::Case>& caseCollection,
                                    const SUMlib::TargetSet& targetSet);

//Returns equidistant range of values of nDataPointsPerDim between -1 and 1.
std::vector<double> valuesSingleParameter(int nDataPointsPerDim);

} //namespace SumLibTestUtils
