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
#include "BrooksCorey.h"

namespace migration {

   CapillarySealStrength::CapillarySealStrength( const vector< vector<translateProps::CreateCapillaryLithoProp::output> >& lithProps, 
                                                 const vector< vector<double> >& lithFracs, 
                                                 const vector<CBMGenerics::capillarySealStrength::MixModel> mixModel,
                                                 const vector<double>& permeability,
                                                 const double& sealFluidDensity,
                                                 const double& lamdaPC,
                                                 const bool isLegacy ) :
      m_lithProps(lithProps),
      m_lithFracs(lithFracs),
      m_mixModel(mixModel),
      m_permeability(permeability),
      m_sealFluidDensity(sealFluidDensity),
      m_lambdaPC( lamdaPC ),
      m_isLegacy( isLegacy )
   {}

   double CapillarySealStrength::compute( const Composition& composition, 
                                          const double& gorm,
                                          const double& T_K ) const
   {
      // Calculate the capillary entry pressure for the gas components.  If there is no material, 
      // the capillary entry pressure doesn't matter. We take zero for that case in order 
      // to show there will be no gas:
      double capSealStrength_H2O_HC = 0.0;

      if ( m_isLegacy )
      {
         if ( composition.getWeight() > 0.0 ) {
            double T_c_K = pvtFlash::criticalTemperatureAccordingToLiMixingRuleWithLumping( composition.
               getWeights(), gorm );
            capSealStrength_H2O_HC = CBMGenerics::capillarySealStrength::capSealStrength_H2O_HC( m_lithProps[0],
               m_lithFracs[0], m_mixModel[0], m_permeability[0], m_sealFluidDensity, composition.getDensity(),
               T_K, T_c_K );
         }
         return capSealStrength_H2O_HC;
      }
      else
      {
         if ( composition.getWeight( ) > 0.0 )
         {
            if ( m_permeability.size( ) == 2 ) //both reservoir and seal formations are defined
            {
               double T_c_K = pvtFlash::criticalTemperatureAccordingToLiMixingRuleWithLumping( composition.
               getWeights( ), gorm );
               
               double capSealStrength_H2O_HC_reservoir = 0;
               double capSealStrength_H2O_HC_seal = 0;

               capSealStrength_H2O_HC_reservoir = CBMGenerics::capillarySealStrength::capSealStrength_H2O_HC( m_lithProps[0],
                  m_lithFracs[0], m_mixModel[0], m_permeability[0], m_sealFluidDensity, composition.getDensity( ),
                  T_K, T_c_K );

               capSealStrength_H2O_HC_seal = CBMGenerics::capillarySealStrength::capSealStrength_H2O_HC( m_lithProps[1],
                  m_lithFracs[1], m_mixModel[1], m_permeability[1], m_sealFluidDensity, composition.getDensity( ),
                  T_K, T_c_K );

               // compute the Brooks Corey correction in the reservoir
               GeoPhysics::BrooksCorey brooksCorey;
               double resCorr = brooksCorey.computeBrooksCoreyCorrection( 0.3, m_lambdaPC );

               capSealStrength_H2O_HC = capSealStrength_H2O_HC_seal - capSealStrength_H2O_HC_reservoir * resCorr;
            }
         }
         return capSealStrength_H2O_HC < 0.0 ? 0.0 : capSealStrength_H2O_HC;
      }
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
