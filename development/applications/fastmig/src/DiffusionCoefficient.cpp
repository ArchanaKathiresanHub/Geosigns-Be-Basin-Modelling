//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include "DiffusionCoefficient.h"
#include "migration.h"

// std library
#include <iostream>
#include <algorithm>

#include "ConstantsMathematics.h"
using Utilities::Maths::CelciusToKelvin;
#include "ConstantsPhysics.h"
using Utilities::Physics::BoltzmannConstant;

namespace migration {

   const double R_GAS = 8.31451;

   DiffusionCoefficient::DiffusionCoefficient(const double& diffusionFactor, const double& activationEnergy):
      m_diffusionFactor(diffusionFactor),
      m_activationTemperature(activationEnergy / R_GAS)
   {
   }

   double DiffusionCoefficient::coefficient(const double& temperatureC, const double& porosityFrac) const
   {
      // FIXME: Probably this is used in order to prevent underflows in pow(porosity, 2.2): 
      double porosity = std::max(0.0001, porosityFrac);

      // Calculate the diffusion coefficient coef for this formation:
      double temperatureK = temperatureC + CelciusToKelvin;
      double dc1 = m_diffusionFactor * exp(- m_activationTemperature / temperatureK);
      double coef = dc1 * pow(porosity, 2.2);

      return coef;
   }
   
   double DiffusionCoefficient::coefficient(const double& temperatureC, const double& porosityFrac, const double& viscosity, const double& gasRadius) const
   {
      // FIXME: Probably this is used in order to prevent underflows in pow(porosity, 2.2): 
      double porosity = std::max(0.0001, porosityFrac);

      // Calculate the diffusion coefficient coef for this formation:
      double temperatureK = temperatureC + CelciusToKelvin;
      double dc1 = BoltzmannConstant*temperatureK / (6.0 * M_PI * gasRadius * viscosity);
      double coef = dc1 * pow(porosity, 2.2);

      return coef;
   }

} // namespace migration
