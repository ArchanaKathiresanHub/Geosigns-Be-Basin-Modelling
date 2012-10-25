#ifndef __Numerics_ApproximateCardinalPreconditioner_HH__
#define __Numerics_ApproximateCardinalPreconditioner_HH__

// Linear algebra
#include "Vector.h"
#include "DenseMatrix.h"
#include "FortranMatrix.h"
#include "FortranMatrixOperations.h"
#include "Preconditioner.h"

// Interpolation
#include "ApproximateCardinalFunction.h"
#include "RBFInterpolant.h"

namespace Numerics {


  /** @addtogroup Interpolation
   *
   * @{
   */

  /// \file ApproximateCardinalPreconditioner.h
  /// \brief Adds an approximate cardinal preconditioner class to the Numerics namespace.

  /// \brief Extends the abstract Preconditioner class and implements a preconditioner based on approximate cardinal functions.
  ///
  /// For references refer to the requirements document.
  template <class RadialBasisFunction>
  class ApproximateCardinalPreconditioner : public Preconditioner {

  public :

    /// \brief Constructor:
    ///
    /// @param interp The interpolant ..
    /// @param newNumberOfNeighbours The number of points to be used in the approximate cardinal functions.
    // It would be better to remove the arguments from the constructor here and create a
    // function performing the same action, since the interponant is not required just the interpolation points.
    // The template could then be removed from this class.
    ApproximateCardinalPreconditioner ( const RBFInterpolant<RadialBasisFunction>& interp,
                                        const int newNumberOfNeighbours );

    ~ApproximateCardinalPreconditioner ();

    /// \brief Compute the approximate cardinal function preconditioner.
    void assemblePreconditioner ( const DenseMatrix& fromTheMatrix ); 

    /// \brief Solve preconditioner problem.
    ///
    /// Solve the cardinal function problem for each of the interpolation points.
    /// Basically this is just a sparse matrix vector product.
    /// Over-rides the base class abstract solve function.
    void solve ( const Vector& theVector, 
                       Vector& result ) const;


    /// \brief Print values of the vector to the output stream.
    void print ( const std::string&  preconditionerName,
                       std::ostream& output ) const;

  protected:


    /// \brief Compute part of the preconditioner associated with the 
    /// polynomial part of the interpolation problem.
    void assemblePreconditionerPolynomialPart ();

    /// \brief Solve part of the preconditioner associated with the 
    /// polynomial part of the interpolation problem.
    void solvePolynomialPart ( const Vector& theVector, 
                                     Vector& result ) const;


    /// \brief The interpolant.
    const RBFInterpolant<RadialBasisFunction>& interpolant;

    /// \brief The sequence of approximate cardinal functions.
    ApproximateCardinalFunctionArray cardinalFunctions;

    /// \brief The lu-factors for solving the polynomial part of the preconditioner.
    FortranMatrix* polynomialMatrix;

    /// \brief Used in the computing and back-solving of the lu-factors in the polynomial part of the preconditioner.
    IntegerArray   permutation;

    /// \brief The number of neighbours.
    int numberOfNeighbours;

  };


  /** @} */

}

//------------------------------------------------------------//

template <class RadialBasisFunction>
Numerics::ApproximateCardinalPreconditioner< RadialBasisFunction>::ApproximateCardinalPreconditioner ( const RBFInterpolant<RadialBasisFunction>& interp,
                                                                                                       const int newNumberOfNeighbours ) :
  interpolant ( interp ) {

  cardinalFunctions.resize ( interp.getInterpolationPoints ().size ());

  numberOfNeighbours = newNumberOfNeighbours;
  polynomialMatrix = 0;
}

//------------------------------------------------------------//

template <class RadialBasisFunction>
Numerics::ApproximateCardinalPreconditioner< RadialBasisFunction>::~ApproximateCardinalPreconditioner () {

  if ( polynomialMatrix != 0 ) {
    delete polynomialMatrix;
    polynomialMatrix = 0;
  }

}

//------------------------------------------------------------//

template <class RadialBasisFunction>
void Numerics::ApproximateCardinalPreconditioner< RadialBasisFunction>::assemblePreconditioner ( const DenseMatrix& fromTheMatrix ) {

  // Need to remove the polynomial dependency from the ApproximateCardinalFunction.
  const int NumberOfPolynomialTerms = 0;//numberOfPolynomialTerms<Point::DIMENSION>( interpolant.getPolynomialDegree ());

  const PointArray& interpolationPoints = interpolant.getInterpolationPoints ();

  Vector rsSquared ( interpolationPoints.size ());

  size_t i;

  for ( i = 0; i < interpolationPoints.size (); i++ ) {
    computeRsSquared ( interpolationPoints [ i ], interpolationPoints, rsSquared );

    // Need to remove the polynomial dependency from the ApproximateCardinalFunction.
    cardinalFunctions [ i ].computeCardinalFunction ( fromTheMatrix,
                                                      rsSquared, 
                                                      numberOfNeighbours,
                                                      NumberOfPolynomialTerms );
  }

  assemblePreconditionerPolynomialPart ();
}

//------------------------------------------------------------//

