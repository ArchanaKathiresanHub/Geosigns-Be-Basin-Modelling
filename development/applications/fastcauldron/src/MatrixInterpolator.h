#ifndef FASTCAULDRON__MATRIX_INTERPOLATOR_H
#define FASTCAULDRON__MATRIX_INTERPOLATOR_H

#include <tr1/array>

#include "Numerics.h"
#include "Vector.h"
#include "FortranMatrix.h"
#include "FortranMatrixOperations.h"


/// \brief An multiplie value interpolator.
///
/// The interpolators are arranged in a two dimensional array.
/// The degree of the interpolating polynomial is fixed.
template<const unsigned int RowCount,
         const unsigned int ColCount>
class MatrixInterpolator {

public :


   /// \brief The degree of the interpolating polynomial.
   static const unsigned int Degree = 2;

   /// \brief The number of coefficients in the interpolating polynomial.
   static const unsigned int NumberOfCoefficients = Degree + 1;

   /// \brief The number of equations the interpolator is to interpolate.
   static const unsigned int NumberOfEquations = RowCount * ColCount;

   /// \brief Array of length NumberOfCoefficients.
   typedef std::tr1::array<double, NumberOfCoefficients> CoefficientArray;

   /// \brief Array of length ColCount.
   typedef std::tr1::array<CoefficientArray, ColCount> ColumnMatrix;

   /// \brief Array of length RowCount.
   typedef std::tr1::array<ColumnMatrix, RowCount> CoefficientTensor;


   /// \brief Compute the coefficients of the interpolating polynomial.
   void compute ( const CoefficientArray&  xs,
                  const CoefficientTensor& ys );

   /// \brief Evaluate the interpolatant at the value x for an equation.
   ///
   /// The value x should be within the same range as was used to compute the interpolant.
   double evaluate ( const unsigned int row,
                     const unsigned int col,
                     const double       x ) const;

private :

   /// \brief The coefficients of the interpolating polynomial.
   ///
   /// The zeroth order coefficient will be in position zero, then increasing in order
   /// until the p'th order coefficient which will be in position p.
   /// \f$  p_e(x) = \sum_=0^p a_{e,i} x^i \f$ for each equation e.
   CoefficientTensor m_coefficients;

};

template <const unsigned int RowCount, const unsigned int ColCount>
const unsigned int MatrixInterpolator<RowCount, ColCount>::Degree;


template<const unsigned int RowCount,
         const unsigned int ColCount>
inline double MatrixInterpolator<RowCount, ColCount>::evaluate ( const unsigned int row,
                                                                 const unsigned int col,
                                                                 const double       x ) const {

// #ifdef DEBUG
//    if ( Degree != 2 ) {
//       // Error
//    }
// #endif

   return ( m_coefficients [ row ][ col ][ 2 ] * x + m_coefficients [ row ][ col ][ 1 ]) * x + m_coefficients [ row ][ col ][ 0 ];
}

template<const unsigned int RowCount,
         const unsigned int ColCount>
void MatrixInterpolator<RowCount, ColCount>::compute ( const CoefficientArray& xs,
                                                       const CoefficientTensor& ys ) {

// #ifdef DEBUG
//    if ( Degree != 2 ) {
//       // Error
//    }
// #endif

   Numerics::FortranMatrix mat ( NumberOfCoefficients, NumberOfCoefficients );
   Numerics::Vector        vec ( NumberOfCoefficients );
   Numerics::IntegerArray  permutation ( NumberOfCoefficients );
   unsigned int i;
   unsigned int j;

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

   // Solve the system of linear eqations.
   Numerics::luFactorise ( mat, permutation );

   for ( i = 0; i < RowCount; ++i ) {

      for ( j = 0; j < ColCount; ++j ) {
         // Set the right hand side matrix.
         vec ( 0 ) = ys [ i ][ j ][ 0 ];
         vec ( 1 ) = ys [ i ][ j ][ 1 ];
         vec ( 2 ) = ys [ i ][ j ][ 2 ];
         Numerics::backSolve ( mat, permutation, vec );

         // Copy back the coefficients.
         m_coefficients [ i ][ j ][ 0 ] = vec ( 0 );
         m_coefficients [ i ][ j ][ 1 ] = vec ( 1 );
         m_coefficients [ i ][ j ][ 2 ] = vec ( 2 );
      }

   }

}

#endif // FASTCAULDRON__MATRIX_INTERPOLATOR_H
