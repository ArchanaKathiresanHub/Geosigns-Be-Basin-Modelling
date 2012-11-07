#include "PiecewiseInterpolator.h"

#include <sstream>
#include <algorithm>

//------------------------------------------------------------//

ibs::PiecewiseInterpolator::PointerSort::PointerSort ( const double* xVals ) : m_xs ( xVals ) {
}

//------------------------------------------------------------//

ibs::PiecewiseInterpolator::PiecewiseInterpolator () {
  m_xs             = 0;
  m_ys             = 0;
  m_aCoeffs        = 0;
  m_bCoeffs        = 0;
  m_cCoeffs        = 0;
  m_dCoeffs        = 0;
  m_method         = PIECEWISE_LINEAR;
  m_numberOfPoints = 0;
}

//------------------------------------------------------------//

ibs::PiecewiseInterpolator::~PiecewiseInterpolator () {
  deleteCoefficients ();
}

//------------------------------------------------------------//

void ibs::PiecewiseInterpolator::deleteCoefficients () {

  if ( m_xs != 0 ) {
    delete [] m_xs;
    m_xs = 0;

    delete [] m_ys;
    m_ys = 0;

    delete [] m_aCoeffs;
    delete [] m_bCoeffs;
    m_aCoeffs = 0;
    m_bCoeffs = 0;
  }

  // The next 2 are not null if the interpolant was a cubic spline
  if ( m_cCoeffs != 0 ) {
    delete [] m_cCoeffs;
    delete [] m_dCoeffs;
    m_cCoeffs = 0;
    m_dCoeffs = 0;
  }

}

//------------------------------------------------------------//

double ibs::PiecewiseInterpolator::evaluate ( const double value ) const {

  if ( m_method == PIECEWISE_LINEAR ) {
    return evaluatePiecewiseLinear ( value );
  } else { // m_method == Cubic_Spline
    return evaluateCubicSpline ( value );
  }

}

//------------------------------------------------------------//

double ibs::PiecewiseInterpolator::evaluateDerivative ( const double value ) const {

  if ( m_method == PIECEWISE_LINEAR ) {
    return evaluatePiecewiseLinearDerivative ( value );
  } else { // m_method == Cubic_Spline
    return evaluateCubicSplineDerivative ( value );
  }

}

//------------------------------------------------------------//

double ibs::PiecewiseInterpolator::evaluatePiecewiseLinear ( const double value ) const {

  int    panelNumber = findPanel ( value );
  double Result;


  Result = m_aCoeffs [ panelNumber ] + m_bCoeffs [ panelNumber ] * value;

  return Result;
}

//------------------------------------------------------------//

double ibs::PiecewiseInterpolator::evaluatePiecewiseLinearDerivative ( const double value ) const {

  int panelNumber = findPanel ( value );

  return m_bCoeffs [ panelNumber ];
}

//------------------------------------------------------------//

double ibs::PiecewiseInterpolator::evaluateCubicSpline ( const double value ) const {

  int panelNumber = findPanel ( value );
  double difference;
  double Result;

  if ( panelNumber == -1 ) {
    Result = m_m0 * value + m_c0;
  } else if ( panelNumber == m_numberOfPoints ) {
    Result = m_mN * value + m_cN;
  } else {

    difference = value - m_xs [ panelNumber ];
    //
    //
    // evaluate spline using Horners m_method. P ( x ) = (( D x + C ) x + B ) x + A
    //
    Result = (( m_dCoeffs [ panelNumber ]  * difference +
                m_cCoeffs [ panelNumber ]) * difference +
                m_bCoeffs [ panelNumber ]) * difference + m_aCoeffs [ panelNumber ];
  }

  return Result;
}

//------------------------------------------------------------//

double ibs::PiecewiseInterpolator::evaluateCubicSplineDerivative ( const double value ) const {

  int panelNumber = findPanel ( value );
  double difference;
  double Result;

  if ( panelNumber == -1 ) {
    Result = m_m0;
  } else if ( panelNumber == m_numberOfPoints ) {
    Result = m_mN;
  } else {

    difference = value - m_xs [ panelNumber ];
    //
    //
    // Evaluate spline using Horners m_method. P  ( x ) = (( D x + C ) x + B ) x + A
    //                                      => P' ( x ) = (( 3 * D x + 2 * C ) x + B
    //
    Result = ( 3.0 * m_dCoeffs [ panelNumber ]  * difference + 2.0 * m_cCoeffs [ panelNumber ]) * difference + m_bCoeffs [ panelNumber ];
  }

  return Result;
}

