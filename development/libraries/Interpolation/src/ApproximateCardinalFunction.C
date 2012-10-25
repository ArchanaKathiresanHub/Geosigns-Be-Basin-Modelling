//------------------------------------------------------------//

#include "ApproximateCardinalFunction.h"

//------------------------------------------------------------//

#include "FortranMatrixOperations.h"

#include <algorithm>
#include <iostream>

//------------------------------------------------------------//

Numerics::ApproximateCardinalFunction::ApproximateCardinalFunction () {
  coefficients = 0;
}

//------------------------------------------------------------//

Numerics::ApproximateCardinalFunction::~ApproximateCardinalFunction () {

  if ( coefficients != 0 ) {
    delete coefficients;
    coefficients = 0;
  }

}

//------------------------------------------------------------//

Numerics::FloatingPoint Numerics::ApproximateCardinalFunction::innerProduct ( const Vector& theVector ) const {

  FloatingPoint result = 0.0;
  int           I;

  // Do the loop in reverse, because the smaller numbers (in magnitude) are at 
  // the end of the vector and this should help in ill-conditioned systems.
  // For optimisation purposes, would it be better to order the neighbours/coefficients
  // (sorting in ascending order, on the neighbour number)? Rather than, as happens now,
  // when doing this calculation, neighbours_i jumps "randomly" around in position.
  for ( I = neighbours.size () - 1; I >= 0; I-- ) {
    result = result + (*coefficients)( I ) * theVector ( neighbours [ I ]);
  }

  return result;
}

//------------------------------------------------------------//

void Numerics::ApproximateCardinalFunction::computeCardinalFunction ( const DenseMatrix& systemMatrix,
                                                                      const Vector&      rsSquared,
                                                                      const int          numberOfNeighbours,
                                                                      const int          numberOfPolynomialTerms ) {

  int dimension    = numberOfNeighbours + numberOfPolynomialTerms;
  IntegerArray permutation;
  FortranMatrix cardinalMatrix ( dimension, dimension );

  neighbours.resize ( dimension );
  coefficients = new Vector ( dimension );
  permutation.resize ( dimension );

  // Find the nearest neighbour set.
  getNearestNeighbours ( neighbours, rsSquared, numberOfNeighbours, numberOfPolynomialTerms );

  // Pull out the entries for the cardinal matrix.
  systemMatrix.getSubMatrix ( neighbours, cardinalMatrix.data ());

  coefficients->fill ( 0.0 );
  (*coefficients)( 0 ) = 1.0;

  // Factorise the cardinal matrix.
  luFactorise ( cardinalMatrix, permutation );

  // Then solve the cardinal matrix equation.
  backSolve ( cardinalMatrix, permutation, *coefficients );
}

//------------------------------------------------------------//

void Numerics::ApproximateCardinalFunction::getNearestNeighbours
   (       IntegerArray& nearestNeighbours,
     const Vector&       rsSquared,
     const int           numberOfNeighbours,
     const int           numberOfPolynomialTerms ) {

  const int numberOfRs = rsSquared.dimension ();

  int          i;
  IntegerArray pointer ( numberOfRs );
  NeighbourComparison comp ( rsSquared );

  // Initialise pointer to be (0, 1, 2, 3, ...).
  for ( i = 0; i < numberOfRs; i++ ) {
    pointer [ i ] = i;
  }

  // Sort the first 0 .. numberOfNeighbours - 1 rsSquared, using a pointer sort.
  std::partial_sort ( pointer.begin (), pointer.begin () + numberOfNeighbours, pointer.end (), comp );

  for ( i = 0; i < numberOfNeighbours; i++ ) {
    nearestNeighbours [ i ] = pointer [ i ];
  }

  // Add in the polynomial part.
  for ( i = 0; i < numberOfPolynomialTerms; i++ ) {
    nearestNeighbours [ numberOfNeighbours + i ] = i + numberOfRs;
  }

}

//------------------------------------------------------------//

void Numerics::ApproximateCardinalFunction::print ( const std::string&  preconditionerName,
                                                    const int           cardinalFunctionNumber,
                                                          std::ostream& output ) const {

  int i;

  for ( i = 0; i < coefficients->dimension (); i++ ) {
    output << preconditionerName << " ( " << cardinalFunctionNumber + 1 << ", " << neighbours [ i ] + 1 << " ) = " << (*coefficients)( i ) << ";" << std::endl;
  }

}

//------------------------------------------------------------//
