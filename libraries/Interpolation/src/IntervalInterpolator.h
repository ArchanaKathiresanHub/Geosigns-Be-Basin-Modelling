#ifndef __Numerics_IntervalInterpolator_H__
#define __Numerics_IntervalInterpolator_H__

// Numerics
#include "Numerics.h"
#include "Vector.h"
#include "GeometryVector.h"
#include "Point.h"

// Interpolation stuff
#include "RBFInterpolant.h"
#include "RBFCubic.h"


namespace Numerics {

  /** @addtogroup Interpolation
   *
   * @{
   */


  /// \brief Interpolates between two input distribution maps.
  ///
  /// An implicit surface representation of the surface of the allochthonous body 
  /// is computed from the surface, exterior and interior points of this body.
  /// The implicit surface is computed using a radial basis function interpolation scheme.
  /// These points are gathered from the pair of input distribution-maps.
  /// These maps must be adjacent input maps, in terms of age, in the input table.
  class IntervalInterpolator {

  public :

    typedef RBFInterpolant<RBFCubic> RBFInterpolator;


    /// \name Interpolator constraint values.
    /// @{

    /// \var SurfaceConstraint
    /// \brief The value assigned to the constraints of the interpolation equations for points that
    /// lie on the surface of the allochthonous body.
    static const FloatingPoint SurfaceConstraint;

    /// \var ExteriorConstraint
    /// \brief The value assigned to the constraints of the interpolation equations for points that
    /// lie outside the surface of the allochthonous body.
    static const FloatingPoint ExteriorConstraint;

    /// \var InteriorConstraint
    /// \brief The value assigned to the constraints of the interpolation equations for points that
    /// lie inside the surface of the allochthonous body.
    static const FloatingPoint InteriorConstraint;

    /// @}

    IntervalInterpolator ();

    virtual ~IntervalInterpolator () {}

    /// \name Post-construction set-up functions
    /// @{

    /// \brief Assign the polynomial degree to be used by the interpolator.
    void setPolynomialDegree ( const int polynomialDegree );

    /// \brief Set the transformation vectors for the any point in the interpolant.
    void setTransformation ( const GeometryVector& newTranslation,
                             const GeometryVector& newScaling );

    /// \brief Set the age bounds of the interval.
    void setIntervalBounds ( const FloatingPoint newLowerAge,
                             const FloatingPoint newUpperAge );

    /// \brief Set the interpolation points.
    void setInterpolationPoints ( const PointArray& interpolationPoints );

    /// \brief Set the interpolation coefficients.
    void setInterpolationCoefficients ( const Vector& interpolationCoefficients );
    /// @}

    /// \name Attribute access functions.
    /// @{

    /// \brief The older age of time interval.
    FloatingPoint getStartAge () const;

    /// \brief The younger age of time interval.
    FloatingPoint getEndAge () const;

    /// \brief Return the translation part of the inteprolation-point transformation.
    const GeometryVector& getTranslation () const;

    /// \brief Return the scaling part of the inteprolation-point transformation.
    const GeometryVector& getScaling () const;

    /// Return the interpolator used.
    const RBFInterpolator& getInterpolator () const;

    /// \brief Returh the degree of the polynomial used by the interpolator.
    int getPolynomialDegree () const;

    /// @}

    /// \brief Evaluate the interpolant at the evaluation point.
    ///
    /// The evaluation points should lie within the time interval for which the interpolant was computed.
    FloatingPoint operator ()( const Point& evaluationPoint ) const;

  protected :

    /// \brief The allochthonous body interpolator.
    RBFInterpolator interpolator;

  private :

    /// \brief How much to translate the interpolation or evaluation points, \f$  \hat{p} = s\cdot ( p - t )\f$
    ///
    /// When computing or evaluating the interpolant, all points are transformed so that they lie within a bounding box.
    GeometryVector translation;

    /// \brief How much to scale the interpolation or evaluation points, , \f$  \hat{p} = s\cdot ( p - t )\f$
    ///
    /// When computing or evaluating the interpolant, all points are transformed so that they lie within a bounding box.
    GeometryVector scaling;

    /// \brief The age of the older of the two maps.
    FloatingPoint startAge;

    /// \brief The age of the younger of the two maps.
    FloatingPoint endAge;

  };

  /** @} */

}

#endif // __Numerics_IntervalInterpolator_H__
