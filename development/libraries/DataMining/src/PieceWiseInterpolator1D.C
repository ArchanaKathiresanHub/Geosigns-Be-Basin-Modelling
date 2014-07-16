#include "PieceWiseInterpolator1D.h"

const double Numerics::PieceWiseInterpolator1D::NullValue = 99999.0;

const std::string& Numerics::PieceWiseInterpolator1D::InterpolationKindImage ( const InterpolationKind kind ) {

   static std::string images [ 3 ] = { "PIECEWISE_LINEAR", "CUBIC_SPLINE", "UNKNOWN" };

   if ( kind == PIECEWISE_LINEAR or kind == CUBIC_SPLINE ) {
      return images [ kind ];
   } else {
      return images [ 2 ];
   }

}

const std::string& Numerics::PieceWiseInterpolator1D::InterpolationKindShortImage ( const InterpolationKind kind ) {

   static std::string images [ 3 ] = { "linear", "cubic", "UNKNOWN" };

   if ( kind == PIECEWISE_LINEAR or kind == CUBIC_SPLINE ) {
      return images [ kind ];
   } else {
      return images [ 2 ];
   }

}


Numerics::PieceWiseInterpolator1D::PieceWiseInterpolator1D () {
   m_kind = PIECEWISE_LINEAR;
   m_frozen = false;
}

Numerics::PieceWiseInterpolator1D::~PieceWiseInterpolator1D () {
   m_xs.clear ();
   m_ys.clear ();
   m_as.clear ();
   m_bs.clear ();
   m_cs.clear ();
   m_ds.clear ();
}

void Numerics::PieceWiseInterpolator1D::addPoint ( const double x,
                                                   const double y ) {

   if ( not m_frozen ) {
      m_xs.push_back ( x );
      m_ys.push_back ( y );
   }

}

void Numerics::PieceWiseInterpolator1D::freeze ( const InterpolationKind kind,
                                                 const bool              allowExtrapolation ) {

   if ( m_frozen ) {
      return;
   }

   m_kind = kind;
   m_allowExtrapolation = allowExtrapolation;

   if ( m_kind == PIECEWISE_LINEAR ) {
      computeLinear ();
   } else {
      computeCubic ();
   }

   m_frozen = true;
}

double Numerics::PieceWiseInterpolator1D::operator ()( const double s ) const {

   if ( m_kind == PIECEWISE_LINEAR ) {
      return evaluateLinear ( s );
   } else {
      return evaluateCubic ( s );
   }

}

void Numerics::PieceWiseInterpolator1D::computeLinear () {

   unsigned int i;
   double divisor;

   m_as.resize ( m_xs.size () - 1 );
   m_bs.resize ( m_xs.size () - 1 );

   for ( i = 0; i < m_xs.size () - 1; ++i ) {
    divisor = 1.0 / ( m_xs [ i + 1 ] - m_xs [ i ]);

    m_as [ i ] = divisor * ( m_xs [ i + 1 ] * m_ys [ i     ] - 
                             m_xs [ i     ] * m_ys [ i + 1 ]);

    m_bs [ i ] = divisor * ( m_ys [ i + 1 ] - m_ys [ i ]);

  }

}

void Numerics::PieceWiseInterpolator1D::computeCubic () {

   const unsigned int size = m_xs.size ();

   int i;

   DoubleArray hs ( size - 1 );
   DoubleArray alphas ( size - 1 );
   DoubleArray ls ( size );
   DoubleArray mus ( size );
   DoubleArray zs ( size );

   m_as.resize ( m_xs.size ());
   m_bs.resize ( m_xs.size () - 1 );
   m_cs.resize ( m_xs.size ());
   m_ds.resize ( m_xs.size () - 1 );


   for ( i = 0; i < size; ++i ) {
      m_as [ i ] = m_ys [ i ];
   }

   for ( i = 0; i < size - 1; ++i ) {
      hs [ i ] = m_xs [ i + 1 ] - m_xs [ i ];
   }

   for ( i = 1; i < size - 1; ++i ) {
      alphas [ i ] = 3.0 * ( m_as [ i + 1 ] * hs [ i - 1 ] - m_as [ i ] * ( m_xs [ i + 1 ] - m_xs [ i - 1 ]) + m_as [ i - 1 ] * hs [ i ]) / 
                     //-------------------------------------------------------------------------------------------------------------------//
                                                                ( hs [ i - 1 ] * hs [ i ]);
   }

   ls  [ 0 ] = 1.0;
   mus [ 0 ] = 0.0;
   zs  [ 0 ] = 0.0;

   for ( i = 1; i < size - 1; ++i ) {
      ls  [ i ] = 2.0 * ( m_xs [ i + 1 ] - m_xs [ i - 1 ]) - hs [ i - 1 ] * mus [ i - 1 ];
      mus [ i ] = hs [ i ] / ls [ i ];
      zs  [ i ] = ( alphas [ i ] - hs [ i - 1 ] * zs [ i - 1 ]) / ls [ i ];
   }

   ls [ size - 1 ] = 1.0;
   zs [ size - 1 ] = 0.0;
   m_cs [ size - 1 ] = 0.0;

   for ( i = size - 2; i >= 0; --i ) {
      m_cs [ i ] = zs [ i ] - mus [ i ] * m_cs [ i + 1 ];
      m_bs [ i ] = ( m_as [ i + 1 ] - m_as [ i ]) / hs [ i ] - hs [ i ] * ( m_cs [ i + 1 ] + 2.0 * m_cs [ i ]) / 3.0;
      m_ds [ i ] = ( m_cs [ i + 1 ] - m_cs [ i ]) / ( 3.0 * hs [ i ]);
   }

}

double Numerics::PieceWiseInterpolator1D::evaluateLinear ( const double s ) const {

   int panel = findPanel ( s );

   if ( 0 <= panel and panel < (int)(m_as.size ())) {
      return m_bs [ panel ] * s + m_as [ panel ];
   } else {

      if ( m_allowExtrapolation ) {

         if ( panel < 0 ) {
            return m_bs [ 0 ] * s + m_as [ 0 ];
         } else {
            return m_bs [ m_bs.size () - 1 ] * s + m_as [ m_as.size () - 1 ];
         }

      } else {
         return NullValue;
      }

   }

}

double Numerics::PieceWiseInterpolator1D::evaluateCubic ( const double s ) const {

   int panel = findPanel ( s );

   if ( 0 <= panel and panel < (int)(m_xs.size ())) {
      double difference = s - m_xs [ panel ];
      return (( m_ds [ panel ] * difference + m_cs [ panel ] ) * difference + m_bs [ panel ]) * difference + m_as [ panel ];
   } else {
      return NullValue;
   }

}

int Numerics::PieceWiseInterpolator1D::findPanel ( const double s ) const {

   if ( s < m_xs [ 0 ]) {
      return -1;
   } else {
      int i;

      for ( i = 1; i <= (int)(m_xs.size ()); ++i ) {

         if ( s < m_xs [ i ]) {
            return i - 1;
         }

      }

      return m_xs.size ();
   }

}

