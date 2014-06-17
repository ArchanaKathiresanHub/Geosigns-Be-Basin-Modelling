#ifndef FASTCAULDRON__VECTOR_INTERPOLATOR_H
#define FASTCAULDRON__VECTOR_INTERPOLATOR_H

#include <tr1/array>
#include <iostream>
#include <iomanip>

#include "Numerics.h"
#include "Vector.h"
#include "FortranMatrix.h"
#include "FortranMatrixOperations.h"


/// \brief An multiplie value interpolator.
///
/// The interpolators are arranged in a one dimensional array.
/// The degree of the interpolating polynomial is fixed.
template<const unsigned int Size>
class VectorInterpolator {

public :


   /// \brief The degree of the interpolating polynomial.
   static const unsigned int Degree = 2;

   /// \brief The number of coefficients in the interpolating polynomial.
   static const unsigned int NumberOfCoefficients = Degree + 1;

   /// \brief The number of equations the interpolator is to interpolate.
   static const unsigned int NumberOfEquations = Size;

   /// \brief Array of length NumberOfCoefficients.
   typedef std::tr1::array<double, NumberOfCoefficients> CoefficientArray;

   /// \brief Array of length NumberOfCoefficients.
   typedef std::tr1::array<CoefficientArray, NumberOfEquations> CoefficientMatrix;


   /// \brief Compute the coefficients of the interpolating polynomial.
   void compute ( const CoefficientArray&  xs,
                  const CoefficientMatrix& ys );

   /// \brief Evaluate the interpolatant at the value x for an equation.
   ///
   /// The value x should be within the same range as was used to compute the interpolant.
   double evaluate ( const unsigned int equation,
                     const double       x ) const;

   /// \brief Prints the coefficients of one of the polynomials.
   void print ( const unsigned int equation ) const;

private :

   /// \brief The coefficients of the interpolating polynomial.
   ///
   /// The zeroth order coefficient will be in position zero, then increasing in order
   /// until the p'th order coefficient which will be in position p.
   /// \f$  p_e(x) = \sum_=0^p a_{e,i} x^i \f$ for each equation e.
   CoefficientMatrix m_coefficients;

};

template<const unsigned int Size>
const unsigned int VectorInterpolator<Size> :: Degree ;

template<const unsigned int Size>
inline double VectorInterpolator<Size>::evaluate ( const unsigned int equation,
                                                   const double       x ) const {

// #ifdef DEBUG
//    if ( Degree != 2 ) {
//       // Error
//    }
// #endif

   return ( m_coefficients [ equation ][ 2 ] * x + m_coefficients [ equation ][ 1 ]) * x + m_coefficients [ equation ][ 0 ];
}

template<const unsigned int Size>
void VectorInterpolator<Size>::compute ( const CoefficientArray& xs,
                                         const CoefficientMatrix& ys ) {

// #ifdef DEBUG
//    if ( Degree != 2 ) {
//       // Error
//    }
// #endif

#if 0
   Numerics::FortranMatrix mat ( NumberOfCoefficients, NumberOfCoefficients );
   Numerics::IntegerArray  permutation ( NumberOfCoefficients );
#endif
   Numerics::FortranMatrix inverse ( NumberOfCoefficients, NumberOfCoefficients );
   Numerics::Vector        vec ( NumberOfCoefficients );
   Numerics::Vector        res ( NumberOfCoefficients );
   unsigned int i;

#if 0
   // Set the interpolation matrix.
   mat ( 0, 0 ) = 1.0;
   mat ( 0, 1 ) = xs [ 0 ];
   mat ( 0, 2 ) = xs [ 0 ] * xs [ 0 ];

   mat ( 1, 0 ) = 1.0;
   mat ( 1, 1 ) = xs [ 1 ];
   mat ( 1, 2 ) = xs [ 1 ] * xs [ 1 ];

   mat ( 2, 0 ) = 1.0;
   mat ( 2, 1 ) = xs [ 2 ];
   mat ( 2, 2 ) = xs [ 2 ] * xs [ 2 ];
#endif

   // Set inverse of the interpolation matrix.
   // The values here are for xs={0,0.5,1}.
   // For a degree = 2 polynomial the x values will always be {0, 0.5, 1}
   //
   // The reason for using the invere rather than compute it (or the LU) is to 
   // reduce rounding errors that can cause negative concentrations in the Darcy.
   inverse ( 0, 0 ) = 1.0;
   inverse ( 0, 1 ) = 0.0;
   inverse ( 0, 2 ) = 0.0;

   inverse ( 1, 0 ) = -3.0;
   inverse ( 1, 1 ) =  4.0;
   inverse ( 1, 2 ) = -1.0;

   inverse ( 2, 0 ) =  2.0;
   inverse ( 2, 1 ) = -4.0;
   inverse ( 2, 2 ) =  2.0;

#if 0
   // Solve the system of linear eqations.
   Numerics::luFactorise ( mat, permutation );
#endif

   for ( i = 0; i < NumberOfEquations; ++i ) {
      // Set the right hand side vector.
      vec ( 0 ) = ys [ i ][ 0 ];
      vec ( 1 ) = ys [ i ][ 1 ];
      vec ( 2 ) = ys [ i ][ 2 ];

#if 0
      Numerics::backSolve ( mat, permutation, vec );
#endif
      Numerics::mvp ( inverse, vec, res );


      // Copy back the coefficients.
      m_coefficients [ i ][ 0 ] = res ( 0 );
      m_coefficients [ i ][ 1 ] = res ( 1 );
      m_coefficients [ i ][ 2 ] = res ( 2 );

#if 0
      m_coefficients [ i ][ 0 ] = vec ( 0 );
      m_coefficients [ i ][ 1 ] = vec ( 1 );
      m_coefficients [ i ][ 2 ] = vec ( 2 );
#endif

   }


}


template<const unsigned int Size>
void VectorInterpolator<Size>::print ( const unsigned int equation ) const {
   std::cout << " coefficients " 
        << std::setw ( 20 ) << m_coefficients [ equation ][ 2 ] << "  "
        << std::setw ( 20 ) << m_coefficients [ equation ][ 1 ] << "  "
        << std::setw ( 20 ) << m_coefficients [ equation ][ 0 ]
        << std::endl;
}


#endif // FASTCAULDRON__VECTOR_INTERPOLATOR_H
