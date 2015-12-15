#ifndef __Numerics_ApproximateCardinalFunction_HH__
#define __Numerics_ApproximateCardinalFunction_HH__

#include <vector>

// Common types
#include "WallTime.h"
#include "Numerics.h"
#include "Vector.h"

// Linear algebra types
#include "DenseMatrix.h"
#include "FortranMatrix.h"
#include "Preconditioner.h"

namespace Numerics {

  
  /** @addtogroup Interpolation
   *
   * @{
   */

  /// \file ApproximateCardinalFunction.h
  /// \brief Adds an approximate cardinal function class to the Numerics namespace.

  /// \brief Approximate cardinal function class.
  class ApproximateCardinalFunction {

  protected :

    /// \brief Enable a pointer sort of array of r-squared values.
    ///
    /// The pointer is an integer array.
    class NeighbourComparison {
    public :

      /// \brief Constructor.
      NeighbourComparison ( const Vector&  newRsSquared ) : rsSquared ( newRsSquared ) {}

      /// \brief Returns of the rs-squared (p1) < rs-squared (p2)
      bool operator ()( const int p1,
                        const int p2 ) const;

    private :

      /// \brief A reference to the rs-squared array.
      const Vector& rsSquared;

    };

  public :

    /// \brief Default constructor.
    ApproximateCardinalFunction ();

    /// \brief Default destructor.
    virtual ~ApproximateCardinalFunction ();

    /// \brief Computes the cardinal function
    virtual void computeCardinalFunction
       ( const DenseMatrix& systemMatrix,
         const Vector&      rsSquared,
         const int          numberOfNeighbours,
         const int          numberOfPolynomialTerms );

    /// \brief Inner product of a vector with the approximate cardinal function.
    virtual FloatingPoint innerProduct ( const Vector& theVector ) const;

    /// \brief Print approximate cardinal function in matlab format.
    void print ( const std::string&  preconditionerName,
                 const int           cardinalFunctionNumber,
                       std::ostream& output ) const;


  protected :

    /// \brief Find the nearest n neighbours.
    void getNearestNeighbours
       (       IntegerArray& nearestNeighbours,
         const Vector&       rsSquared,
         const int           numberOfNeighbours,
         const int           numberOfPolynomialTerms );


    /// \brief The positions of the nearest neighbours.
    IntegerArray  neighbours;

    /// \brief The coefficients of the cardinal function.
    // I would like to remove the pointer but am unable to do so, 
    // as the programme crashes immediately on construction of the
    // cardinal preconditioner.
    Vector*       coefficients;

  };

  /// \typedef ApproximateCardinalFunctionArray
  /// \brief Define an array of approximate cardinal functions.
  typedef std::vector<ApproximateCardinalFunction> ApproximateCardinalFunctionArray;

  /** @} */

}

//------------------------------------------------------------//

// Inline functions

inline bool Numerics::ApproximateCardinalFunction::NeighbourComparison::operator ()( const int p1,
                                                                                     const int p2 ) const {
  return rsSquared ( p1 ) < rsSquared ( p2 );
}


#endif // __Numerics_ApproximateCardinalFunction_HH__
