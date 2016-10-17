//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#include "mpi.h"
#include "Lithology.h"
#include "BrooksCorey.h"
#include <cmath>
#include <sstream>

#include "GeoPhysicalConstants.h"

#include "NumericFunctions.h"
#include "FastcauldronSimulator.h"

// utilities library
#include "ConstantsNumerical.h"
using Utilities::Numerical::IbsNoDataValue;
#include "ConstantsMathematics.h"
using Utilities::Maths::M2ToMillyDarcy;

Lithology::Lithology ( GeoPhysics::ProjectHandle* projectHandle ) : GeoPhysics::CompoundLithology ( projectHandle ) {
   m_lithologyId = -9999;

   m_contactAngle.zero ();

   m_contactAngle ( pvtFlash::VAPOUR_PHASE ) =  0.0;
   m_contactAngle ( pvtFlash::LIQUID_PHASE ) = 30.0 / 180.0 * M_PI;

   m_cosContactAngle ( pvtFlash::VAPOUR_PHASE ) = std::cos ( m_contactAngle ( pvtFlash::VAPOUR_PHASE ));
   m_cosContactAngle ( pvtFlash::LIQUID_PHASE ) = std::cos ( m_contactAngle ( pvtFlash::LIQUID_PHASE ));
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

   //1)Brine or Water relative permeability
   //2)HCLiquid relative permeability
   //3)HCVapour relative permeability
   
   double relPerm = 1.0;
      
   //Brine is wetting phase
   
   if ( phase == Saturation::WATER ) {

      if ( LambdaKr () == IbsNoDataValue ) {
         // What should the correct values be here?
         // 1.0 here because we would like the brine-pressure solver to run normally.
         return 1.0;
      } else {
         GeoPhysics::BrooksCorey brooksCorey;
         return brooksCorey.krw ( saturation ( Saturation::WATER ), LambdaKr()); // water relative permeability
      }

   } else if ( phase == Saturation::LIQUID || phase == Saturation::VAPOUR ) {

      if ( LambdaKr () == IbsNoDataValue ) {
         // What should the correct values be here?
         // 0.0 because we would like for there to be no transport in such lithologies (normally).
         return 0.0;
      } else {
         // NOTE Sir (irreducible water) = 0.1 is fixed;

#if 1
         GeoPhysics::BrooksCorey brooksCorey;
         return brooksCorey.kro ( saturation ( Saturation::WATER ), LambdaKr()); // Liquid and Vapour relative permeability
#else

         if ( saturation ( phase ) < BrooksCorey::Sor ) {
            return 0.0;
         } else {
            return BrooksCorey::kro ( saturation ( Saturation::WATER ), LambdaKr()); // Liquid and Vapour relative permeability
         }
#endif

      }

   }

   return relPerm;
}


//------------------------------------------------------------//

double Lithology::calculateTemisRelPerm ( const Saturation::Phase phase,
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
         relPerm = pow (( minHcSaturation - minHcSaturation ) / ( maxHcSaturation - minHcSaturation ), FastcauldronSimulator::getInstance ().getHcLiquidCurveExponent ());
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

double Lithology::capillaryPressure ( const Saturation::Phase phase,
                                      const Saturation        saturation,
                                      const double            permeability ) const {

   // two phase system: HCVapour-Brine and HCLiquid-Brine
   //Brine is wetting phase
   Saturation::Phase wettingPhase= Saturation::WATER;

   double capillaryEntryPressure;

   if ( FastcauldronSimulator::getInstance ().useCalculatedCapillaryPressure ()) {
      GeoPhysics::BrooksCorey brooksCorey;
      capillaryEntryPressure = brooksCorey.computeCapillaryEntryPressure ( permeability * M2TOMILLIDARCY, capC1 (), tenPowerCapC2 ());
   } else {
      capillaryEntryPressure = GeoPhysics::BrooksCorey::Pe;
   }

   if ( LambdaPc () == IbsNoDataValue ) {
      // What should the correct values be here?
      return capillaryEntryPressure;
      return GeoPhysics::BrooksCorey::Pe;
   } else {
      GeoPhysics::BrooksCorey brooksCorey;
      return brooksCorey.pc ( saturation ( wettingPhase ), LambdaPc(), capillaryEntryPressure );
   }

}

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
