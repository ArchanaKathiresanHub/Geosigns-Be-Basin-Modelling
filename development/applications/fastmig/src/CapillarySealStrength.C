//
// Copyright (C) 2010-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//


#include "CapillarySealStrength.h"

#include "EosPack.h"
#include "capillarySealStrength.h"

namespace migration {

   CapillarySealStrength::CapillarySealStrength(const vector<translateProps::CreateCapillaryLithoProp::output>& 
                                                lithProps, const vector<double>& lithFracs, CBMGenerics::capillarySealStrength::MixModel mixModel,
                                                const double& permeability, const double& sealFluidDensity):
      m_lithProps(lithProps),
      m_lithFracs(lithFracs),
      m_mixModel(mixModel),
      m_permeability(permeability),
      m_sealFluidDensity(sealFluidDensity)
   {}

   double CapillarySealStrength::compute(const Composition& composition, const double& gorm, 
                                         const double& T_K) const
   {
      // Calculate the capillary entry pressure for the gas components.  If there is no material, 
      // the capillary entry pressure doesn't matter. We take zero for that case in order 
      // to show there will be no gas:
      double capSealStrength_H2O_HC = 0.0;
      if (composition.getWeight() > 0.0) {
         double T_c_K = pvtFlash::criticalTemperatureAccordingToLiMixingRuleWithLumping(composition.
                                                                                        getWeights(), gorm);
         capSealStrength_H2O_HC = CBMGenerics::capillarySealStrength::capSealStrength_H2O_HC(m_lithProps, 
                                                                                             m_lithFracs, m_mixModel, m_permeability, m_sealFluidDensity, composition.getDensity(), 
                                                                                             T_K, T_c_K);
      }
      return capSealStrength_H2O_HC;
   }

   double CapillarySealStrength::criticalTemperature(const Composition& composition, const double& gorm) const
   {
      // Calculate the critical temperature for the gas components.
      if (composition.getWeight() > 0.0) {
         double T_c_K = pvtFlash::criticalTemperatureAccordingToLiMixingRuleWithLumping(composition.
                                                                                        getWeights(), gorm);
         return T_c_K;
      }
      return 0;
   }

   double CapillarySealStrength::interfacialTension(const Composition& composition, const double& gorm,
                                                    const double& T_K) const
   {
      // Calculate the capillary entry pressure for the gas components.  If there is no material, 
      // the capillary entry pressure doesn't matter.  We take zero for that case in order 
      // to show there will be no gas:
      double it = 0.0;
      if (composition.getWeight() > 0.0) {
         double T_c_K = pvtFlash::criticalTemperatureAccordingToLiMixingRuleWithLumping(composition.
                                                                                        getWeights(), gorm);
         it = CBMGenerics::capillarySealStrength::capTension_H2O_HC(
                                                                    m_sealFluidDensity, composition.getDensity(), T_K, T_c_K);
      }
      return it;
   }

} // namespace migration
