#include "DiffusionCoefficient.h"
#include "migration.h"
#include "consts.h"

#include <iostream>
#include <algorithm>
#include <math.h>

using CBMGenerics::C2K;

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
      double porosity = max(0.0001, porosityFrac);

      // Calculate the diffusion coefficient coef for this formation:
      double temperatureK = temperatureC + C2K;
      double dc1 = m_diffusionFactor * exp(- m_activationTemperature / temperatureK);
      double coef = dc1 * pow(porosity, 2.2);

      return coef;
   }

} // namespace migration
