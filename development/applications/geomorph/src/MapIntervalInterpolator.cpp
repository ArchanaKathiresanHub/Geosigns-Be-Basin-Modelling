//------------------------------------------------------------//

#include <iostream>

#include "MapIntervalInterpolator.h"

#include "SubsampledGridPointCondenser.h"
#include "PointSelectionArbiter.h"
#include "PrescribedGridPointCondenser.h"
#include "UniformRandomGenerator.h"
#include "ErrorPointSet.h"

// Include matrix type
#include "DenseRowMatrix.h"
#include "FortranMatrix.h"

// Include Preconditioner type
#include "Preconditioner.h"
#include "ApproximateCardinalPreconditioner.h"

// Include linear solver type
#include "LinearSolver.h"
#include "GMRes.h"

// Interpolation functions
#include "RBFGeneralOperations.h"

//------------------------------------------------------------//

AllochMod::MapIntervalInterpolator::MapIntervalInterpolator () {
  setPolynomialDegree ( DefaultPolynomialDegree );
}

//------------------------------------------------------------//

void AllochMod::MapIntervalInterpolator::computeInterpolator ( GeoMorphAllochthonousLithologyDistribution* lower,
                                                               GeoMorphAllochthonousLithologyDistribution* upper,
                                                               const int                           debugLevel ) {

  const unsigned int oldPrecision = std::cout.precision ( 8 );
  const std::ios::fmtflags oldFlags = std::cout.flags(std::ios::scientific);

  const int MaximumNumberOfIterations = 10;

  int numberOfPointsAdded;
  int iterationCount = 0;

  const int xBound = Numerics::integerMax ( 1, lower->mapXDimension () / 100 );
  const int yBound = Numerics::integerMax ( 1, lower->mapYDimension () / 100 );

  FloatingPoint exteriorErrorLevel;
  FloatingPoint interiorErrorLevel;


  setIntervalBounds ( lower->getAge (), upper->getAge ());


//   lowerBound = lower->getAge ();
//   upperBound = upper->getAge ();

  initialisePoints ( lower, upper );

  if ( debugLevel >= 1 ) {
    std::cout << " Points count is: " << surfacePoints.size () << "  " 
              << exteriorBodyPoints.size () << "  " 
              << interiorBodyPoints.size () << std::endl;

    std::cout << " Age bounds: " << getStartAge () << "  " << getEndAge () << std::endl;
  }


  computeInterpolator ( debugLevel );

  exteriorErrorLevel = 0.0;
  interiorErrorLevel = 0.0;

  do {
    enhancePointSet ( *lower, *upper, exteriorErrorLevel, interiorErrorLevel, xBound, yBound, numberOfPointsAdded, debugLevel );

    if ( debugLevel >= 1 ) {
      std::cout << " Enhanced point count is: " 
                << surfacePoints.size () << "  " 
                << exteriorBodyPoints.size () << "  " 
                << interiorBodyPoints.size () << std::endl;
    }

    if ( numberOfPointsAdded != 0 ) {
      computeInterpolator ( debugLevel );
    }

    exteriorErrorLevel = 0.01 * ExteriorConstraint;
    interiorErrorLevel = 0.01 * InteriorConstraint;
    ++iterationCount;
  } while ( numberOfPointsAdded != 0 && iterationCount <= MaximumNumberOfIterations );

  std::cout.precision ( oldPrecision );
  std::cout.flags( oldFlags );
}

//------------------------------------------------------------//

void AllochMod::MapIntervalInterpolator::setInterpolationData ( const Numerics::PointArray& interpolationPoints,
                                                                const Numerics::Vector&     coefficients ) {

  Numerics::PointArray transformedInterpolationPoints ( interpolationPoints );


  transformPointArray ( transformedInterpolationPoints );

  interpolator.setInterpolationPoints ( transformedInterpolationPoints );
  interpolator.setCoefficients ( coefficients );
}

//------------------------------------------------------------//

