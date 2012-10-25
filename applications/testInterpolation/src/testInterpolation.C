#include <iostream>
#include <iomanip>


#include "Vector.h"
#include "FortranMatrix.h"
#include "Preconditioner.h"
#include "ApproximateCardinalPreconditioner.h"
#include "GMRes.h"
#include "Point.h"
#include "RBFCubic.h"
#include "RBFInterpolant.h"
#include "RBFGeneralOperations.h"

using Numerics::FloatingPoint;

typedef Numerics::RBFCubic RadialBasisFunction;

typedef Numerics::RBFInterpolant<RadialBasisFunction> RBFInterpolant;

typedef Numerics::ApproximateCardinalPreconditioner<RadialBasisFunction> CardinalPreconditioner;

int main ( int argc, char* argv []) {

  const int Size = std::atoi ( argv [ 1 ]);
  const int NumberOfPoints = Size * Size * Size;
  const int NumberOfNeighbours = std::atoi ( argv [ 2 ]);

  const int PolynomialDegree = Numerics::ConstantPolynomial;
  const int MatrixSize = NumberOfPoints + Numerics::numberOfPolynomialTerms< Numerics::Point::DIMENSION >( PolynomialDegree );

  const bool PrintResiduals = true;

  int i;
  int j;
  int k;
  size_t l;
  int count;
  FloatingPoint H = 1.0 / FloatingPoint ( Size - 1 );
  
  Numerics::FortranMatrix interpolationMatrix ( MatrixSize, MatrixSize );
  Numerics::PointArray interpolationPoints ( NumberOfPoints );
  Numerics::GMRes gmres;

  Numerics::Vector vec ( MatrixSize );
  Numerics::Vector result ( MatrixSize );
  Numerics::Vector interpolationVector ( MatrixSize );
  Numerics::Vector coefficients ( MatrixSize );
  RBFInterpolant interp;

  // Generate simple regular grid of points.
  for ( i = 0, count = 0; i < Size; ++i ) {

    for ( j = 0; j < Size; ++j ) {

      for ( k = 0; k < Size; ++k, ++count ) {
        interpolationPoints [ count ]( 0 ) = FloatingPoint ( i ) * H;
        interpolationPoints [ count ]( 1 ) = FloatingPoint ( j ) * H;
        interpolationPoints [ count ]( 2 ) = FloatingPoint ( k ) * H;
      }

    }

  }

  std::cout.flags ( std::ios::scientific );
  std::cout.precision ( 8 );

  interp.setInterpolationPoints ( interpolationPoints );
  interp.setPolynomialDegree ( PolynomialDegree );

  CardinalPreconditioner precond ( interp, NumberOfNeighbours );

  Numerics::assembleInterpolationMatrix ( interp.getRadialBasisFunction (),
                                          interpolationPoints,
                                          interp.getPolynomialDegree (),
                                          interpolationMatrix );

  precond.assemblePreconditioner ( interpolationMatrix );
  interpolationVector.fill ( 1.0 );

  // The RHS for the polynomial part is zero.
  for ( i = NumberOfPoints; i < interpolationVector.dimension (); i++ ) {
    interpolationVector ( i ) = 0.0;
  }

  coefficients.fill ( 0.0 );

  // Set up the linear solver
  gmres.setTolerance ( 1.0e-6 * vectorLength ( interpolationVector ));
  gmres.setPrintResidual ( PrintResiduals );
  gmres.setMatrix ( interpolationMatrix );
  gmres.setPreconditioner ( precond );

  // Solve the rbf-interpolation equation.
  gmres ( interpolationVector, coefficients );

  coefficients.print ( " coefficients ", std::cout );
  interp.setCoefficients ( coefficients );

  std::cout << std::endl;
  std::cout << "+--------+--------------------+--------------------+----------------------+" << std::endl;
  std::cout << "|    i   |        RHS_i       |      interp(x_i)   |    (RHS-interp)/RHS  |" << std::endl;
  std::cout << "+--------+--------------------+--------------------+----------------------+" << std::endl;

  for ( l = 0; l < interpolationPoints.size (); ++l ) {
    FloatingPoint eval = interp ( interpolationPoints [ l ]);
    std::cout << "|" << std::setw ( 6 ) << l + 1 << "  |" 
              << std::setw ( 18 ) << interpolationVector ( l ) << "  |" 
              << std::setw ( 18 ) << eval << "  |  " 
              << std::setw ( 18 ) << ( interpolationVector ( l ) - eval ) / interpolationVector ( l ) 
              << "  |"
              << std::endl;
  }

  std::cout << "+--------+--------------------+--------------------+----------------------+" << std::endl;

  return 0;
}
