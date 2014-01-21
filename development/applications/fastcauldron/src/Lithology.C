#include "mpi.h"
#include "Lithology.h"
#include "BrooksCorey.h"
#include <cmath>
#include <sstream>

#include "GeoPhysicalConstants.h"

#include "NumericFunctions.h"
#include "FastcauldronSimulator.h"

Lithology::Lithology ( GeoPhysics::ProjectHandle* projectHandle ) : GeoPhysics::CompoundLithology ( projectHandle ) {
   m_lithologyId = -9999;

   m_contactAngle.zero ();

   m_contactAngle ( pvtFlash::VAPOUR_PHASE ) = M_PI;
   m_contactAngle ( pvtFlash::LIQUID_PHASE ) = 150.0 / 180.0 * M_PI;

   // m_contactAngle ( pvtFlash::VAPOUR_PHASE ) = 0.0;
   // m_contactAngle ( pvtFlash::LIQUID_PHASE ) = 30.0 / 180.0 * M_PI;

   m_cosContactAngle ( pvtFlash::VAPOUR_PHASE ) = std::cos ( m_contactAngle ( pvtFlash::VAPOUR_PHASE ));
   m_cosContactAngle ( pvtFlash::LIQUID_PHASE ) = std::cos ( m_contactAngle ( pvtFlash::LIQUID_PHASE ));

   // 180 Degrees
   m_cosHcWaterContactAngle = std::cos ( M_PI );

   // 140 Degrees
   m_cosHgAirContactAngle = std::cos ( 140.0 / 180.0 * M_PI );

   // 480 mN/metre = 0.48 N/m
   m_hgAirInterfacialTension = 0.48;

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

   if ( getPcKrModel () == DataAccess::Interface::BROOKS_COREY_MODEL ) {
      return brooksAndCoreyRelativePermeability ( phase, saturation );
   } else {
      return temisRelativePermeability ( phase, saturation ( phase ));
   }

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
         return BrooksCorey::krw ( saturation ( Saturation::WATER ), getWaterRelPermExponent ()); // water relative permeability
      }

   } else if ( phase == Saturation::LIQUID || phase == Saturation::VAPOUR ) {

      if ( getHcRelPermExponent () == IBSNULLVALUE ) {
         // What should the correct values be here?
         // 0.0 because we would like for there to be no transport in such lithologies (normally).
         return 0.0;
      } else {
         // NOTE Sir (irreducible water) = 0.1 is fixed;

#if 1
         return BrooksCorey::kro ( saturation ( Saturation::WATER ), getHcRelPermExponent ()); // Liquid and Vapour relative permeability
#else
         if ( saturation ( phase ) < BrooksCorey::IrreducibleHcSaturation ) {
            return 0.0;
         } else {
            return BrooksCorey::kro ( saturation ( Saturation::WATER ), getHcRelPermExponent ()); // Liquid and Vapour relative permeability
         }
#endif

      }

   }

   return relPerm;
}


//------------------------------------------------------------//

double Lithology::temisRelativePermeability ( const Saturation::Phase phase,
                                              const double            saturation ) const {

   double minHcSaturation = FastcauldronSimulator::getInstance ().getMinimumHcSaturation ();
   double maxHcSaturation = FastcauldronSimulator::getInstance ().getMaximumHcSaturation ();
   double relPerm;

   if ( phase == Saturation::WATER ) {
      double waterCurveExponent = FastcauldronSimulator::getInstance ().getWaterCurveExponent ();
      double minWaterSat = 1.0 - maxHcSaturation;

      if ( saturation > 1.0 - minHcSaturation ) {
         relPerm = 1.0;
      } else if ( saturation < minWaterSat ) {
         relPerm = pow (( minWaterSat - minWaterSat ) / ( maxHcSaturation - minHcSaturation ), waterCurveExponent );
      } else {
         relPerm = pow (( saturation - minWaterSat ) / ( maxHcSaturation - minHcSaturation ), waterCurveExponent );
      }

   } else if ( phase == Saturation::LIQUID ) {

      if ( saturation > maxHcSaturation ) {
         relPerm = 1.0;
      } else if ( saturation < minHcSaturation ) {
         relPerm = 0.0; // pow (( minHcSaturation - minHcSaturation ) / ( maxHcSaturation - minHcSaturation ), FastcauldronSimulator::getInstance ().getHcLiquidCurveExponent ());
      } else {
         relPerm = pow (( saturation - minHcSaturation ) / ( maxHcSaturation - minHcSaturation ), FastcauldronSimulator::getInstance ().getHcLiquidCurveExponent ());
      }

   } else {

      if ( saturation > maxHcSaturation ) {
         relPerm = 1.0;
      } else if ( saturation < minHcSaturation ) {
         relPerm = pow (( minHcSaturation - minHcSaturation ) / ( maxHcSaturation - minHcSaturation ), FastcauldronSimulator::getInstance ().getHcVapourCurveExponent ());
      } else {
         relPerm = pow (( saturation - minHcSaturation ) / ( maxHcSaturation - minHcSaturation ), FastcauldronSimulator::getInstance ().getHcVapourCurveExponent ());
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

         // Units of interfacial-tension are mN/M (or dynes/cm?) so they need to be scaled by 0.001 to get into N/M.
         double hcWaterInterfacialTension = 0.001 * CBMGenerics::capillarySealStrength::capTension_H2O_HC ( brineDensity,
                                                                                                            usedHcPhaseDensity,
                                                                                                            temperature + 273.15,
                                                                                                            criticalTemperature );
         double cpeHgAir = BrooksCorey::computeCapillaryEntryPressure ( permeability * GeoPhysics::M2TOMILLIDARCY, capC1 (), tenPowerCapC2 ());

         // Convert to a hc-water entry pressure.
         pce = hcWaterInterfacialTension * m_cosContactAngle ( phase ) / ( m_cosHgAirContactAngle * m_hgAirInterfacialTension ) * cpeHgAir;
      } else {

         double hcWaterInterfacialTension;

         // Values obtained from "Empirical_Capillary_Relationship.pdf", attached to 20528 TFS item.
         if ( phase == pvtFlash::LIQUID_PHASE ) {
            hcWaterInterfacialTension = 0.025;
         } else {
            hcWaterInterfacialTension = 0.05;
         }

         double cpeHgAir = BrooksCorey::computeCapillaryEntryPressure ( permeability * GeoPhysics::M2TOMILLIDARCY, capC1 (), tenPowerCapC2 ());

         // Convert to a hc-water entry pressure.
         pce = hcWaterInterfacialTension * m_cosContactAngle ( phase ) / ( m_cosHgAirContactAngle * m_hgAirInterfacialTension ) * cpeHgAir;
      }

   } else {
      pce = BrooksCorey::CapillaryEntryPressure;
   }

   return pce;
}

