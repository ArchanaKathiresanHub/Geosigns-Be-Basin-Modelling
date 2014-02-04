#include "mpi.h"
#include "Lithology.h"
#include "BrooksCorey.h"
#include <cmath>
#include <sstream>

#include "GeoPhysicalConstants.h"

#include "NumericFunctions.h"
#include "FastcauldronSimulator.h"

Lithology::Lithology ( GeoPhysics::ProjectHandle* projectHandle ) : GeoPhysics::CompoundLithology ( projectHandle ) {

   PVTPhaseValues contactAngle;

   m_lithologyId = IBSNULLVALUE;

   contactAngle ( pvtFlash::VAPOUR_PHASE ) = BrooksCorey::GasWaterContactAngle * M_PI / 180.0;
   contactAngle ( pvtFlash::LIQUID_PHASE ) = BrooksCorey::OilWaterContactAngle * M_PI / 180.0;

   m_cosContactAngle ( pvtFlash::VAPOUR_PHASE ) = std::cos ( contactAngle ( pvtFlash::VAPOUR_PHASE ));
   m_cosContactAngle ( pvtFlash::LIQUID_PHASE ) = std::cos ( contactAngle ( pvtFlash::LIQUID_PHASE ));
}

//------------------------------------------------------------//

std::string Lithology::getName () const {

   std::stringstream buffer;
   size_t i;

   buffer << "{";

   for ( i = 0; i < m_lithoComponents.size (); ++i ) {

      if ( m_lithoComponents [ i ] != 0 ) {
         buffer << m_lithoComponents [ i ]->getName ();
      // } else {
      }

      if ( i < m_lithoComponents.size () - 1 ) {
         buffer << ", ";
      }

   }

   buffer << "}";
   return buffer.str ();
}

//------------------------------------------------------------//

double Lithology::relativePermeability ( const Saturation::Phase phase,
                                         const Saturation&       saturation ) const {

   return brooksAndCoreyRelativePermeability ( phase, saturation );
}

//------------------------------------------------------------//

double Lithology::brooksAndCoreyRelativePermeability ( const Saturation::Phase phase,
                                                       const Saturation&       saturation ) const {

   //1)Brine or Water relative permeability
   //2)HCLiquid relative permeability
   //3)HCVapour relative permeability
   
   double relPerm = 1.0;
      
   //Brine is wetting phase
   
   if ( phase == Saturation::WATER ) {

      if ( getWaterRelPermExponent () == IBSNULLVALUE ) {
         // What should the correct values be here?
         // 1.0 here because we would like the brine-pressure solver to run normally.
         return 1.0;
      } else {
         return BrooksCorey::krw ( saturation ( Saturation::WATER ),
                                   getWaterRelPermExponent (),
                                   getIrreducibleWaterSaturation (),
                                   getResidualVapourSaturation (),
                                   getResidualLiquidSaturation (),
                                   getWaterRelPermMax ()); // water relative permeability
      }

   } else if ( phase == Saturation::VAPOUR ) {

      if ( getVapourRelPermExponent () == IBSNULLVALUE ) {
         // What should the correct values be here?
         // 0.0 because we would like for there to be no transport in such lithologies (normally).
         return 0.0;
      } else {
         return BrooksCorey::kro ( pvtFlash::VAPOUR_PHASE,
                                   saturation ( Saturation::VAPOUR ),
                                   getVapourRelPermExponent (),
                                   getIrreducibleWaterSaturation (),
                                   getResidualVapourSaturation (),
                                   getResidualLiquidSaturation (),
                                   getVapourRelPermMax ()); // Vapour hydrocarbon relative permeability
      }

   } else { // phase == Saturation::LIQUID

      if ( getLiquidRelPermExponent () == IBSNULLVALUE ) {
         // What should the correct values be here?
         // 0.0 because we would like for there to be no transport in such lithologies (normally).
         return 0.0;
      } else {
         return BrooksCorey::kro ( pvtFlash::LIQUID_PHASE,
                                   saturation ( Saturation::LIQUID ),
                                   getLiquidRelPermExponent (),
                                   getIrreducibleWaterSaturation (),
                                   getResidualVapourSaturation (),
                                   getResidualLiquidSaturation (),
                                   getLiquidRelPermMax ()); // Liquid hydrocarbon relative permeability
      }

   }

   return relPerm;
}


//------------------------------------------------------------//

double Lithology::capillaryEntryPressure ( const pvtFlash::PVTPhase phase,
                                           const double             temperature,
                                           const double             permeability,
                                           const double             brineDensity,
                                           const double             hcPhaseDensity,
                                           const double             criticalTemperature ) const {

   double pce;

   if ( FastcauldronSimulator::getInstance ().useCalculatedCapillaryEntryPressure ()) {

      if ( criticalTemperature != CAULDRONIBSNULLVALUE ) {
         double usedHcPhaseDensity;

         if ( brineDensity <= hcPhaseDensity ) {
            // Get the largest floating point number that is smaller than brineDensity.
            // This is because the cap-tension function will return floating-point-max if the hc-densiy > h20-density.
            usedHcPhaseDensity = nextafter ( brineDensity, 0.0 );
         } else {
            usedHcPhaseDensity = hcPhaseDensity;
         }

         // Units of interfacial-tension are mN/M so they need to be scaled by 0.001 to get into N/M.
         double hcWaterInterfacialTension = 0.001 * CBMGenerics::capillarySealStrength::capTension_H2O_HC ( brineDensity,
                                                                                                            usedHcPhaseDensity,
                                                                                                            temperature + 273.15,
                                                                                                            criticalTemperature );
         double cpeHgAir = BrooksCorey::computeCapillaryEntryPressure ( permeability * GeoPhysics::M2TOMILLIDARCY, capC1 (), tenPowerCapC2 ());

         // Convert to a hc-water entry pressure.
         pce = hcWaterInterfacialTension * m_cosContactAngle ( phase ) / ( BrooksCorey::CosMercuryAirContactAngle * BrooksCorey::MercuryAirInterfacialTension ) * cpeHgAir;
      } else {

         double hcWaterInterfacialTension;

         // Values obtained from "Empirical_Capillary_Relationship.pdf", attached to 20528 TFS item.
         if ( phase == pvtFlash::LIQUID_PHASE ) {
            hcWaterInterfacialTension = GeoPhysics::WaterLiquidHcInterfacialTension;
         } else {
            hcWaterInterfacialTension = GeoPhysics::WaterVapourHcInterfacialTension;
         }

         double cpeHgAir = BrooksCorey::computeCapillaryEntryPressure ( permeability * GeoPhysics::M2TOMILLIDARCY, capC1 (), tenPowerCapC2 ());

         // Convert to a hc-water entry pressure.
         pce = hcWaterInterfacialTension * m_cosContactAngle ( phase ) / ( BrooksCorey::CosMercuryAirContactAngle * BrooksCorey::MercuryAirInterfacialTension ) * cpeHgAir;
      }

   } else {
      pce = BrooksCorey::CapillaryEntryPressure;
   }

   return pce;
}

