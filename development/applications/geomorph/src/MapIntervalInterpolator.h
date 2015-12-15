#ifndef __IntervalInterpolator_HH__
#define __IntervalInterpolator_HH__

#include "Numerics.h"
#include "IntervalInterpolator.h"
#include "AllochthonousLithologyDistribution.h"

namespace AllochMod {

  /** @addtogroup AllochMod
   *
   * @{
   */


  using Numerics::FloatingPoint;



  /// \brief Computes the interpolator for an input map-pair.
  class MapIntervalInterpolator : public Numerics::IntervalInterpolator {

  public:

    /// \var PolynomialDegree
    /// \brief The degree of polynomial used in the interpolator.
    // This must be set to NoPolynomial.
    // At present the nearest neighbour set in the cardinal function may produce
    // a set of points that is entirely from one map, thus some of the rows in the 
    // polynomial part of the matrix will be linearly dependent. Another scheme
    // for picking points must be implemented, e.g. the nearest neighbours and 
    // some points from both maps, is a simple solution, these points may be the
    // most distantly separated points, digaonally opposite corners from each map, for example.
    static const int DefaultPolynomialDegree = Numerics::NoPolynomial; //Numerics::QuadraticPolynomial;

    MapIntervalInterpolator ();

    /// \brief Compute the interpolant on the interval bounded by the input maps.
    void computeInterpolator ( AllochthonousLithologyDistribution* lower,
                               AllochthonousLithologyDistribution* upper,
                               const int                           debugLevel );

    /// \brief Assign the interpolation points and coefficients of the basis function.
    void setInterpolationData ( const Numerics::PointArray& interpolationPoints,
                                const Numerics::Vector&     coefficients );


    /// Return the interpolation RHS vector.
    const Numerics::Vector& getInterpolationVector () const;

  protected :

    /// \brief Concatinates the surface, exterior and interior points into a single array.
    ///
    /// The points are also translated and scaled.
    void setInterpolationPoints ( Numerics::PointArray& interpolationPoints ) const;

    /// \brief Initialise the interpolation vector for solving the interpolation equations.
    void setInterpolationVector ( Numerics::Vector& rhs ) const;


    /// \brief A set of points that lie on the surface of the allochthonous body.
    Numerics::PointArray surfacePoints;

    /// \brief A set of points that lie on the exterior of the allochthonous body.
    Numerics::PointArray exteriorBodyPoints;

    /// \brief A set of points that lie on the interior of the allochthonous body.
    Numerics::PointArray interiorBodyPoints;

    /// \brief The constraints of the interpolation problem, the rhs vector.
    Numerics::Vector interpolationVector;


  private :

    /// \brief Initialise the surface, exterior and interior point sets.
    void initialisePoints ( AllochthonousLithologyDistribution* lower,
                            AllochthonousLithologyDistribution* upper );


    /// \brief Update the exterior and interior point sets in the areas the interpolant was in error for both maps.
    void enhancePointSet (       AllochthonousLithologyDistribution& lower,
                                 AllochthonousLithologyDistribution& upper,
                           const FloatingPoint                       exteriorErrorLevel,
                           const FloatingPoint                       interiorErrorLevel,
                           const int                                 xBound,
                           const int                                 yBound,
                                 int&                                numberOfPointsAdded,
			   const int                                 debugLevel );

    /// \brief Update the exterior and interior point sets in the areas the interpolant was in error for a specific map.
    void enhancePointSet (       AllochthonousLithologyDistribution& saltBody,
                           const FloatingPoint                       exteriorErrorLevel,
                           const FloatingPoint                       interiorErrorLevel,
                           const int                                 xBound,
                           const int                                 yBound,
                                 int&                                numberOfPointsAdded,
			   const int                                 debugLevel );

    /// \brief Transform the point array by the transformation data.
    ///
    /// The translation and scaling vectors.
    void transformPointArray ( Numerics::PointArray& interpolationPoints ) const;

    /// \brief Compute an interpolator for the given surface, exterior and interior point sets.
    virtual void computeInterpolator ( const int debugLevel );


  };

  /** @} */

}

#endif // __IntervalInterpolator_HH__
