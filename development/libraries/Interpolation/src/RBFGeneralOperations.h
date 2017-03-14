#ifndef __RBFGeneralOperations_HH__
#define __RBFGeneralOperations_HH__

#include <cstdlib>
#include <stdlib.h>

#include "DenseMatrix.h"
#include "Vector.h"
#include "Point.h"

namespace Numerics {


  /** @addtogroup Interpolation
   *
   * @{
   */

  /// \file RBFGeneralOperations.h
  /// \brief Defines several useful operations for radial basis function interpolation.

  /// \var NoPolynomial
  /// \brief Indicates that during the rbf interpolation no polynomial should be used.
  ///
  /// These have not been implemented as an enumeration because the enumeration would then be
  /// of a fixed set of values, this way the degree can be extended to any value.
  const int NoPolynomial        = -1;

  /// \var ConstantPolynomial
  /// \brief Indicates that during the rbf interpolation a degree 0 polynomial should be used.
  const int ConstantPolynomial  =  0;

  /// \var LinearPolynomial
  /// \brief Indicates that during the rbf interpolation a linear polynomial should be used.
  const int LinearPolynomial    =  1;

  /// \var QuadraticPolynomial
  /// \brief Indicates that during the rbf interpolation a quadratic polynomial should be used.
  const int QuadraticPolynomial =  2;

  /// \brief The number of terms that make up the polynomial of degree Degree.
  /// Returns the number of terms in the truncated polynomial space.
  // This function, and the polynomial part of the RBF interpolant, needs to be re-thought.
  // Should I include the full polynomial space or the trunc-space only? I have tried to include
  // only the trunc-space, but the full space would be easier to develop and would only add a 
  // few extra entries to the interpolation-space as a whole.
  template<const int Dimension>
  int numberOfPolynomialTerms ( const int degree );

  /// \brief Evaluate the radial basis function rbf, at all interpolation points.
  template <  class RadialBasisFunction>
  void assembleInterpolationMatrix ( const RadialBasisFunction& rbf,
                                     const PointArray&          interpolationPoints,
                                     const int                  polynomialDegree,
                                           DenseMatrix&         interpolationMatrix );


  /// \brief Computes the polynomial part of the interpolation matrix.
  void assembleInterpolationMatrixPolynomialPart ( const PointArray&  interpolationPoints,
                                                   const int          polynomialDegree,
                                                         DenseMatrix& interpolationMatrix );

  /// \brief Computes the polynomial terms used in the interpolation matrix or bases.
  void assemblePolynomialPart ( const Point&  evaluationPoint, 
                                const int     polynomialDegree,
                                      Vector& polynomialPart );

  /// \brief Evaluate the radial basis function rbf, at all interpolation points.
  template < class RadialBasisFunction>
  void evaluateBasisFunction ( const RadialBasisFunction& rbf,
                               const PointArray&          interpolationPoints,
                               const int                  polynomialDegree,
                               const Point&               evaluationPoint,
                                     Vector&              bases );

  /// \brief Fill vector with separation distance evaluation point from the interpolation point set.
  void computeRsSquared ( const Point&      evaluationPoint, 
                          const PointArray& interpolationPoints,
                                Vector&     rsSquared );


  /// \brief Fill the interpolation point array with a set of random points.
  ///
  /// Of little use outside of testing.
  template <typename RandomGenerator>
  void randomInterpolationPoints ( PointArray&      interpolationPoints,
                                   RandomGenerator& rand );


  /** @} */


}

//------------------------------------------------------------//

// Needs some attention!
template<const int Dimension>
int Numerics::numberOfPolynomialTerms ( const int degree ) {

  static_assert(Dimension > 0, "Dimension must be positive");
  if ( degree <= NoPolynomial ) {
    return 0;
  } else if ( degree == ConstantPolynomial ) {
    return 1;
  }

  int result;

  switch ( Dimension ) {

    case 1 : result = degree + 1;
             break;

    case 2 : 

      if ( degree == LinearPolynomial ) {
        result = 3;
      } else if ( degree == QuadraticPolynomial ) {
        result = 6;
      } else if ( degree == 3 ) {
        result = 10;
      } else {
        result = 0;
      }

      break;

    case 3 : 

      if ( degree == LinearPolynomial ) {
        result = 4;
      } else if ( degree == QuadraticPolynomial ) {
        result = 10;
      } else {
        result = 0;
      }

      break;


    default : return 0;

  }

  return result;
}

