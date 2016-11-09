//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef BIOMARKERS_UTILITIES_H
#define BIOMARKERS_UTILITIES_H

// std library
#include <cmath>

// utilities library
#include "ConstantsMathematics.h"
using Utilities::Maths::MillionYearToSecond;
#include "ConstantsPhysics.h"
using Utilities::Physics::IdealGasConstantCalibration;

namespace Calibration
{

class AromatizationFunction
{
public:
   AromatizationFunction(const double &frequencyFactor)
   {
      m_frequencyFactor = frequencyFactor * MillionYearToSecond;
   }
   ~AromatizationFunction(){}
   double operator()(const double &integral) const
   {
      return (1. - std::exp (-m_frequencyFactor * integral)); 
   }
private:
   double m_frequencyFactor;
};
class IsomerizationFunction
{
public:
   IsomerizationFunction(const double &gamma, const double &frequencyFactor):
   m_gamma(gamma)
   {
      m_frequencyFactor = frequencyFactor * MillionYearToSecond;
   }
   ~IsomerizationFunction(){}
   double operator()(const double &integral) const
   {
      return (m_gamma / (1. + m_gamma) * (1. - std::exp ((-1. - m_gamma) * m_frequencyFactor * integral)));
   }
private:
   double m_gamma;
   double m_frequencyFactor;
};
class BiomarkerKineticsFunction
{
public:
   BiomarkerKineticsFunction(const double &inActivationEnergy):
   m_activationEnergy(inActivationEnergy)
   {
   }
   ~BiomarkerKineticsFunction(){}
   double operator()(const double &temperature) const
   {
      return std::exp (-m_activationEnergy / (IdealGasConstantCalibration * temperature));
   }
private:
   double m_activationEnergy;
};
class TrapezoidalIntegrator
{
public:
   double operator()(const BiomarkerKineticsFunction &theFunction, const double &timestepSize, const double &startTemp, const double &endTemp) const
   {
      return (timestepSize * 0.5 * (theFunction(startTemp) + theFunction(endTemp)));
   }
};

}
#endif
