#ifndef BROOKS_COREY_H
#define BROOKS_COREY_H

#include <cassert>
#include <cmath>

#include "GeoPhysicalConstants.h"
#include "EosPack.h"


class BrooksCorey
{
 public:

   /// The natural log of 10.
   ///
   /// Used to eliminate a pow and replace it with an exp, since it is usually faster.
   static const double log10;

   /// The contact angle for the gas-water interface.
   ///
   /// 180 Degrees.
   static const double VapourWaterContactAngle;

   /// The contact angle for the oil-water interface.
   ///
   /// 150 Degrees = 180 - 30.
   static const double LiquidWaterContactAngle;

   /// The contact angle for the oil-gas interface.
   ///
   /// 180 - 21 degrees?
   static const double LiquidVapourContactAngle;

   /// The cosine of the contact angle for the oil-gas interface.
   ///
   /// cos ( OilGasContactAngle degrees ) = cos ( 180 - 21 ).
   static const double CosLiquidVapourContactAngle;

   /// The contact angle for the mercury-air interface.
   ///
   /// 140 degrees.
   static const double MercuryAirContactAngle;

   /// The interfacial tension for the mercury-air interface.
   ///
   /// Units: N/m
   static const double MercuryAirInterfacialTension;

   /// The cos of the contact angle for the mercury-air interface.
   static const double CosMercuryAirContactAngle;

   /// entry pressure = 1 MPa
   static const double CapillaryEntryPressure;


   /// \brief The value by which the saturation is changed in order to prevent an infinity.
   ///
   /// Ony necessary at the upper end of the saturation curve.
   static const double Adjustment;



   /// \brief The capillary entry pressure for mercury-air.
   ///
   /// Capillary entry pressure is calculated in Pascals.
   /// The permeability must be in milli-Darcy.
   /// \f[
   ///   log P_{ce} = -c1 log k - c2
   /// \f]
   static double computeCapillaryEntryPressure ( const double permeability,
                                                 const double c1,
                                                 const double c2 );

   /// Compute the interfacial tension for oil-gas contact.
   ///
   /// \f[ 19.099e-3 c_1 10^{-0.034878 c1} \f]
   /// Units N/m
   static double liquidVapourInterfacialTension ( const double pressure );

   // =========== The Brooks-Corey capillary pressure function  ===========//
   // Params: wetting saturation, connate/irreducible saturation, entry pressure, lambda (exponent)
   static double computeCapillaryPressure ( const double sat,
                                            const double lambda,
                                            const double sir,
                                            const double pce );

   // ===========The Brooks-Corey relative permeability function ===========//
   // Brine relative permeability
   static double krw ( const double waterSaturation, 
                       const double lambda,
                       const double sir,
                       const double sgr,
                       const double sor,
                       const double krwMax );

   // HC relative permeability as function of brine saturation
   static double kro ( const pvtFlash::PVTPhase phase,
                       const double             hcPhaseSaturation, 
                       const double             lambda,
                       const double             sir,
                       const double             sgr,
                       const double             sor,
                       const double             krMax );
  
};

//------------------------------------------------------------//
// Some inline functions

inline double BrooksCorey::computeCapillaryEntryPressure ( const double permeability,
                                                           const double c1,
                                                           const double c2 ) {
   /// Where $P_{ce}$ has units of MPa. and so must be scaled by 1e6 for Pascals.
   return 1.0e6 * c2 * std::pow ( permeability, -c1 );
}


inline double BrooksCorey::liquidVapourInterfacialTension ( const double pressure ) {
   return 19.099e-3 * std::exp ( -0.034878 * pressure * log10 );
}

//------------------------------------------------------------//


#endif // end BROOKS_COREY_H