void AllochMod::MapIntervalInterpolator::setInterpolationPoints ( Numerics::PointArray& interpolationPoints ) const {

  size_t i;
  size_t pointCount;

  // First the points on the boundary of the salt body.
  for ( i = 0, pointCount = 0; i < surfacePoints.size (); i++, pointCount++ ) {
    interpolationPoints [ pointCount ] = surfacePoints [ i ];
  }

  // Then the points that are outside the salt body.
  for ( i = 0; i < exteriorBodyPoints.size (); i++, pointCount++ ) {
    interpolationPoints [ pointCount ] = exteriorBodyPoints [ i ];
  }

  // And finally the points that are inside the salt body.
  for ( i = 0; i < interiorBodyPoints.size (); i++, pointCount++ ) {
    interpolationPoints [ pointCount ] = interiorBodyPoints [ i ];
  }

  transformPointArray ( interpolationPoints );
}

//------------------------------------------------------------//

void AllochMod::MapIntervalInterpolator::setInterpolationVector ( Numerics::Vector& rhs ) const {

  const int lastPoint = surfacePoints.size () + exteriorBodyPoints.size () + interiorBodyPoints.size ();
  int i;

  rhs.fill ( 0.0 );

  // Assign 0 to all thos points that lie on the surface of the salt body.
  for ( i = 0; i < int ( surfacePoints.size ()); i++ ) {
    rhs ( i ) = SurfaceConstraint;
  }

  // Assign the correct value to all those points that lie outside the salt body.
  for ( i = int ( surfacePoints.size ()); i < int ( surfacePoints.size () + exteriorBodyPoints.size ()); i++ ) {
    rhs ( i ) = ExteriorConstraint;
  }

  // Assign the correct value to all those points that lie inside the salt body.
  for ( i = int ( surfacePoints.size () + exteriorBodyPoints.size ()); i < lastPoint; i++ ) {
    rhs ( i ) = InteriorConstraint;
  }

}

//------------------------------------------------------------//

void AllochMod::MapIntervalInterpolator::initialisePoints ( GeoMorphAllochthonousLithologyDistribution* lower,
                                                            GeoMorphAllochthonousLithologyDistribution* upper ) {

  const int xResampleFactor = Numerics::integerMax ( 1, 2 + int ( 0.048 * FloatingPoint ( lower->mapXDimension ())));
  const int yResampleFactor = Numerics::integerMax ( 1, 2 + int ( 0.048 * FloatingPoint ( lower->mapYDimension ())));

  PointSelectionArbiter<Numerics::UniformRandomGenerator> extract;
  SubsampledGridPointCondenser gridExtractor ( xResampleFactor, yResampleFactor );

  // Clear all previously stored points.
  surfacePoints.clear ();
  interiorBodyPoints.clear ();
  exteriorBodyPoints.clear ();

  // Tell extraction function to extract ALL the points from the surface point set.
  extract.setThreshold ( 1.0 );

  // Get ALL points from the surface of the salt body.
  lower->pickOutBoundaryPoints ( extract, surfacePoints );
  upper->pickOutBoundaryPoints ( extract, surfacePoints );

  // Extract 1% of the points in the interior point set.
  // This number should probably be set different.
  // However 1% seems to give a fairly good point set for the interior.
  // The points are pick out in a pseudo-random fashion.
  // What is the reason for picking these points randmoly?
  extract.setThreshold ( 0.01 );

  lower->pickOutInteriorPoints ( extract, interiorBodyPoints );
  upper->pickOutInteriorPoints ( extract, interiorBodyPoints );

  // Pick out a regular grid as possible from the exterior points.
  gridExtractor.setSubSampling ( xResampleFactor, yResampleFactor );

  lower->pickOutExteriorPoints ( gridExtractor, exteriorBodyPoints );
  upper->pickOutExteriorPoints ( gridExtractor, exteriorBodyPoints );

}

//------------------------------------------------------------//

const Numerics::Vector& AllochMod::MapIntervalInterpolator::getInterpolationVector () const {
  return interpolationVector;
}

//------------------------------------------------------------//

void AllochMod::MapIntervalInterpolator::enhancePointSet (       GeoMorphAllochthonousLithologyDistribution& lower,
                                                                 GeoMorphAllochthonousLithologyDistribution& upper,
                                                           const FloatingPoint                       exteriorErrorLevel,
                                                           const FloatingPoint                       interiorErrorLevel,
                                                           const int                                 xBound,
                                                           const int                                 yBound,
                                                                 int&                                numberOfPointsAdded,
                                                           const int                                 debugLevel ) {

  int lowerPointsAdded;
  int upperPointsAdded;

  enhancePointSet ( lower, exteriorErrorLevel, interiorErrorLevel, xBound, yBound, lowerPointsAdded, debugLevel );
  enhancePointSet ( upper, exteriorErrorLevel, interiorErrorLevel, xBound, yBound, upperPointsAdded, debugLevel );

  numberOfPointsAdded = lowerPointsAdded + upperPointsAdded;
}

