#include "PThreadsMapIntervalInterpolator.h"

// Include matrix type
#include "DenseRowMatrix.h"
#include "FortranMatrix.h"
#include "PThreadsDenseRowMatrix.h"

// Include Preconditioner type
#include "Preconditioner.h"
#include "ApproximateCardinalPreconditioner.h"
#include "PThreadsApproximateCardinalPreconditioner.h"

// Include linear solver type
#include "LinearSolver.h"
#include "GMRes.h"


AllochMod::PThreadsMapIntervalInterpolator::PThreadsMapIntervalInterpolator ( const int initialNumberOfThreads ) : numberOfThreads ( initialNumberOfThreads ) {
}

void AllochMod::PThreadsMapIntervalInterpolator::computeInterpolator ( const int debugLevel ) {

  const int MinimumNumberOfPointsForThreads = 100;

  const int NeighbourUpperBound = 300;
  const int NeighbourLowerBound = 50;

  const int NumberOfPoints =  surfacePoints.size () +
                              exteriorBodyPoints.size () +
                              interiorBodyPoints.size ();

  const int InitialNumberOfNeighbours = int ( 0.025 * FloatingPoint ( NumberOfPoints ));

  const int MaximumNumberOfNeighbours = Numerics::integerMin ( InitialNumberOfNeighbours, NeighbourUpperBound );
  const int MinimumNumberOfNeighbours = Numerics::integerMax ( NeighbourLowerBound, MaximumNumberOfNeighbours );

  const int NumberOfNeighbours = Numerics::integerMin ( NumberOfPoints, MinimumNumberOfNeighbours );

  const int MatrixSize = NumberOfPoints + Numerics::numberOfPolynomialTerms< Numerics::Point::DIMENSION >( getPolynomialDegree ());

  const int NumberOfThreadsToUse = (( NumberOfPoints >= MinimumNumberOfPointsForThreads ) ? numberOfThreads : 1 );

  Numerics::GMRes linearSolver;
  Numerics::Vector coefficients ( MatrixSize );

  Numerics::PointArray interpolationPoints ( NumberOfPoints );
  setInterpolationPoints ( interpolationPoints );

  // Now the points must be passed to the interpolator.
  interpolator.setInterpolationPoints ( interpolationPoints );

  if ( debugLevel >= 1 ) {
    std::cout << " Starting interpolation assembly and computation:" << std::endl;
    std::cout << " There are " << NumberOfThreadsToUse << " threads " << std::endl;
    std::cout << " There are " << NumberOfPoints << " NumberOfPoints " << std::endl;
  }

  // The interpolator needs the interpolation points before the preconditioner can be initialised.
  // This should be changed in the future.
  Numerics::PThreadsDenseRowMatrix interpolationMatrix ( MatrixSize, MatrixSize, NumberOfThreadsToUse );
  Numerics::PThreadsApproximateCardinalPreconditioner<Numerics::RBFCubic> precon ( interpolator, NumberOfNeighbours, NumberOfThreadsToUse );

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
  //
  // Initialise the solution vector.
  coefficients.fill ( 0.0 );

  // Initialise the iterative solver.
  linearSolver.setMaximumNumberOfIterations ( 1000 );
  linearSolver.setTolerance ( 1.0e-6  );
  linearSolver.setPrintResidual ( debugLevel >= 2 );
  linearSolver.setMatrix ( interpolationMatrix );
  linearSolver.setPreconditioner ( precon );

  linearSolver ( interpolationVector, coefficients );

  if ( debugLevel ) {
    std::cout << " Linear solver took " << linearSolver.iterationsTaken () << std::endl;
  }

  interpolator.setCoefficients ( coefficients );
}
