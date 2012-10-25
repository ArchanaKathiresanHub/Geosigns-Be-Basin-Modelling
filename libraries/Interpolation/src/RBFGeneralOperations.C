//------------------------------------------------------------//

#include "RBFGeneralOperations.h"

#include <iostream>

//------------------------------------------------------------//

void Numerics::assembleInterpolationMatrixPolynomialPart ( const PointArray&  interpolationPoints,
                                                           const int          polynomialDegree,
                                                                 DenseMatrix& interpolationMatrix ) {

  const int NumberOfPoints = interpolationPoints.size ();

  int i;
  int j;

  if ( polynomialDegree >= ConstantPolynomial ) {
    Vector polynomialPart ( numberOfPolynomialTerms< Point::DIMENSION >( polynomialDegree ));

    for ( i = 0; i < NumberOfPoints; i++ ) {
      assemblePolynomialPart ( interpolationPoints [ i ], polynomialDegree, polynomialPart );

      for ( j = 0; j < polynomialPart.dimension (); j++ ) {
        interpolationMatrix ( i, NumberOfPoints + j ) = polynomialPart ( j );
      }

      for ( j = 0; j < polynomialPart.dimension (); j++ ) {
        interpolationMatrix ( NumberOfPoints + j, i ) = polynomialPart ( j );
      }

    }

  }

}

//------------------------------------------------------------//

void Numerics::assemblePolynomialPart ( const Point&  evaluationPoint, 
                                        const int     polynomialDegree,
                                              Vector& polynomialPart ) {


  int i;
  int j;
  int count = 1;

  switch ( polynomialDegree ) {

  case ConstantPolynomial : 

    polynomialPart ( 0 ) = 1.0;

    break;

  case LinearPolynomial :

    polynomialPart ( 0 ) = 1.0;

    for ( i = 0, count = 1; i < Point::DIMENSION; ++i, ++count ) {
      polynomialPart ( count ) = evaluationPoint ( i );
    }
    
    break;

  case QuadraticPolynomial :

    polynomialPart ( 0 ) = 1.0;

    for ( i = 0, count = 1; i < Point::DIMENSION; ++i, ++count ) {
      polynomialPart ( count ) = evaluationPoint ( i );
    }

    for ( i = 0; i < Point::DIMENSION - 1; ++i ) {

      for ( j = i + 1; j < Point::DIMENSION; j++, ++count ) {
        polynomialPart ( count ) = evaluationPoint ( i ) * evaluationPoint ( j );
      }

    }

    for ( i = 0; i < Point::DIMENSION; ++i, ++count ) {
      polynomialPart ( count ) = evaluationPoint ( i ) * evaluationPoint ( i );
    }

    break;

  default :

    break;

  }
  

}

//------------------------------------------------------------//

void Numerics::computeRsSquared ( const Point&      evaluationPoint, 
                                  const PointArray& interpolationPoints,
                                        Vector&     rsSquared ) {

  int i;

  for ( i = 0; i < interpolationPoints.size (); i++ ) {
    rsSquared ( i ) = separationDistanceSquared ( evaluationPoint, interpolationPoints [ i ]);
  }

}

//------------------------------------------------------------//
