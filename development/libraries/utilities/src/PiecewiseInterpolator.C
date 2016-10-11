//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "PiecewiseInterpolator.h"

#include <sstream>
#include <algorithm>

#include "AlignedMemoryAllocator.h"
#include "FormattingException.h"

#pragma warning(disable:3180)
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
     AlignedMemoryAllocator<double,ARRAY_ALIGNMENT>::free ( m_xs );
     AlignedMemoryAllocator<double,ARRAY_ALIGNMENT>::free ( m_ys );
     AlignedMemoryAllocator<double,ARRAY_ALIGNMENT>::free ( m_aCoeffs );
     AlignedMemoryAllocator<double,ARRAY_ALIGNMENT>::free ( m_bCoeffs );
  }

}

//------------------------------------------------------------//

double ibs::PiecewiseInterpolator::evaluate ( const double value ) const {
  unsigned int panelNumber = findPanel ( value );
  return m_aCoeffs [ panelNumber ] + m_bCoeffs [ panelNumber ] * value;
}

//------------------------------------------------------------//

void ibs::PiecewiseInterpolator::evaluate ( const unsigned int       size,
                                            ArrayDefs::ConstReal_ptr pnts,
                                            ArrayDefs::Real_ptr      values ) const {

   if ( m_numberOfPoints == 2 ) {
      // Only a single panel

#pragma omp simd aligned (pnts, values )
      for ( unsigned int i = 0; i < size; ++i ) {
         values [ i ] = m_aCoeffs [ 0 ] + m_bCoeffs [ 0 ] * pnts [ i ];
      }

   } else {

#pragma omp simd aligned (pnts, values)
      for ( unsigned int i = 0; i < size; ++i ) {
         values [ i ] = evaluate ( pnts [ i ]);
      }

   }

}

//------------------------------------------------------------//

double ibs::PiecewiseInterpolator::evaluateDerivative ( const double value ) const {
   unsigned int panelNumber = findPanel ( value );
   return m_bCoeffs [ panelNumber ];
}

//------------------------------------------------------------//

void ibs::PiecewiseInterpolator::evaluateDerivative ( const unsigned int       size,
                                                      ArrayDefs::ConstReal_ptr pnts,
                                                      ArrayDefs::Real_ptr      values ) const {

   if ( m_numberOfPoints == 2 ) {
      // Only a single panel

#pragma omp simd aligned (pnts, values)
      for ( unsigned int i = 0; i < size; ++i ) {
         values [ i ] = m_bCoeffs [ 0 ];
      }

   } else {

#pragma omp simd aligned (pnts, values)
      for ( unsigned int i = 0; i < size; ++i ) {
         values [ i ] = evaluateDerivative ( pnts [ i ]);
      }

   }

}

//------------------------------------------------------------//

void ibs::PiecewiseInterpolator::evaluate ( const double x,
                                            double&      value,
                                            double&      derivative ) const {

   unsigned int panelNumber = findPanel ( x );

   value = m_aCoeffs [ panelNumber ] + m_bCoeffs [ panelNumber ] * x;
   derivative = m_bCoeffs [ panelNumber ];
}

//------------------------------------------------------------//

void ibs::PiecewiseInterpolator::evaluate (  const unsigned int       size,
                                             ArrayDefs::ConstReal_ptr pnts,
                                             ArrayDefs::Real_ptr      values,
                                             ArrayDefs::Real_ptr      derivatives ) const {

   if ( m_numberOfPoints == 2 ) {
      // Only a single panel

#pragma omp simd aligned (pnts, values, derivatives)
      for ( unsigned int i = 0; i < size; ++i ) {
         values [ i ] = m_aCoeffs [ 0 ] + m_bCoeffs [ 0 ] * pnts [ i ];
         derivatives [ i ] = m_bCoeffs [ 0 ];
      }

   } else {

#pragma omp simd aligned (pnts, values, derivatives)
      for ( unsigned int i = 0; i < size; ++i ) {
         evaluate ( pnts [ i ], values [ i ], derivatives [ i ]);
      }

   }

}

//------------------------------------------------------------//

