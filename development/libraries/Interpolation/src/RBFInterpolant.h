#ifndef __RBFInterpolant_HH__
#define __RBFInterpolant_HH__

#include "Vector.h"
#include "Point.h"
#include "RBFGeneralOperations.h"

/*! \defgroup Interpolation Interpolation
 * \ingroup Numerics
 *
 */

namespace Numerics {


  /** @addtogroup Interpolation
   *
   * @{
   */

  /// \file RBFInterpolant.h
  /// \brief Adds the radial basis function interpolant.

  /// \brief The radial basis function interpolant type.
  template <class RadialBasisFunction >
  class RBFInterpolant {

  public :

    static const int DefaultPolynomialDegree = Numerics::NoPolynomial;

    /// \name Constructors
    /// @{

    /// \brief Construct, the polynomial degree is initialised to the default degree
    RBFInterpolant ();

    RBFInterpolant ( const int                 initialPolynomialDegree );

    RBFInterpolant ( const RadialBasisFunction newRBF,
                     const int                 initialPolynomialDegree );

    RBFInterpolant ( const RadialBasisFunction newRBF,
                     const PointArray&         newInterpolationPoints,
                     const int                 initialPolynomialDegree );

    /// @}

    /// \brief Set the degree of the polynomial that extends the radial basis function interpolant.
    void setPolynomialDegree ( const int newPolynomialDegree );

    /// \brief Set the interpolation points.
    /// 
    /// A deep copy of the interpolation-points is made here.
    void setInterpolationPoints ( const PointArray& newPoints );

    /// \brief Set the coefficients of the interpolant.
    void setCoefficients ( const Vector& newCoefficients );

    /// \brief Evaluate the interpolant at the specified point.
    FloatingPoint operator ()( const Point& evaluationPoint ) const;


    /// \brief Get the points that are in the interpolation point set.
    const PointArray& getInterpolationPoints () const;

    /// \brief Get the coefficients of the basis functions.
    const Vector& getCoefficients () const;

    /// \brief Get the degree of the polynomial.
    int getPolynomialDegree () const;

    /// \brief Get the radial basis function used.
    const RadialBasisFunction& getRadialBasisFunction () const;

  protected :

    /// \brief The point set used for the interpolation.
    PointArray  interpolationPoints;

    /// \brief Coefficients of basis functions and polynomlai part of interpolation.
    ///
    /// Length of this vector is the number of interpolation points and 
    /// the number of terms in the polynomial.
    Vector              coefficients;

    /// \brief A work space vector.
    /// Rather than creating a vector each time the interpolant is
    /// evaluated this work space is used.
    mutable Vector      workSpace;

    /// \brief An instance of the radial basis function template parameter.
    RadialBasisFunction rbf;

    /// \brief The degree of the polynomial term.
    ///
    /// \warning This is not the number of terms in the polynomial.
    int                 polynomialDegree;

  }; 

  /** @} */

}

//------------------------------------------------------------//

template <class RadialBasisFunction>
Numerics::RBFInterpolant<RadialBasisFunction>::RBFInterpolant () {
  polynomialDegree = DefaultPolynomialDegree;
}

//------------------------------------------------------------//

template <class RadialBasisFunction>
Numerics::RBFInterpolant<RadialBasisFunction>::RBFInterpolant ( const int initialPolynomialDegree ) {
  polynomialDegree = initialPolynomialDegree;
}

//------------------------------------------------------------//

template <class RadialBasisFunction>
Numerics::RBFInterpolant<RadialBasisFunction>::RBFInterpolant ( const RadialBasisFunction newRBF,
                                                                const int                 initialPolynomialDegree ) {
  rbf = newRBF;
  polynomialDegree = initialPolynomialDegree;
}

//------------------------------------------------------------//

template <class RadialBasisFunction>
Numerics::RBFInterpolant<RadialBasisFunction>::RBFInterpolant 
   ( const RadialBasisFunction newRBF,
     const PointArray&         newInterpolationPoints,
     const int                 initialPolynomialDegree ) :

     interpolationPoints ( newInterpolationPoints ),
     coefficients ( newInterpolationPoints.size () + numberOfPolynomialTerms<Point::DIMENSION>( initialPolynomialDegree )),
     workSpace    ( newInterpolationPoints.size () + numberOfPolynomialTerms<Point::DIMENSION>( initialPolynomialDegree )) {


  polynomialDegree = initialPolynomialDegree;
  rbf = newRBF;
}

//------------------------------------------------------------//

template <class RadialBasisFunction>
Numerics::FloatingPoint Numerics::RBFInterpolant<RadialBasisFunction>::operator ()( const Point& evaluationPoint ) const {

  evaluateBasisFunction<RadialBasisFunction> ( rbf,
                                               interpolationPoints,
                                               polynomialDegree,
                                               evaluationPoint,
                                               workSpace );

  return innerProduct ( coefficients, workSpace );
}

//------------------------------------------------------------//

template <class RadialBasisFunction>
const Numerics::PointArray& Numerics::RBFInterpolant<RadialBasisFunction>::getInterpolationPoints () const {
  return interpolationPoints;
}

//------------------------------------------------------------//

template <class RadialBasisFunction>
const Numerics::Vector& Numerics::RBFInterpolant<RadialBasisFunction>::getCoefficients () const {
  return coefficients;
}

//------------------------------------------------------------//

template <class RadialBasisFunction>
void Numerics::RBFInterpolant<RadialBasisFunction>::setInterpolationPoints ( const PointArray& newPoints ) {
  interpolationPoints = newPoints;
  workSpace.resize ( interpolationPoints.size () + numberOfPolynomialTerms<Point::DIMENSION>( polynomialDegree ));
}

//------------------------------------------------------------//

template <class RadialBasisFunction>
void Numerics::RBFInterpolant<RadialBasisFunction>::setPolynomialDegree ( const int newPolynomialDegree ) {
  polynomialDegree = newPolynomialDegree;
}

//------------------------------------------------------------//

template <class RadialBasisFunction>
void Numerics::RBFInterpolant<RadialBasisFunction>::setCoefficients ( const Vector& newCoefficients ) {

  if ( coefficients.dimension () != newCoefficients.dimension ()) {
    coefficients.resize ( newCoefficients.dimension ());
  }

  coefficients = newCoefficients;
}

//------------------------------------------------------------//

template <class RadialBasisFunction>
const RadialBasisFunction& Numerics::RBFInterpolant<RadialBasisFunction>::getRadialBasisFunction () const {
  return rbf;
}

//------------------------------------------------------------//

template <class RadialBasisFunction>
int Numerics::RBFInterpolant<RadialBasisFunction>::getPolynomialDegree () const {
  return polynomialDegree;
}

//------------------------------------------------------------//

#endif // __RBFInterpolant_HH__
