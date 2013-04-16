#include "IntervalInterpolator.h"

const Numerics::FloatingPoint Numerics::IntervalInterpolator::SurfaceConstraint  = 0.0;
const Numerics::FloatingPoint Numerics::IntervalInterpolator::ExteriorConstraint = -10.0;
const Numerics::FloatingPoint Numerics::IntervalInterpolator::InteriorConstraint =  10.0;

//------------------------------------------------------------//

Numerics::IntervalInterpolator::IntervalInterpolator () {
  translation ( 0 ) = 0.0;
  translation ( 1 ) = 0.0;
  translation ( 2 ) = 0.0;

  scaling ( 0 ) = 1.0;
  scaling ( 1 ) = 1.0;
  scaling ( 2 ) = 1.0;
}

//------------------------------------------------------------//

void Numerics::IntervalInterpolator::setPolynomialDegree ( const int polynomialDegree ) {
  interpolator.setPolynomialDegree ( polynomialDegree );
}

//------------------------------------------------------------//

void Numerics::IntervalInterpolator::setTransformation ( const GeometryVector& newTranslation,
                                                         const GeometryVector& newScaling ) {

  translation = newTranslation;
  scaling     = newScaling;
}

//------------------------------------------------------------//

void Numerics::IntervalInterpolator::setIntervalBounds ( const FloatingPoint newStartAge,
                                                         const FloatingPoint newEndAge ) {

  startAge = newStartAge;
  endAge = newEndAge;
}

//------------------------------------------------------------//

void Numerics::IntervalInterpolator::setInterpolationPoints ( const PointArray& interpolationPoints ) {
  interpolator.setInterpolationPoints ( interpolationPoints );
}

//------------------------------------------------------------//

void Numerics::IntervalInterpolator::setInterpolationCoefficients ( const Vector& interpolationCoefficients ) {
  interpolator.setCoefficients ( interpolationCoefficients );
}

//------------------------------------------------------------//

Numerics::FloatingPoint Numerics::IntervalInterpolator::operator ()( const Point& evaluationPoint ) const {

  // Evaluate the interpolant.
  // First transform the point to the interpolation co-ordinate space.
  return interpolator ( scaling * ( evaluationPoint - translation ));
}

//------------------------------------------------------------//

const Numerics::IntervalInterpolator::RBFInterpolator& Numerics::IntervalInterpolator::getInterpolator () const {
  return interpolator;
}

//------------------------------------------------------------//

int Numerics::IntervalInterpolator::getPolynomialDegree () const {
  return interpolator.getPolynomialDegree ();
}

//------------------------------------------------------------//

const Numerics::GeometryVector& Numerics::IntervalInterpolator::getScaling () const {
  return scaling;
} 

//------------------------------------------------------------//

const Numerics::GeometryVector& Numerics::IntervalInterpolator::getTranslation () const {
  return translation;
}

//------------------------------------------------------------//

Numerics::FloatingPoint Numerics::IntervalInterpolator::getStartAge () const {
  return startAge;
}

//------------------------------------------------------------//

Numerics::FloatingPoint Numerics::IntervalInterpolator::getEndAge () const {
  return endAge;
}

//------------------------------------------------------------//