//------------------------------------------------------------//

void AllochMod::MapIntervalInterpolator::enhancePointSet (        GeoMorphAllochthonousLithologyDistribution& saltBody,
                                                            const FloatingPoint                       exteriorErrorLevel,
                                                            const FloatingPoint                       interiorErrorLevel,
                                                            const int                                 xBound,
                                                            const int                                 yBound,
                                                                  int&                                numberOfPointsAdded,
                                                            const int                                 debugLevel ) { 

  const Numerics::FloatingPointArray& xs = saltBody.getXCoordinates ();
  const Numerics::FloatingPointArray& ys = saltBody.getYCoordinates ();

  // Should these constants depend on the size of the mesh?
  const int xStride = 1;
  const int yStride = 1;

  PrescribedGridPointCondenser extract ( 0, xs.size (), 0, ys.size ());
  Numerics::Point evaluationPoint;

  ErrorPointSet errors;
  ErrorPointSet requiredPoints;

  int i;
  int j;
  int count = 0;


  FloatingPoint interpolatedValue;

  //------------------------------------------------------------//

  // Enhance the exterior point set

  extract.fill ( false );

  // First stage, find all points that have an error.
  evaluationPoint ( 2 ) = saltBody.getAge ();

  for ( i = 0; i < int ( xs.size ()); i += xStride ) {
    evaluationPoint ( 0 ) = xs [ i ];

    for ( j = 0; j < int ( ys.size ()); j += yStride ) {
      evaluationPoint ( 1 ) = ys [ j ];

      // Only try points that have not been used in the computation of the interpolant.
      if ( saltBody.pointIsExterior ( i, j ) && !saltBody.pointHasBeenSelected ( PRIMAL_MAP, i, j )) {
        interpolatedValue = operator ()( evaluationPoint );

        if ( interpolatedValue > exteriorErrorLevel ) {
          errors.addError ( interpolatedValue, i, j );
        }

      }

    }

  }

  errors.sortErrors ();
  count = 0;
  numberOfPointsAdded = 0;
  int notAdded = 0;

  // Second stage, iterate through points, starting with those most in error.
  // Collecting point that are not near those that have been selected already.
  // Here, since the interpolant should be negative, as we are looking at the exterior
  // points, we iterate backwards.
  for ( i = errors.size () - 1; i >= 0; i-- ) {

    if ( ! requiredPoints.hasNearby ( errors ( i ), xBound, yBound )) {
      requiredPoints.addError ( errors ( i ));
      extract ( errors ( i ).i, errors ( i ).j ) = true;
      count = count + 1;
    } else {
      notAdded = notAdded + 1;
    }

  }

  numberOfPointsAdded = numberOfPointsAdded + count;

  saltBody.pickOutExteriorPoints ( extract, exteriorBodyPoints );

  if ( debugLevel > 0 ) {
    std::cout << " added a further " << count << " exterior points " << notAdded << " not added " << std::endl;
  }

  //------------------------------------------------------------//

  // Enhance the interior point set
  count = 0;

  extract.fill ( false );
  errors.clear ();
  requiredPoints.clear ();

  // First stage, find all points that have an error.
  for ( i = 0; i < int ( xs.size ()); i += xStride ) {
    evaluationPoint ( 0 ) = xs [ i ];

    for ( j = 0; j < int ( ys.size ()); j += yStride ) {
      evaluationPoint ( 1 ) = ys [ j ];

      // Only try points that have not been used in the computation of the interpolant.
      if ( saltBody.pointIsInterior ( i, j ) && !saltBody.pointHasBeenSelected ( PRIMAL_MAP, i, j )) {
        interpolatedValue = operator ()( evaluationPoint );

        if ( interpolatedValue < interiorErrorLevel ) {
          errors.addError ( interpolatedValue, i, j );
        }

      }

    }

  }

  errors.sortErrors ();
  count = 0;
  notAdded =0;

  // Second stage, iterate through points, starting with those most in error.
  // Collecting points that are not near those that have been selected already.
  // Here, since the interpolant should be positive, as we are looking at the interior
  // points, we iterate forwards. The largest error will be first in the array.
  for ( i = 0; i < errors.size (); i++ ) {

    if ( ! requiredPoints.hasNearby ( errors ( i ), xBound, yBound )) {
      requiredPoints.addError ( errors ( i ));
      extract ( errors ( i ).i, errors ( i ).j ) = true;
      count = count + 1;
    } else {
      notAdded = notAdded + 1;
    }

  }

  saltBody.pickOutInteriorPoints ( extract, interiorBodyPoints );
  numberOfPointsAdded = numberOfPointsAdded + count;

  if ( debugLevel > 0 ) {
    std::cerr << " added a further " << count << " interior points " << notAdded << " not added " << std::endl;
  }

}