//------------------------------------------------------------//

double Lithology::brooksAndCoreyCapillaryPressure ( const pvtFlash::PVTPhase phase,
                                                    const Saturation         saturation,
                                                    const double             temperature,
                                                    const double             permeability,
                                                    const double             brineDensity,
                                                    const double             hcPhaseDensity,
                                                    const double             criticalTemperature ) const {

   double pce;

   pce = capillaryEntryPressure ( phase, temperature, permeability, brineDensity, hcPhaseDensity, criticalTemperature );

   if ( getHcCapPresExponent () == IBSNULLVALUE ) {
      // What should the correct values be here?
      return pce;
   } else {
      return BrooksCorey::computeCapillaryPressure ( saturation ( Saturation::WATER ), getHcCapPresExponent (), pce );//, getIrreducibleWaterSaturation ());
   }

}

//------------------------------------------------------------//

double Lithology::temisCapillaryPressure ( const pvtFlash::PVTPhase phase,
                                           const double             saturation ) const {
   assert ( false && "Temis capillary pressure has not yet been implemented. " );
   return 0.0;
}

//------------------------------------------------------------//

double Lithology::capillaryPressure ( const pvtFlash::PVTPhase phase,
                                      const Saturation         saturation,
                                      const double             temperature,
                                      const double             permeability,
                                      const double             brineDensity,
                                      const double             hcPhaseDensity,
                                      const double             criticalTemperature ) const {

   if ( getPcKrModel () == DataAccess::Interface::BROOKS_COREY_MODEL ) {
      return brooksAndCoreyCapillaryPressure ( phase,
                                               saturation,
                                               temperature,
                                               permeability,
                                               brineDensity,
                                               hcPhaseDensity,
                                               criticalTemperature );
   } else {
      return temisCapillaryPressure ( phase,
                                      saturation ( Saturation::convert ( phase )));
   }

}

//------------------------------------------------------------//

#if 0
double Lithology::capillaryPressure ( const Saturation::Phase phase,
                                      const Saturation        saturation,
                                      const double            permeability ) const {

   // two phase system: HCVapour-Brine and HCLiquid-Brine
   //Brine is wetting phase
   Saturation::Phase wettingPhase= Saturation::WATER;

   double capillaryEntryPressure;

   if ( FastcauldronSimulator::getInstance ().useCalculatedCapillaryEntryPressure ()) {
      capillaryEntryPressure = BrooksCorey::computeCapillaryEntryPressure ( permeability * GeoPhysics::M2TOMILLIDARCY, capC1 (), tenPowerCapC2 ());
   } else {
      capillaryEntryPressure = BrooksCorey::CapillaryEntryPressure;
   }

   if ( lambdaPc () == IBSNULLVALUE ) {
      // What should the correct values be here?
      return capillaryEntryPressure;
   } else {
      return BrooksCorey::computeCapillaryPressure ( saturation ( wettingPhase ), lambdaPc(), capillaryEntryPressure );
   }

}
#endif

//------------------------------------------------------------//

#if 0
double Lithology::capillaryPressure ( const pvtFlash::PVTPhase phase,
                                      const double             temperature,
                                      const double             criticalTemperature,
                                      const double             hcPhaseSaturation,
                                      const double             hcPhaseDensity,
                                      const double             brineDensity,
                                      const double             porosity ) const {

   // static const double A = ;
   // static const double B = ;

   double cp;

   if ( hcPhaseDensity > brineDensity ) {
      cp = 0.0;
   } else {
      // Units of interfacial-tension are mN/M so they need to be scaled by 0.001 to get into N/M.
      double interfacialTension = CBMGenerics::capillarySealStrength::capTension_H2O_HC ( brineDensity, hcPhaseDensity, temperature + 273.15, criticalTemperature );
      // double interfacialTension = 111.0 * pow ( 0.001 * ( brineDensity - hcPhaseDensity ), 1.024 ) * pow ( ( temperature + 273.15 ) / criticalTemperature, 1.25 );
      // double interfacialTension;

      // interfacialTension = ( iftTerm * iftTerm ) * ( iftTerm * iftTerm ) / ( criticalTemperature * sqrt ( sqrt ( criticalTemperature )));

      cp = 0.001 * interfacialTension * m_cosContactAngle ( phase ) * specificSurfaceArea () * density () * exp ( -( 1.0 - hcPhaseSaturation ) * 2.0 * sqrt ( geometricVariance ())) * ( 1.0 - porosity ) / porosity;
   }

   return cp;
}
#endif

//------------------------------------------------------------//
