//                                                                      
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef BROOKS_COREY_H
#define BROOKS_COREY_H

#include <assert.h>
#include <math.h>


namespace GeoPhysics
{
   
   class BrooksCorey
   {
   public:

      //the following is assumed to be fixed at the moment
      static const double Sir;    // connnate saturation -- irreducible saturaiton
      static const double Pe;     // entry pressure = 1 MPa
      static const double Sor;    // residual oil saturation
      static const double Krwor;  // end point relative permeability of Brine
      static const double Krocw;  // end point relative permeability of HC
      static const double Adjustment;
      static const double log10;

      static const double VapourWaterContactAngle;       /// The contact angle for the gas-water interface.
      static const double LiquidWaterContactAngle;       /// The contact angle for the oil-water interface.
      static const double LiquidVapourContactAngle;      /// The contact angle for the oil-gas interface.
      static const double CosLiquidVapourContactAngle;   /// Cosine of the above
      static const double MercuryAirContactAngle;        /// The contact angle for the mercury-air interface.
      static const double CosMercuryAirContactAngle;     /// Cosine of the above
      static const double MercuryAirInterfacialTension;  /// The interfacial tension for the mercury-air interface. Units: N/m

      inline double getSor()
      {
         return Sor;
      }

      /// \brief The capillary entry pressure.
      ///
      /// Capillary entry pressure is calculated in Pascals.
      /// The permeability must be in milli-Darcy.
      /// \f[
      ///   log P_{ce} = -c1 log k - c2
      /// \f]
      /// Where $P_{ce}$ has units of MPa. and so must be scaled by 1e6 for Pascals.
      double computeCapillaryEntryPressure(const double permeability, const double c1, const double c2);

      double liquidVapourInterfacialTension(const double pressure);

      // =========== The Brooks-Corey capillary pressure function  ===========//
      // Params: wetting saturation, connate/irreducible saturation, entry pressure, lambda (exponent)
      double pc(const double Sw, const double lambda, const double pce = Pe);

      // ===========The Brooks-Corey relative permeability function ===========//
      //Brine relative permeability
      double krw(double Sw, double lambda);

      //HC relative permeability as function of brine saturation
      double kro(double Sw, double lambda);
   };

}



#endif