//------------------------------------------------------------//

void ibs::PiecewiseInterpolator::setInterpolation
   ( const InterpolationMethod newInterpolationMethod,
     const int                 newNumberOfPoints,
     const double*             newXs,
     const double*             newYs ) {

   int* pointer = new int [ newNumberOfPoints ];
   int i;

   PointerSort comp ( newXs );

   // Initialise the array index pointer.
   for ( i = 0; i < newNumberOfPoints; ++i ) {
      pointer [ i ] = i;
   }

   m_method         = newInterpolationMethod;
   m_numberOfPoints = newNumberOfPoints;

#if 0
   if ( m_numberOfPoints < 4 ) {
      //
      //
      // Need at least 4 points for a Cubic Spline interpolant!
      //
      m_method = PIECEWISE_LINEAR;
   } else {
      m_method = CUBIC_SPLINE;
   }
#endif

   m_method = PIECEWISE_LINEAR;

   if ( m_numberOfPoints != 0 ) {
      deleteCoefficients ();
   }

   m_xs = new double [ m_numberOfPoints ];
   m_ys = new double [ m_numberOfPoints ];

   if ( m_method == PIECEWISE_LINEAR ) {
      m_aCoeffs = new double [ m_numberOfPoints - 1 ];
      m_bCoeffs = new double [ m_numberOfPoints - 1 ];
   } else {
      m_aCoeffs = new double [ m_numberOfPoints ];
      m_bCoeffs = new double [ m_numberOfPoints - 1 ];
      m_cCoeffs = new double [ m_numberOfPoints - 1 ];
      m_dCoeffs = new double [ m_numberOfPoints - 1 ];
   }

   // Sort the array index pointer based on the order of the x-values.
   std::sort ( pointer, pointer + m_numberOfPoints, comp );

   // Copy the x- and y-values based on their order given by the index pointer.
   for ( i = 0; i < m_numberOfPoints; i++ ) {
      m_xs [ i ] = newXs [ pointer [ i ]];
      m_ys [ i ] = newYs [ pointer [ i ]];
   }

   delete [] pointer;

}

//------------------------------------------------------------//

void ibs::PiecewiseInterpolator::computePiecewiseLinearCoefficients () {

  int    i;
  double divisor;


  //             ( x - a )            ( b - x )
  //     P(x) =  --------- * f(b)  +  --------- * f (a)
  //             ( b - a )            ( b - a )
  // 
  // 
  //             x * ( f(b) - f(a))    b * f (a)    a * f (b)
  //          =  ------------------ +  --------  -  ---------
  //                  ( b - a )        ( b - a )    ( b - a )
  // 
  //          = A + B x
  // 
  // 
  //        b * f (a)     a * f (b)
  //   A =  ---------  -  ---------
  //        ( b - a )     ( b - a )
  // 
  // 
  //        ( f(b) - f(a))    
  //   B  =  --------------
  //           ( b - a )
  // 
  // 
  // 
  //  Rearranging the calculation like this reduces the flop count
  //  considerably, from 6 (including a division) to 2, per evaluation.
  // 
  // 
  for ( i = 0; i < m_numberOfPoints - 1; i++ ) {
    divisor = 1.0 / ( m_xs [ i + 1 ] - m_xs [ i ]);

    m_aCoeffs [ i ] = divisor * ( m_xs [ i + 1 ] * m_ys [ i ] -  m_xs [ i ] * m_ys [ i + 1 ]);
    m_bCoeffs [ i ] = divisor * ( m_ys [ i + 1 ] - m_ys [ i ]);

  }

}


//------------------------------------------------------------//

