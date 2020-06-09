#include "Matrix8x8.h"


//------------------------------------------------------------//

void FiniteElementMethod::scale ( Matrix8x8&   Result,  const double Factor ) {


  for ( int i = 0; i <  8 ; ++i ) {
     Result ( 0, i ) *= Factor;
     Result ( 1, i ) *= Factor;
     Result ( 2, i ) *= Factor;
     Result ( 3, i ) *= Factor;
     Result ( 4, i ) *= Factor;
     Result ( 5, i ) *= Factor;
     Result ( 6, i ) *= Factor;
     Result ( 7, i ) *= Factor;
  }

}
