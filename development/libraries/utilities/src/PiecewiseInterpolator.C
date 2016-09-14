#include "PiecewiseInterpolator.h"

#include <sstream>
#include <algorithm>

#include "FormattingException.h"

//------------------------------------------------------------//

ibs::PiecewiseInterpolator::PointerSort::PointerSort ( const double* xVals ) : m_xs ( xVals ) {
}

//------------------------------------------------------------//

ibs::PiecewiseInterpolator::PiecewiseInterpolator () {
  m_xs             = 0;
  m_ys             = 0;
  m_aCoeffs        = 0;
  m_bCoeffs        = 0;
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

}

//------------------------------------------------------------//

double ibs::PiecewiseInterpolator::evaluate ( const double value ) const {
  int panelNumber = findPanel ( value );
  return m_aCoeffs [ panelNumber ] + m_bCoeffs [ panelNumber ] * value;
}

//------------------------------------------------------------//

double ibs::PiecewiseInterpolator::evaluateDerivative ( const double value ) const {
   int panelNumber = findPanel ( value );
   return m_bCoeffs [ panelNumber ];
}

//------------------------------------------------------------//

void ibs::PiecewiseInterpolator::setInterpolation ( const int                 newNumberOfPoints,
                                                    const double*             newXs,
                                                    const double*             newYs ) {

   if ( newNumberOfPoints <= 1 ) {
      throw formattingexception::GeneralException () << "Incorrect number of points "
                                                     << newNumberOfPoints
                                                     << ", must be greater than 1";
   }

   if ( newXs == nullptr or newYs == nullptr ) {
      throw formattingexception::GeneralException () << "The data arrays are null.";
   }

   int* pointer = new int [ newNumberOfPoints ];
   int i;

   PointerSort comp ( newXs );

   // Initialise the array index pointer.
   for ( i = 0; i < newNumberOfPoints; ++i ) {
      pointer [ i ] = i;
   }

   if ( m_numberOfPoints != 0 ) {
      deleteCoefficients ();
   }

   m_numberOfPoints = newNumberOfPoints;

   m_xs = new double [ m_numberOfPoints ];
   m_ys = new double [ m_numberOfPoints ];

   m_aCoeffs = new double [ m_numberOfPoints - 1 ];
   m_bCoeffs = new double [ m_numberOfPoints - 1 ];

   // Sort the array index pointer based on the order of the x-values.
   std::sort ( pointer, pointer + m_numberOfPoints, comp );

   // Copy the x- and y-values based on their order given by the index pointer.
   for ( i = 0; i < m_numberOfPoints; i++ ) {
      m_xs [ i ] = newXs [ pointer [ i ]];
      m_ys [ i ] = newYs [ pointer [ i ]];
   }

   delete [] pointer;
   computeCoefficients ();
}

//------------------------------------------------------------//

void ibs::PiecewiseInterpolator::computeCoefficients () {

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

int ibs::PiecewiseInterpolator::findPanel ( const double value ) const {

  for ( int i = 1; i < m_numberOfPoints; ++i ) {

     if ( value < m_xs [ i ]) {
        return i - 1;
     }

  }

  return m_numberOfPoints - 2;
}

//------------------------------------------------------------//

void ibs::PiecewiseInterpolator::print ( std::ostream& o ) const {

  int i;

  std::ios::fmtflags new_options = std::ios::scientific;
  std::ios::fmtflags old_options = o.flags ( new_options );

  int Old_Precision = o.precision ( 10 );

  o << " There are " << m_numberOfPoints << " points "  << std::endl;

  o << std::endl << " m_xs | m_ys " << std::endl;

  for ( i = 0; i < m_numberOfPoints; i++ ) {
    o << m_xs [ i ] << "  " << m_ys [ i ] << std::endl;
  }

  o << std::endl << " m_aCoeffs      |     m_bCoeffs  " << std::endl;

  for ( i = 0; i < m_numberOfPoints - 1; i++ ) {
     o << m_aCoeffs [ i ] << "  " << m_bCoeffs [ i ] << std::endl;
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

   buffer << std::endl << " m_aCoeffs      |     m_bCoeffs  " << std::endl;

   for ( I = 0; I < m_numberOfPoints - 1; I++ ) {
      buffer << m_aCoeffs [ I ] << "  " << m_bCoeffs [ I ] << std::endl;
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

  ///
  /// Should I compute the new coefficients before deleting the old stuff?
  ///
  setInterpolation ( newInterpolator.m_numberOfPoints, newInterpolator.m_xs, newInterpolator.m_ys );
  computeCoefficients ();

  return *this;
}
