#ifndef BROOKS_COREY_H
#define BROOKS_COREY_H

#include <assert.h>
#include <math.h>

class BrooksCorey
{
public:

   //the following is assumed to be fixed at the moment
   static const double Sir; //connnate saturation -- irreducible saturaiton
   static const double Pe;//entry pressure = 1 MPa
   static const double Sor; //residual oil saturation
   static const double Krwor; // end point relative permeability of Brine
   static const double Krocw; // end point relative permeability of HC
   static const double Adjustment;

   BrooksCorey();

   static double getSor () {
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
   static double computeCapillaryEntryPressure ( const double permeability,
                                                 const double c1,
                                                 const double c2 ) {
      return 1.0e6 * c2 * pow ( permeability, -c1 );
   }

   // =========== The Brooks-Corey capillary pressure function  ===========//
   // Params: wetting saturation, connate/irreducible saturation, entry pressure, lambda (exponent)
   static double pc ( const double Sw, const double lambda, const double pce = Pe )
   {

      double Sr;

      if ( Sw <= Sir + Adjustment ) {
         Sr = Adjustment / ( 1.0 - Sir ); 
     } else if ( Sw == 1.0 ) {
         Sr = 1; // is this condition correct?
      } else {
         Sr = ( Sw - Sir ) / ( 1.0 - Sir );
      }

      if ( Sw == 1 ) {
         return pce; //  Pe = Pc(Sw==1) 
      }

#if 0
      if(Sw <= Sir)
	return 0;
      //effective saturation

      double Sr =  (Sw - Sir)/(1 - Sir); 
#endif

      assert(0 <= Sr && Sr <= 1);
	
      return pce*pow(Sr, -lambda);
    }
   // ===========The Brooks-Corey relative permeability function ===========//
   //Brine relative permeability
   static double krw(double Sw,  double lambda)
   {

      double Swe;

      if ( Sw >= 1.0 - Sir - Sor ) {
         Swe = 1.0;
      } else if ( Sw <= Sir + Adjustment ) {
         Swe = Adjustment / ( 1.0 - Sir - Sor );
      } else {
         Swe = ( Sw - Sir ) / ( 1.0 - Sir - Sor );
      }

#if 0
       if ( Sw < Sir ) {
          return 0.0;
       }


       if ( Sw > 1.0 - Sor ) {
          return Krwor;
       }

       double Swe =  (Sw - Sir)/(1 - Sir - Sor);
#endif

#if 0
       if ( 0 > Swe or Swe > 1 or not ( 0 <= Swe && Swe <= 1 )) {
          cout << endl << endl << " krw " << Sw << "  " << Swe << "  " << lambda << endl << endl << flush;
       } 
#endif

       assert(0 <= Swe && Swe <= 1);

       return Krwor*pow(Swe, lambda);
    }

   //HC relative permeability as function of brine saturation
   static double kro(double Sw, double lambda)
   {

      double Swe;

      if ( Sw >= 1.0 - Sor ) {
         Swe = 0.0;
      } else if ( Sw <= Sir ) {
         Swe = 1.0;
      } else {
         Swe = ( 1.0 - Sw - Sor ) / ( 1.0 - Sir - Sor ); 
      }
      

#if 0
       if(Sw<=Sir) return 1.0;

       if ( Sw > 1.0 - Sor ) {
          return 0.0;
       }

       double Swe =  (1- Sw - Sor)/(1 - Sir - Sor); 

#endif

       assert(0 <= Swe && Swe <= 1);
       return Krocw*pow(Swe, lambda);
    }


  
};



#endif
