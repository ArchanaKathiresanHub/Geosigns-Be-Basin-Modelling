#include <iostream>
#include <cmath>
#include <cassert>
#include <cstdlib>

#include "BrooksCorey.h"

static const double DefaultC1 = 0.33;
static const double DefaultC2 = pow ( 10.0, -0.2611 );

bool isEqual ( const double x, const double y, const double tolerance = 1.0e-10 );


int main () {

   double permeability;
   double pce;

   permeability = 1.0e-8;
   pce = BrooksCorey::computeCapillaryEntryPressure ( permeability, DefaultC1, DefaultC2 );
   assert ( isEqual ( pce, 239276473.852775 ));

   permeability = 1.0e-6;
   pce = BrooksCorey::computeCapillaryEntryPressure ( permeability, DefaultC1, DefaultC2 );
   assert ( isEqual ( pce, 52347988.7009069 ));

   permeability = 1.0e-4;
   pce = BrooksCorey::computeCapillaryEntryPressure ( permeability, DefaultC1, DefaultC2 );
   assert ( isEqual ( pce, 11452492.0770789 ));

   permeability = 1.0e-2;
   pce = BrooksCorey::computeCapillaryEntryPressure ( permeability, DefaultC1, DefaultC2 );
   assert ( isEqual ( pce, 2505532.26648195 ));

   permeability = 1.0;
   pce = BrooksCorey::computeCapillaryEntryPressure ( permeability, DefaultC1, DefaultC2 );
   assert ( isEqual ( pce, 548150.734017655 ));

   permeability = 1.0e2;
   pce = BrooksCorey::computeCapillaryEntryPressure ( permeability, DefaultC1, DefaultC2 );
   assert ( isEqual ( pce, 119922.314002360 ));

   return 0;
}



bool isEqual ( const double x, const double y, const double tolerance ) {

   if ( x == y ) {
      return true;
   } else {

      double absX = std::abs ( x );
      double absY = std::abs ( y );

      if ( absX > absY ) {
         return std::abs ( x - y ) <= tolerance * absX;
      } else {
         return std::abs ( x - y ) <= tolerance * absY;
      }

   }

}
