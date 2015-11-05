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