//------------------------------------------------------------//

template <class RadialBasisFunction>
void Numerics::assembleInterpolationMatrix ( const RadialBasisFunction& rbf,
                                             const PointArray&          interpolationPoints,
                                             const int                  polynomialDegree,
                                                   DenseMatrix&         interpolationMatrix ) {

  const int PolynomialTerms = numberOfPolynomialTerms< Point::DIMENSION >( polynomialDegree );
  const int NumberOfPoints  = interpolationPoints.size ();

  FloatingPoint rSquared;
  int i;
  int j;

  // Compute basis function part of matrix.
  if ( interpolationMatrix.isRowMajorOrder ()) {

    for ( i = 0; i < NumberOfPoints; i++ ) {

      for ( j = 0; j < NumberOfPoints; j++ ) {
        rSquared = separationDistanceSquared ( interpolationPoints [ i ], interpolationPoints [ j ]);
        interpolationMatrix ( i, j ) = rbf ( rSquared );
      }

    }

  } else {

    for ( i = 0; i < NumberOfPoints; i++ ) {

      for ( j = 0; j < NumberOfPoints; j++ ) {
        rSquared = separationDistanceSquared ( interpolationPoints [ i ], interpolationPoints [ j ]);
        interpolationMatrix ( j, i ) = rbf ( rSquared );
      }

    }

  }

  // Compute the polynomial part of the matrix.
  assembleInterpolationMatrixPolynomialPart ( interpolationPoints, polynomialDegree, interpolationMatrix );

  // And finally the zero block, bottom right corner block
  for ( i = NumberOfPoints; i < NumberOfPoints + PolynomialTerms; i++ ) {

    for ( j = NumberOfPoints; j < NumberOfPoints + PolynomialTerms; j++ ) {
      interpolationMatrix ( i, j ) = 0.0;
    }

  }

}

//------------------------------------------------------------//

template <class RadialBasisFunction>
void Numerics::evaluateBasisFunction ( const RadialBasisFunction& rbf,
                                       const PointArray&          interpolationPoints,
                                       const int                  polynomialDegree,
                                       const Point&               evaluationPoint,
                                             Vector&              bases ) {

  int i;
  int j;
  FloatingPoint rSquared;

  for ( i = 0; i < (int)interpolationPoints.size (); i++ ) {
    rSquared = separationDistanceSquared ( evaluationPoint, interpolationPoints [ i ]);
    bases ( i ) = rbf ( rSquared );
  }

  if ( polynomialDegree >= ConstantPolynomial ) {
    Vector polynomialPart ( numberOfPolynomialTerms< Point::DIMENSION >( polynomialDegree ));

    // Need to use the basis vector and NOT create a new one (polynomialPart).
    assemblePolynomialPart ( evaluationPoint, polynomialDegree, polynomialPart );

    for ( j = 0; j < polynomialPart.dimension (); j++ ) {
      bases ( int ( interpolationPoints.size ()) + j ) = polynomialPart ( j );
    }

  }

}

//------------------------------------------------------------//

template <typename RandomGenerator>
void Numerics::randomInterpolationPoints ( PointArray&      interpolationPoints,
                                           RandomGenerator& rand ) {

  size_t i;
  int j;

  for ( i = 0; i < interpolationPoints.size (); ++i ) {

    for ( j = 0; j < Point::DIMENSION; ++j ) {
      interpolationPoints [ i ]( j ) = rand ();
    }

  }

}

//------------------------------------------------------------//

#endif // __RBFGeneralOperations_HH__

