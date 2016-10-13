#include "ElementFaceValues.h"

#include <cmath>

ElementFaceValues& ElementFaceValues::operator*=( const double scalar ) {

   m_values [ 0 ] *= scalar;
   m_values [ 1 ] *= scalar;
   m_values [ 2 ] *= scalar;
   m_values [ 3 ] *= scalar;
   m_values [ 4 ] *= scalar;
   m_values [ 5 ] *= scalar;

   return *this;
}

void ElementFaceValues::zero () {

   m_values [ 0 ] = 0.0;
   m_values [ 1 ] = 0.0;
   m_values [ 2 ] = 0.0;
   m_values [ 3 ] = 0.0;
   m_values [ 4 ] = 0.0;
   m_values [ 5 ] = 0.0;

}

double ElementFaceValues::sumGt0 () const {

   double result = 0.0;
   int i;

   for ( i = 0; i < 6; ++i ) {

      if ( m_values [ i ] > 0.0 ) {
         result += m_values [ i ];
      }

   }

   return result;
}