void ibs::PiecewiseInterpolator::computeCubicSplineCoefficients () {

  int i;
  double* hs     = new double [ m_numberOfPoints - 1 ];
  double* alphas = new double [ m_numberOfPoints - 1 ];
  double* ls     = new double [ m_numberOfPoints ];
  double* mus    = new double [ m_numberOfPoints ];
  double* zs     = new double [ m_numberOfPoints ];

  for ( i = 0; i < m_numberOfPoints; i++ ) {
    m_aCoeffs [ i ] = m_ys [ i ];
  }

  for ( i = 0; i < m_numberOfPoints - 1; i++ ) {
    hs [ i ] = m_xs [ i + 1 ] - m_xs [ i ];
  }

  for ( i = 1; i < m_numberOfPoints - 1; i++ ) {
    alphas [ i ] = 3.0 * ( m_aCoeffs [ i + 1 ] * hs [ i - 1 ] - m_aCoeffs [ i ] * ( m_xs [ i + 1 ] - m_xs [ i - 1 ]) + m_aCoeffs [ i - 1 ] * hs [ i ]) / 
                   //-------------------------------------------------------------------------------------------------------------------------//
                                                             ( hs [ i - 1 ] * hs [ i ]);
  }

  ls  [ 0 ] = 1.0;
  mus [ 0 ] = 0.0;
  zs  [ 0 ] = 0.0;

  for ( i = 1; i < m_numberOfPoints - 1; i++ ) {
    ls  [ i ] = 2.0 * ( m_xs [ i + 1 ] - m_xs [ i - 1 ]) - hs [ i - 1 ] * mus [ i - 1 ];
    mus [ i ] = hs [ i ] / ls [ i ];
    zs  [ i ] = ( alphas [ i ] - hs [ i - 1 ] * zs [ i - 1 ]) / ls [ i ];
  }

  ls [ m_numberOfPoints - 1 ] = 1.0;
  zs [ m_numberOfPoints - 1 ] = 0.0;
  m_cCoeffs [ m_numberOfPoints - 1 ] = 0.0;

  for ( i = m_numberOfPoints - 2; i >= 0; i-- ) {
    m_cCoeffs [ i ] = zs [ i ] - mus [ i ] * m_cCoeffs [ i + 1 ];
    m_bCoeffs [ i ] = ( m_aCoeffs [ i + 1 ] - m_aCoeffs [ i ]) / hs [ i ] - hs [ i ] * ( m_cCoeffs [ i + 1 ] + 2.0 * m_cCoeffs [ i ]) / 3.0;
    m_dCoeffs [ i ] = ( m_cCoeffs [ i + 1 ] - m_cCoeffs [ i ]) / ( 3.0 * hs [ i ]);
  }

  delete [] hs;
  delete [] alphas;
  delete [] ls;
  delete [] mus;
  delete [] zs;

  //------------------------------------------------------------//
  //
  //
  // Now compute the extrapolants.
  //
  const int N = m_numberOfPoints - 1;

  m_m0 = m_bCoeffs [ 0 ];
  m_c0 = m_ys [ 0 ] - m_m0 * m_xs [ 0 ];

  double difference = m_xs [ N ] - m_xs [ N - 1 ];

  m_mN = m_bCoeffs [ N - 1 ] + 2.0 * m_cCoeffs [ N - 1 ] * difference + 2.0 * m_dCoeffs [ N - 1 ] * difference * difference;
  m_cN = m_ys [ N ] - m_mN * m_xs [ N ];

}

//------------------------------------------------------------//

void ibs::PiecewiseInterpolator::computeCoefficients () {

  if ( m_xs == 0 ) {
    //
    //
    // There is nothing to compute!
    //
    return;
  }

  if ( m_method == PIECEWISE_LINEAR ) {
    computePiecewiseLinearCoefficients ();
  } else if ( m_method == CUBIC_SPLINE ) {
    computeCubicSplineCoefficients ();
  }

}

//------------------------------------------------------------//

int ibs::PiecewiseInterpolator::findPanel ( const double value ) const {

  int panelNumber;

  if ( m_method == PIECEWISE_LINEAR ) {

    for ( panelNumber = 1; panelNumber < m_numberOfPoints; panelNumber++ ) {

      if ( value < m_xs [ panelNumber ]) {
        return panelNumber - 1;
      }

    }

    return m_numberOfPoints - 2;
  } else {

    for ( panelNumber = 0; panelNumber < m_numberOfPoints; panelNumber++ ) {

      if ( value < m_xs [ panelNumber ]) {
        return panelNumber - 1;
      }

    }

    return m_numberOfPoints;
  }

}