void ibs::PiecewiseInterpolator::setInterpolation ( const unsigned int        newNumberOfPoints,
                                                    const double*             newXs,
                                                    const double*             newYs ) {

   if ( newNumberOfPoints <= 1 ) {
      throw formattingexception::GeneralException () << "Incorrect number of points "
                                                     << newNumberOfPoints
                                                     << ", must be greater than 1";
   }

   if ( newXs == nullptr or newYs == nullptr ) {
      throw formattingexception::GeneralException () << "One or both of the data arrays are null.";
   }

   unsigned int* pointer = new unsigned int [ newNumberOfPoints ];

   PointerSort comp ( newXs );

   // Initialise the array index pointer.
   for ( unsigned int i = 0; i < newNumberOfPoints; ++i ) {
      pointer [ i ] = i;
   }

   if ( m_numberOfPoints != 0 ) {
      deleteCoefficients ();
   }

   m_numberOfPoints = newNumberOfPoints;

   m_xs = AlignedMemoryAllocator<double,ARRAY_ALIGNMENT>::allocate ( m_numberOfPoints );
   m_ys = AlignedMemoryAllocator<double,ARRAY_ALIGNMENT>::allocate ( m_numberOfPoints );

   m_aCoeffs = AlignedMemoryAllocator<double,ARRAY_ALIGNMENT>::allocate ( m_numberOfPoints - 1 );
   m_bCoeffs = AlignedMemoryAllocator<double,ARRAY_ALIGNMENT>::allocate ( m_numberOfPoints - 1 );

   // Sort the array index pointer based on the order of the x-values.
   std::sort ( pointer, pointer + m_numberOfPoints, comp );

   // Copy the x- and y-values based on their order given by the index pointer.
   for ( unsigned int i = 0; i < m_numberOfPoints; i++ ) {
      m_xs [ i ] = newXs [ pointer [ i ]];
      m_ys [ i ] = newYs [ pointer [ i ]];
   }

   delete [] pointer;
   computeCoefficients ();
}

//------------------------------------------------------------//

void ibs::PiecewiseInterpolator::computeCoefficients () {

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
  for ( unsigned int i = 0; i < m_numberOfPoints - 1; i++ ) {
    divisor = 1.0 / ( m_xs [ i + 1 ] - m_xs [ i ]);

    m_aCoeffs [ i ] = divisor * ( m_xs [ i + 1 ] * m_ys [ i ] -  m_xs [ i ] * m_ys [ i + 1 ]);
    m_bCoeffs [ i ] = divisor * ( m_ys [ i + 1 ] - m_ys [ i ]);
  }

}

//------------------------------------------------------------//

unsigned int ibs::PiecewiseInterpolator::findPanel ( const double value ) const {

  for ( unsigned int i = 1; i < m_numberOfPoints; ++i ) {

     if ( value < m_xs [ i ]) {
        return i - 1;
     }

  }

  return m_numberOfPoints - 2;
}

//------------------------------------------------------------//

void ibs::PiecewiseInterpolator::print ( std::ostream& o ) const {

  std::ios::fmtflags new_options = std::ios::scientific;
  std::ios::fmtflags old_options = o.flags ( new_options );

  int Old_Precision = o.precision ( 10 );

  o << " permeability interpolator: " << std::endl << " There are " << m_numberOfPoints << " points "  << std::endl;
  o << std::endl << " m_xs | m_ys " << std::endl;

  for ( unsigned int i = 0; i < m_numberOfPoints; i++ ) {
    o << m_xs [ i ] << "  " << m_ys [ i ] << std::endl;
  }

  o << std::endl << " m_aCoeffs      |     m_bCoeffs  " << std::endl;

  for ( unsigned int i = 0; i < m_numberOfPoints - 1; i++ ) {
     o << m_aCoeffs [ i ] << "  " << m_bCoeffs [ i ] << std::endl;
  }

  o << std::endl;
  o.precision ( Old_Precision );
  o.flags ( old_options );

}

//------------------------------------------------------------//

std::string ibs::PiecewiseInterpolator::image () const {
   std::stringstream buffer;
   print ( buffer );
   return buffer.str ();
}

//------------------------------------------------------------//

ibs::PiecewiseInterpolator& ibs::PiecewiseInterpolator::operator=( const PiecewiseInterpolator& newInterpolator ) {
  setInterpolation ( newInterpolator.m_numberOfPoints, newInterpolator.m_xs, newInterpolator.m_ys );
  return *this;
}
