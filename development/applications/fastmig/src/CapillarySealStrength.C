//
// Copyright (C) 2010-2018 Shell International Exploration & Production.
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
                                                 const vector<CBMGenerics::capillarySealStrength::MixModel>& mixModel,
                                                 const vector<double>& permeability,
                                                 const double sealFluidDensity,
                                                 const double lamdaPC ) :
   m_lithProps(lithProps),
      m_lithFracs(lithFracs),
      m_mixModel(mixModel),
      m_permeability(permeability),
      m_sealFluidDensity(sealFluidDensity),
      m_lambdaPC( lamdaPC )
   {}

   //compute corrected capillary pressure for gas
   void CapillarySealStrength::compute(const std::vector<Composition> & composition, const double gorm, const double T_K, const double brinePressure,
                                       double& capSealStrength_H2O_Gas, double& capSealStrength_H2O_Oil) const
   {
      capSealStrength_H2O_Gas = 0.0;
      capSealStrength_H2O_Oil = 0.0;


      if ( m_permeability.size() == 2 ) //both reservoir and seal formations are defined
      {
         double capSealStrength_oil_reservoir = 0.0;
         double capSealStrength_oil_seal = 0.0;
         double capSealStrength_gas_reservoir = 0.0;
         double capSealStrength_gas_seal = 0.0;

         // compute the Brooks Corey correction in the reservoir
         GeoPhysics::BrooksCorey brooksCorey;
         double resCorr = brooksCorey.computeBrooksCoreyCorrection(0.3, m_lambdaPC);

         if ( composition[1].getWeight() > 0.0 ) // oil is present?-> oil is the wetting phase for gas
         {
            // oil
            double T_c_K_oil = pvtFlash::criticalTemperatureAccordingToLiMixingRuleWithLumping(composition[1].getWeights(), gorm);

            capSealStrength_oil_reservoir = CBMGenerics::capillarySealStrength::capSealStrength_H2O_HC(m_lithProps[0],
                                                                                                       m_lithFracs[0], m_mixModel[0], m_permeability[0], m_sealFluidDensity, composition[1].getDensity(),
                                                                                                       T_K, T_c_K_oil);

            capSealStrength_oil_seal = CBMGenerics::capillarySealStrength::capSealStrength_H2O_HC(m_lithProps[1],
                                                                                                  m_lithFracs[1], m_mixModel[1], m_permeability[1], m_sealFluidDensity, composition[1].getDensity(),
                                                                                                  T_K, T_c_K_oil);
               
            // Seal and reservoir P_c values for liquid have been determined. Put it all together to get the capSealStrength
            capSealStrength_H2O_Oil = capSealStrength_oil_seal - capSealStrength_oil_reservoir * resCorr;
            if ( capSealStrength_H2O_Oil < 0.0 ) capSealStrength_H2O_Oil = 0.0;

            // gas
            if ( composition[0].getWeight() > 0.0 )
            {
               double capC1;
               double capC2;
               double pceHgAir_reservoir;
               double pceHgAir_seal;

               double T_c_K_gas = pvtFlash::criticalTemperatureAccordingToLiMixingRuleWithLumping(composition[0].getWeights(), gorm);

               // Ratio of interfacial-tensions and cos(contact-angle).
               double hgAigToLiquidVapourConversionFactor = brooksCorey.liquidVapourInterfacialTension(brinePressure) * GeoPhysics::BrooksCorey::CosLiquidVapourContactAngle /
                  (GeoPhysics::BrooksCorey::MercuryAirInterfacialTension * GeoPhysics::BrooksCorey::CosMercuryAirContactAngle);

               // reservoir
               CBMGenerics::capillarySealStrength::capParameters(m_lithProps[0], m_lithFracs[0], m_mixModel[0], capC1, capC2);
               pceHgAir_reservoir = CBMGenerics::capillarySealStrength::capSealStrength_Air_Hg(capC1, capC2, m_permeability[0]);
               capSealStrength_gas_reservoir = capSealStrength_oil_reservoir + pceHgAir_reservoir * hgAigToLiquidVapourConversionFactor;

               // seal
               CBMGenerics::capillarySealStrength::capParameters(m_lithProps[1], m_lithFracs[1], m_mixModel[1], capC1, capC2);
               capSealStrength_gas_seal = CBMGenerics::capillarySealStrength::capSealStrength_H2O_HC(m_lithProps[1],
                                                                                                     m_lithFracs[1], m_mixModel[1], m_permeability[1], m_sealFluidDensity, composition[0].getDensity(),
                                                                                                     T_K, T_c_K_gas);
            }

         }
         else if ( composition[0].getWeight() > 0.0 ) // if only gas is present -> cp gas not corrected
         {
            double T_c_K_gas = pvtFlash::criticalTemperatureAccordingToLiMixingRuleWithLumping(composition[0].getWeights(), gorm);

            capSealStrength_gas_reservoir = CBMGenerics::capillarySealStrength::capSealStrength_H2O_HC(m_lithProps[0],
                                                                                                       m_lithFracs[0], m_mixModel[0], m_permeability[0], m_sealFluidDensity, composition[0].getDensity(),
                                                                                                       T_K, T_c_K_gas);

            capSealStrength_gas_seal = CBMGenerics::capillarySealStrength::capSealStrength_H2O_HC(m_lithProps[1],
                                                                                                  m_lithFracs[1], m_mixModel[1], m_permeability[1], m_sealFluidDensity, composition[0].getDensity(),
                                                                                                  T_K, T_c_K_gas);
         }
         // Seal and reservoir P_c values for vapour have been determined. Put it all together to get the capSealStrength
         capSealStrength_H2O_Gas = capSealStrength_gas_seal - capSealStrength_gas_reservoir * resCorr;
         if ( capSealStrength_H2O_Gas < 0.0 ) capSealStrength_H2O_Gas = 0.0;            
      }
   }

   double CapillarySealStrength::criticalTemperature(const Composition& composition, const double gorm) const
   {
      // Calculate the critical temperature for the gas components.
      if (composition.getWeight() > 0.0) {
         double T_c_K = pvtFlash::criticalTemperatureAccordingToLiMixingRuleWithLumping(composition.
                                                                                        getWeights(), gorm);
         return T_c_K;
      }
      return 0;
   }

   double CapillarySealStrength::interfacialTension(const Composition& composition, const double gorm,
                                                    const double T_K) const
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
