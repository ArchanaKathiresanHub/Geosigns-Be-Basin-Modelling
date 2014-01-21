#ifndef BROOKS_COREY_H
#define BROOKS_COREY_H

#include <assert.h>
#include "GeoPhysicalConstants.h"


class BrooksCorey
{
public:

   //the following is assumed to be fixed at the moment

   // connnate saturation -- irreducible water saturaiton -- Sir
   static const double IrreducibleWaterSaturation = GeoPhysics::IrreducibleWaterSaturation;

   // entry pressure = 1 MPa
   static const double CapillaryEntryPressure = 1000000;

   // residual oil saturation -- Sor
   static const double ResidualHcSaturation = GeoPhysics::ResidualHcSaturation;

   // end point relative permeability of Brine
   static const double Krwor = 1.0;

   // end point relative permeability of HC
   static const double Krocw = 1.0;

   static const double Adjustment = 1.0e-4;



   BrooksCorey() {}

   /// \brief The capillary entry pressure.
   ///
   /// Capillary entry pressure is calculated in Pascals.
   /// The permeability must be in milli-Darcy.
   /// \f[
   ///   log P_{ce} = -c1 log k - c2
   /// \f]
   /// Where $P_{ce}$ has units of MPa. and so must be scaled by 1e6 for Pascals.
   static double computeCapillaryEntryPressure ( const double permeability,
                                                 const double c1,
                                                 const double c2 ) {
      return 1.0e6 * c2 * pow ( permeability, -c1 );
   }

   // =========== The Brooks-Corey capillary pressure function  ===========//
   // Params: wetting saturation, connate/irreducible saturation, entry pressure, lambda (exponent)
   static double computeCapillaryPressure ( const double Sw,
                                            const double lambda,
                                            const double sir = IrreducibleWaterSaturation,
                                            const double pce = CapillaryEntryPressure )
   {

      double Sr;

      if ( Sw <= sir + Adjustment ) {
         Sr = Adjustment / ( 1.0 - sir ); 
      } else if ( Sw == 1.0 ) {
         Sr = 1; // is this condition correct?
      } else {
         Sr = ( Sw - sir ) / ( 1.0 - sir );
      }

      if ( Sw == 1 ) {
         return pce; //  CapillaryEntryPressure = Pc(Sw==1) 
      }

      assert(0 <= Sr && Sr <= 1);
	
      return pce*pow(Sr, -lambda);
   }

   // ===========The Brooks-Corey relative permeability function ===========//
   //Brine relative permeability
   static double krw ( const double Sw, 
                       const double lambda,
                       const double sir,
                       const double sor ) {

      double Swe;

      if ( Sw >= 1.0 - sir - sor ) {
         Swe = 1.0;
      } else if ( Sw <= sir + Adjustment ) {
         Swe = Adjustment / ( 1.0 - sir - sor );
      } else {
         Swe = ( Sw - sir ) / ( 1.0 - sir - sor );
      }

      assert(0 <= Swe && Swe <= 1);

      return Krwor*pow(Swe, lambda);
   }

   //HC relative permeability as function of brine saturation
   static double kro ( const double Sw, 
                       const double lambda,
                       const double sir,
                       const double sor ) {

      double Swe;

      if ( Sw >= 1.0 - sor ) {
         Swe = 0.0;
      } else if ( Sw <= sir ) {
         Swe = 1.0;
      } else {
         Swe = ( 1.0 - Sw - sor ) / ( 1.0 - sir - sor ); 
      }
      
      assert(0 <= Swe && Swe <= 1);
      return Krocw*pow(Swe, lambda);
   }
  
};

#endif // end BROOKS_COREY_H