//------------------------------------------------------------//

void AllochMod::MapIntervalInterpolator::transformPointArray ( Numerics::PointArray& interpolationPoints ) const {

  size_t i;

  for ( i = 0; i < interpolationPoints.size (); ++i ) {

    // Translate point so bounding box starts at the origin (0,0,0).
    interpolationPoints [ i ] -= getTranslation ();
    
    // Scale the point such that the bounding box is the required size.
    interpolationPoints [ i ] *= getScaling ();
  }

}

//------------------------------------------------------------//

void AllochMod::MapIntervalInterpolator::computeInterpolator ( const int debugLevel ) {

  const int NeighbourUpperBound = 300;
  const int NeighbourLowerBound = 50;

  const int NumberOfPoints =  surfacePoints.size () + exteriorBodyPoints.size () + interiorBodyPoints.size ();

  const int InitialNumberOfNeighbours = int ( 0.025 * FloatingPoint ( NumberOfPoints ));

  const int MaximumNumberOfNeighbours = Numerics::integerMin ( InitialNumberOfNeighbours, NeighbourUpperBound );
  const int MinimumNumberOfNeighbours = Numerics::integerMax ( NeighbourLowerBound, MaximumNumberOfNeighbours );

  const int NumberOfNeighbours = Numerics::integerMin ( NumberOfPoints, MinimumNumberOfNeighbours );

  const int MatrixSize = NumberOfPoints + Numerics::numberOfPolynomialTerms< Numerics::Point::DIMENSION >( getPolynomialDegree ());


  Numerics::GMRes linearSolver;

  Numerics::Vector coefficients ( MatrixSize );

  Numerics::PointArray interpolationPoints ( NumberOfPoints );
  setInterpolationPoints ( interpolationPoints );

  // Now the points must be passed to the interpolator.
  interpolator.setInterpolationPoints ( interpolationPoints );

  if ( debugLevel > 0 ) {
    std::cout << " Starting interpolation assembly and computation:" << std::endl;
    std::cout << " There are " << NumberOfPoints << " NumberOfPoints " << std::endl;
  }

  // The interpolator needs the interpolation points before the preconditioner can be initialised.
  // This should be changed in the future.
  Numerics::FortranMatrix interpolationMatrix ( MatrixSize, MatrixSize );
  Numerics::ApproximateCardinalPreconditioner<Numerics::RBFCubic> precon ( interpolator, NumberOfNeighbours );

  Numerics::assembleInterpolationMatrix ( interpolator.getRadialBasisFunction (),
                                          interpolationPoints,
                                          getPolynomialDegree (),
                                          interpolationMatrix );

  interpolationVector.resize ( MatrixSize );

  // Set the constraints of the problem.
  //    o  0 For those points that lie on the boundary of the salt body
  //    o  C For those points that lie on the inside of the salt body
  //    o -C For those points that lie on the outside of the salt body
  setInterpolationVector ( interpolationVector );

  precon.assemblePreconditioner ( interpolationMatrix );

  // Now solve the matrix equation.
  coefficients.fill ( 0.0 );

  linearSolver.setMaximumNumberOfIterations ( 1000 );
  linearSolver.setTolerance ( 1.0e-6  );
  linearSolver.setPrintResidual ( debugLevel >= 2 );
  linearSolver.setMatrix ( interpolationMatrix );
  linearSolver.setPreconditioner ( precon );

  linearSolver ( interpolationVector, coefficients );

  if ( debugLevel > 0 ) {
    std::cout << " Linear solver took " << linearSolver.iterationsTaken () << std::endl;
  }

  interpolator.setCoefficients ( coefficients );
}

