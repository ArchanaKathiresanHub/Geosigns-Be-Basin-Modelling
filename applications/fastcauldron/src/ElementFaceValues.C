#include "ElementFaceValues.h"

#include <cmath>

std::string ElementFaceValues::image () const {

   std::stringstream buffer;
   int i;

   buffer << " Values = { " << VolumeData::boundaryIdImage ( VolumeData::GAMMA_1 ) 
          << " => " 
          << operator ()( VolumeData::GAMMA_1 );

   for ( i = 1; i < VolumeData::NumberOfBoundaries; ++i ) {
      VolumeData::BoundaryId face = VolumeData::BoundaryId ( i );

      buffer << ", " << VolumeData::boundaryIdImage ( face ) << " => " << operator ()( face );
   }

   buffer << " }";

   return buffer.str ();
}

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

ElementFaceValues operator+( const ElementFaceValues& left, const ElementFaceValues& right ) {

   ElementFaceValues result;

   result ( VolumeData::GAMMA_1 ) = left ( VolumeData::GAMMA_1 ) + right ( VolumeData::GAMMA_1 );
   result ( VolumeData::GAMMA_2 ) = left ( VolumeData::GAMMA_2 ) + right ( VolumeData::GAMMA_2 );
   result ( VolumeData::GAMMA_3 ) = left ( VolumeData::GAMMA_3 ) + right ( VolumeData::GAMMA_3 );
   result ( VolumeData::GAMMA_4 ) = left ( VolumeData::GAMMA_4 ) + right ( VolumeData::GAMMA_4 );
   result ( VolumeData::GAMMA_5 ) = left ( VolumeData::GAMMA_5 ) + right ( VolumeData::GAMMA_5 );
   result ( VolumeData::GAMMA_6 ) = left ( VolumeData::GAMMA_6 ) + right ( VolumeData::GAMMA_6 );
   
   return result;
}

ElementFaceValues operator-( const ElementFaceValues& left, const ElementFaceValues& right ) {

   ElementFaceValues result;

   result ( VolumeData::GAMMA_1 ) = left ( VolumeData::GAMMA_1 ) - right ( VolumeData::GAMMA_1 );
   result ( VolumeData::GAMMA_2 ) = left ( VolumeData::GAMMA_2 ) - right ( VolumeData::GAMMA_2 );
   result ( VolumeData::GAMMA_3 ) = left ( VolumeData::GAMMA_3 ) - right ( VolumeData::GAMMA_3 );
   result ( VolumeData::GAMMA_4 ) = left ( VolumeData::GAMMA_4 ) - right ( VolumeData::GAMMA_4 );
   result ( VolumeData::GAMMA_5 ) = left ( VolumeData::GAMMA_5 ) - right ( VolumeData::GAMMA_5 );
   result ( VolumeData::GAMMA_6 ) = left ( VolumeData::GAMMA_6 ) - right ( VolumeData::GAMMA_6 );
   
   return result;
}

double ElementFaceValues::sum () const {
   return m_values [ 0 ] + m_values [ 1 ] + m_values [ 2 ] + m_values [ 3 ] + m_values [ 4 ] + m_values [ 5 ];
}

double ElementFaceValues::sumAbs () const {
   return std::abs ( m_values [ 0 ]) + std::abs ( m_values [ 1 ]) + std::abs ( m_values [ 2 ]) + std::abs ( m_values [ 3 ]) + std::abs ( m_values [ 4 ]) + std::abs ( m_values [ 5 ]);
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