//------------------------------------------------------------//

void ibs::PiecewiseInterpolator::print ( std::ostream& o ) const {

  int i;

  #if defined (sgi)
     unsigned long new_options = std::ios::scientific;
     unsigned long old_options = o.flags ( new_options );
  #else
     std::ios::fmtflags new_options = std::ios::scientific;
     std::ios::fmtflags old_options = o.flags ( new_options );
  #endif

  int Old_Precision = o.precision ( 10 );

  o << " There are " << m_numberOfPoints << " points "  << std::endl;

  o << std::endl << " m_xs | m_ys " << std::endl;

  for ( i = 0; i < m_numberOfPoints; i++ ) {
    o << m_xs [ i ] << "  " << m_ys [ i ] << std::endl;
  }

  if ( m_method == PIECEWISE_LINEAR ) {
    o << std::endl << " m_aCoeffs      |     m_bCoeffs  " << std::endl;

    for ( i = 0; i < m_numberOfPoints - 1; i++ ) {
      o << m_aCoeffs [ i ] << "  " << m_bCoeffs [ i ] << std::endl;
    }

  } else {
    o << std::endl << " m_aCoeffs      |     m_bCoeffs   |    m_cCoeffs    | m_dCoeffs   " << std::endl;

    for ( i = 0; i < m_numberOfPoints - 1; i++ ) {
      o << m_aCoeffs [ i ] << "  " << m_bCoeffs [ i ] << "  " << m_cCoeffs [ i ] << "  " << m_dCoeffs [ i ] << std::endl;
    }

  }

  o.precision ( Old_Precision );
  o.flags ( old_options );

}

//------------------------------------------------------------//

std::string ibs::PiecewiseInterpolator::image () const {

   std::stringstream buffer;
   int I;

   buffer.precision ( 10 );
   buffer.flags ( std::ios::scientific );

   buffer << " permeability interpolator: " << std::endl << " There are " << m_numberOfPoints << " points "  << std::endl;
   buffer << std::endl << " m_xs | m_ys " << std::endl;

   for ( I = 0; I < m_numberOfPoints; I++ ) {
      buffer << m_xs [ I ] << "  " << m_ys [ I ] << std::endl;
   }

   if ( m_method == PIECEWISE_LINEAR ) {
      buffer << std::endl << " m_aCoeffs      |     m_bCoeffs  " << std::endl;

      for ( I = 0; I < m_numberOfPoints - 1; I++ ) {
         buffer << m_aCoeffs [ I ] << "  " << m_bCoeffs [ I ] << std::endl;
      }

   } else {
      buffer << std::endl << " m_aCoeffs      |     m_bCoeffs   |    m_cCoeffs    | m_dCoeffs   " << std::endl;

      for ( I = 0; I < m_numberOfPoints - 1; I++ ) {
         buffer << m_aCoeffs [ I ] << "  " << m_bCoeffs [ I ] << "  " << m_cCoeffs [ I ] << "  " << m_dCoeffs [ I ] << std::endl;
      }

   }

   buffer << std::endl << std::endl;

   return buffer.str ();
}

//------------------------------------------------------------//

ibs::PiecewiseInterpolator& ibs::PiecewiseInterpolator::operator=( const PiecewiseInterpolator& newInterpolator ) {

  if ( m_xs != 0 ) {
    delete [] m_xs;
    delete [] m_ys;
  }

  if ( m_aCoeffs != 0 ) {
    delete [] m_aCoeffs;
    delete [] m_bCoeffs;
  }

  if ( m_cCoeffs != 0 ) {
    delete [] m_cCoeffs;
    delete [] m_dCoeffs;
  }

  ///
  /// Should I compute the new coefficients before deleting the old stuff?
  ///
  setInterpolation ( newInterpolator.m_method, newInterpolator.m_numberOfPoints, newInterpolator.m_xs, newInterpolator.m_ys );
  computeCoefficients ();

  return *this;
}