template <class RadialBasisFunction>
void Numerics::ApproximateCardinalPreconditioner<RadialBasisFunction>::assemblePreconditionerPolynomialPart () {

  const int PolynomialDegree = interpolant.getPolynomialDegree ();

  if ( PolynomialDegree >= 0 ) {
    const int NumberOfPolynomialTerms = numberOfPolynomialTerms<Point::DIMENSION>( PolynomialDegree );
    const PointArray& interpolationPoints = interpolant.getInterpolationPoints ();

    if ( polynomialMatrix == 0 ) {
      polynomialMatrix = new FortranMatrix ( NumberOfPolynomialTerms, NumberOfPolynomialTerms );
    } else if ( polynomialMatrix->numberOfRows () != NumberOfPolynomialTerms ) {
      delete polynomialMatrix;
      polynomialMatrix = new FortranMatrix ( NumberOfPolynomialTerms, NumberOfPolynomialTerms );
    }

    if ( PolynomialDegree == 0 ) {
      (*polynomialMatrix)( 0, 0 ) = 1.0 / FloatingPoint ( interpolationPoints.size ());
    } else {
      Vector polynomialPart ( NumberOfPolynomialTerms );
      size_t i;

      polynomialMatrix->fill ( 0.0 );

      for ( i = 0; i < interpolationPoints.size (); i++ ) {
        assemblePolynomialPart ( interpolationPoints [ i ], PolynomialDegree, polynomialPart );
        addOuterProduct ( *polynomialMatrix, polynomialPart, polynomialPart );
      }

      permutation.resize ( NumberOfPolynomialTerms );
      luFactorise ( *polynomialMatrix, permutation );
    }


  } else {
    polynomialMatrix = 0;
  }

}

//------------------------------------------------------------//

template <class RadialBasisFunction>
void Numerics::ApproximateCardinalPreconditioner<RadialBasisFunction>::solve ( const Vector& theVector,
                                                                                     Vector& result ) const {

  const PointArray& interpolationPoints = interpolant.getInterpolationPoints ();

  size_t i;

  for ( i = 0; i < interpolationPoints.size (); i++ ) {
    result ( i ) = cardinalFunctions [ i ].innerProduct ( theVector );
  }

  solvePolynomialPart ( theVector, result );
}

//------------------------------------------------------------//

template <class RadialBasisFunction>
void Numerics::ApproximateCardinalPreconditioner< RadialBasisFunction>::solvePolynomialPart ( const Vector& theVector,
                                                                                                    Vector& result ) const {

  const int PolynomialDegree = interpolant.getPolynomialDegree ();

  if ( PolynomialDegree >= 0 ) {
    const int NumberOfPolynomialTerms = numberOfPolynomialTerms<Point::DIMENSION>( PolynomialDegree );
    const PointArray& interpolationPoints = interpolant.getInterpolationPoints ();

    int i;

    if ( PolynomialDegree == 0 ) {
      FloatingPoint sum = 0.0;

      for ( i = 0; i < int ( interpolationPoints.size ()); i++ ) {
        sum += theVector ( i );
      }

      result ( result.dimension () - 1 ) = sum * (*polynomialMatrix)( 0, 0 );
    } else {
      Vector polynomialVector ( NumberOfPolynomialTerms );
      Vector polynomialResult ( NumberOfPolynomialTerms );
      Vector polynomialPart   ( NumberOfPolynomialTerms );

      int polynomialCount;

      polynomialVector.fill ( 0.0 );

      for ( i = 0; i < int ( interpolationPoints.size ()); i++ ) {
        assemblePolynomialPart ( interpolationPoints [ i ], PolynomialDegree, polynomialPart );
        polynomialPart *= theVector ( i );
        polynomialVector += polynomialPart;
      }

      polynomialResult = polynomialVector;
      backSolve ( *polynomialMatrix, permutation, polynomialResult );

      for ( i = int ( interpolationPoints.size ()), polynomialCount = 0; i < result.dimension (); ++i, ++polynomialCount ) {
        result ( i ) = polynomialResult ( polynomialCount );
      }

    }

  }

}

//------------------------------------------------------------//

template <class RadialBasisFunction>
void Numerics::ApproximateCardinalPreconditioner< RadialBasisFunction>::print ( const std::string&  preconditionerName,
                                                                                      std::ostream& output ) const {

  size_t i;
  int row;
  int col;

  for ( i = 0; i < cardinalFunctions.size (); i++ ) {
    cardinalFunctions [ i ].print ( preconditionerName, i, output );
  }

  if ( polynomialMatrix != 0 ) {

    for ( row = 0; row < polynomialMatrix->numberOfRows (); row++ ) {

      for ( col = 0; col < polynomialMatrix->numberOfColumns (); col++ ) {
	output << preconditionerName 
	       << " ( " 
	       << cardinalFunctions.size () + row + 1 << ", " 
	       << cardinalFunctions.size () + col + 1 << " ) = " 
	       << (*polynomialMatrix)( row, col ) << ";"
	       << std::endl;
      }

    }

  }

}

//------------------------------------------------------------//

#endif // __Numerics_ApproximateCardinalPreconditioner_HH__
