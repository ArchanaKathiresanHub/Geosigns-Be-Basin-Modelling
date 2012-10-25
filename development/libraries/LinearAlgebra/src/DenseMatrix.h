#ifndef __Nunmerics_DenseMatrix_HH__
#define __Nunmerics_DenseMatrix_HH__

#include <iostream>
#include <string>

#include "Numerics.h"
#include "Vector.h"

/*! \defgroup LinearAlgebra Linear Algebra
 * \ingroup Numerics
 *
 */

namespace Numerics {

  
  /** @addtogroup LinearAlgebra
   *
   * @{
   */


  /// \file DenseMatrix.h
  /// \brief Provides base dense-matrix class.

  /// \brief Base class for dense matrix hierarchy used throughout numerics code.
  ///
  /// Entries are accessed in the ranges in the closed set [1,rows]x[1,columns].
  class DenseMatrix {

  public :

    virtual ~DenseMatrix () {}

    /// \brief Accessor function, returns entry at position rowPosition, colPosition.
    virtual FloatingPoint  operator ()( const int rowPosition, const int colPosition ) const = 0;

    /// \brief Accessor function, returns entry at position rowPosition, colPosition.
    virtual FloatingPoint& operator ()( const int rowPosition, const int colPosition ) = 0;

    /// \brief Returns the number of rows in the matrix.
    int numberOfRows () const;

    /// \brief Returns the number of columns in the matrix.
    int numberOfColumns () const;

    virtual bool isRowMajorOrder () const;

    /// \brief Fills the vector with a specified value.
    virtual void fill ( const FloatingPoint withTheValue ) = 0;

    /// \brief Computes the matrix-vector product.
    ///
    /// \param vec The 
    /// \param result The result of the matrix-vector product.
    virtual void matrixVectorProduct ( const Vector& vec,
                                             Vector& result ) const = 0;

    /// \brief Retrieve a sub-matrix, and put into a one-dimensional contiguous buffer.
    ///
    /// The buffer must be long enough to contain all the values of the sub-matrix.
    virtual void getSubMatrix ( const IntegerArray&  inds,
                                      FloatingPoint* subMatrixBuffer ) const;

    /// \brief Print values of the matrix to the output stream in Matlab ASCII format.
    virtual void print ( const std::string&  matrixName,
                               std::ostream& output ) const;

  protected :

    /// \brief The number of rows in the matrix.
    int rows;

    /// \brief The number of columns in the matrix.
    int columns;

  };

  /** @} */

}


//------------------------------------------------------------//
//
// Inline functions from matrix class
//


inline int Numerics::DenseMatrix::numberOfRows () const {
  return rows;
}

inline int Numerics::DenseMatrix::numberOfColumns () const {
  return columns;
}

inline bool Numerics::DenseMatrix::isRowMajorOrder () const {
  return true;
}

#endif // __Nunmerics_DenseMatrix_HH__
