//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_DIFFUSIONLEAK_H_
#define _MIGRATION_DIFFUSIONLEAK_H_

#include "DiffusionCoefficient.h"

#include <vector>

using std::vector;

namespace migration {

   class DiffusionLeak 
   {
   public:

      struct OverburdenProp 
      {
         double m_thickness;

         double m_topPorosity;
         double m_basePorosity;

         double m_topTemperature;
         double m_baseTemperature;
         
         double m_topViscosity;
         double m_baseViscosity;

      OverburdenProp(const double& thickness, const double& topPorosity, 
                     const double& basePorosity, const double& topTemperature, 
                     const double& baseTemperature, const double& topViscosity, const double& baseViscosity):
         m_thickness(thickness),
            m_topPorosity(topPorosity),
            m_basePorosity(basePorosity),
            m_topTemperature(topTemperature),
            m_baseTemperature(baseTemperature),
            m_topViscosity(topViscosity),
            m_baseViscosity(baseViscosity)
         {}

      OverburdenProp(const OverburdenProp& other):
         m_thickness(other.m_thickness),
            m_topPorosity(other.m_topPorosity),
            m_basePorosity(other.m_basePorosity),
            m_topTemperature(other.m_topTemperature),
            m_baseTemperature(other.m_baseTemperature),
            m_topViscosity(other.m_topViscosity),
            m_baseViscosity(other.m_baseViscosity)
         {}
      };

   private:

      vector<DiffusionLeak::OverburdenProp> m_overburdenProps;
      double m_sealFluidDensity;

      double m_penetrationDistance;
      double m_maxPenetrationDistance;

      DiffusionCoefficient m_coefficient;

      double m_maxTimeStep;
      double m_maxFluxError;

      // As in this version of DiffusionLeak, Deff only changes when m_penetration crosses a 
      // formation boundary, the old m_Deff is cached together with the maximum penetration
      // for which Deff is valid:
      mutable double m_Deff;
      mutable double m_maxPenetrationDistanceForDeff;

      bool performDiffusionTimeStep(const double & diffusionStartTime, const double& stepStartTime, const double& stepEndTime, 
                                    const double& componentWeight, const double& molarFraction, const double& solubility, 
                                    const double& surfaceArea, double& lost, const double& gasRadius, const double* maxError = 0);

      bool updatePenetrationDistance(const double & diffusionStartTime, const double& stepStartTime, const double& stepEndTime, 
                                     double& Deff, double& effPenetrationDistance, const double& gasRadius, const double* maxError = 0);

      double computeDeff(const double& penetrationDistance, const double& gasRadius) const;

      double getOverburdenThickness ();

      double propagatePenetrationDistance(const double& Deff, const double & diffusionStartTime, const double& stepStartTime, 
                                          const double& stepEndTime);

   public:

      DiffusionLeak(const vector<DiffusionLeak::OverburdenProp>& overburdenProps, 
                    const double& sealFluidDensity, 
                    const double& penetrationDistance, const double& maxPenetrationDistance, 
                    const DiffusionCoefficient& diffCoef, 
                    const double& maxTimeStep, const double& maxFluxError);

      void compute(const double& diffusionStartTime, const double & intervalStartTime, const double & intervalEndTime,
                   const double& componentWeight, const double& molarFraction, const double& solubility, const double& surfaceArea, double& lost, const double& gasRadius);
  
      const double& penetrationDistance() const { return m_penetrationDistance; }
   };

} // namespace migration

#endif
