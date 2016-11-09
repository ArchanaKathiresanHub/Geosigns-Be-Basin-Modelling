//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_DIFFUSIONCOEFFICIENT_H_
#define _MIGRATION_DIFFUSIONCOEFFICIENT_H_

namespace migration {

   class DiffusionCoefficient {

   private:

      double m_diffusionFactor;
      double m_activationTemperature;

   public:

      DiffusionCoefficient(const double& diffusionFactor, const double& ActivationEnergy);

      double coefficient(const double& temperatureC, const double& porosityFrac) const; 
      
      //Stokes-Einstein equation: applied to diffuse molecules in water!
      double coefficient(const double& temperatureC, const double& porosityFrac, const double& viscosity, const double& gasRadius) const;
   
   };

} // namespace migration

#endif