//------------------------------------------------------------//

double Lithology::capillaryEntryPressureOilGas ( const double permeability,
                                                 const double brinePressure ) const {

   double pceHgAir;
   double pceog;
   double hgAigToOilGasConversionFactor;

   // Ratio of interfacial-tensions and cos(contact-angle).
   hgAigToOilGasConversionFactor = BrooksCorey::oilGasInterfacialTension ( brinePressure ) * BrooksCorey::CosOilGasContactAngle /
                                   //------------------------------------------------------------------------------------------//
                                      ( BrooksCorey::MercuryAirInterfacialTension * BrooksCorey::CosMercuryAirContactAngle );

   pceHgAir = BrooksCorey::computeCapillaryEntryPressure ( permeability * GeoPhysics::M2TOMILLIDARCY, capC1 (), tenPowerCapC2 ());
   pceog = pceHgAir * hgAigToOilGasConversionFactor;

   return pceog;
}

//------------------------------------------------------------//

double Lithology::brooksAndCoreyCapillaryPressure ( const pvtFlash::PVTPhase phase,
                                                    const Saturation         saturation,
                                                    const double             pressure,
                                                    const double             temperature,
                                                    const double             permeability,
                                                    const double             brineDensity,
                                                    // const double             hcPhaseDensity,
                                                    const PVTPhaseValues&    hcPhaseDensity,
                                                    const double             criticalTemperature ) const {

   if ( getLiquidWaterCapPresExponent () == IBSNULLVALUE ) {
      // What should the correct values be here?
      return capillaryEntryPressure ( phase,
                                      temperature,
                                      permeability,
                                      brineDensity,
                                      hcPhaseDensity ( phase ),
                                      criticalTemperature );

   } else if ( phase == pvtFlash::LIQUID_PHASE or ( phase == pvtFlash::VAPOUR_PHASE and not FastcauldronSimulator::getInstance ().useGasPressure ())) {

      double hcWaterEntryPressure = capillaryEntryPressure ( phase,
                                                             temperature,
                                                             permeability,
                                                             brineDensity,
                                                             hcPhaseDensity ( phase ),
                                                             criticalTemperature );

      // If phase = vapour we will use the liquid exponent.
      return BrooksCorey::computeCapillaryPressure ( saturation ( Saturation::WATER ),
                                                     getLiquidWaterCapPresExponent (),
                                                     getIrreducibleWaterSaturation (),
                                                     hcWaterEntryPressure );

   } else { // phase == pvtFlash::VAPOUR_PHASE and FastcauldronSimulator::getInstance ().useGasPressure ()

      double oilWaterEntryPressure = capillaryEntryPressure ( pvtFlash::LIQUID_PHASE,
                                                              temperature,
                                                              permeability,
                                                              brineDensity,
                                                              hcPhaseDensity ( pvtFlash::LIQUID_PHASE ),
                                                              criticalTemperature );

      double oilWaterCapillaryPressure = BrooksCorey::computeCapillaryPressure ( saturation ( Saturation::WATER ),
                                                                                 getLiquidWaterCapPresExponent (),
                                                                                 getIrreducibleWaterSaturation (),
                                                                                 oilWaterEntryPressure );


      double gasOilEntryPressure = capillaryEntryPressureOilGas ( permeability, pressure );

      double gasOilCapillaryPressure = BrooksCorey::computeCapillaryPressure ( 1.0 - saturation ( Saturation::VAPOUR ),
                                                                               getVapourLiquidCapPresExponent (),
                                                                               getResidualLiquidSaturation () + getIrreducibleWaterSaturation (),
                                                                               gasOilEntryPressure );

      return oilWaterCapillaryPressure + gasOilCapillaryPressure;

   }

}

//------------------------------------------------------------//

double Lithology::capillaryPressure ( const pvtFlash::PVTPhase phase,
                                      const Saturation         saturation,
                                      const double             pressure,
                                      const double             temperature,
                                      const double             permeability,
                                      const double             brineDensity,
                                      // const double             hcPhaseDensity,
                                      const PVTPhaseValues&    hcPhaseDensity,
                                      const double             criticalTemperature ) const {

   return brooksAndCoreyCapillaryPressure ( phase,
                                            saturation,
                                            pressure,
                                            temperature,
                                            permeability,
                                            brineDensity,
                                            hcPhaseDensity,
                                            criticalTemperature );

}

//------------------------------------------------------------//
