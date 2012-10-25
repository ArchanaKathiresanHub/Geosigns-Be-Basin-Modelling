#ifndef __Numerics_FortranMatrix_HH__
#define __Numerics_FortranMatrix_HH__

#include "DenseMatrix.h"
#include "Vector.h"

namespace Numerics {

  
  /** @addtogroup LinearAlgebra
   *
   * @{
   */


  /// \file FortranMatrix.h
  /// \brief Provides fortran-matrix class.

  /// \brief Fortran matrix implementation, entries are stored in column major order. 
  ///
  /// Entries are accessed in the ranges in the half open set [0,rows)x[0,columns).
  class FortranMatrix : public DenseMatrix  {

  public :

    /// \brief Standard constructor.
    FortranMatrix ( const int rowSize, const int colSize );

    /// \brief Copy constructor.
    FortranMatrix ( const FortranMatrix& mat );

    ~FortranMatrix ();

    /// \brief Accessor function, returns entry at position rowPosition, colPosition.
    FloatingPoint  operator ()( const int rowPosition, const int colPosition ) const;

    /// \brief Accessor function, returns entry at position rowPosition, colPosition.
    FloatingPoint& operator ()( const int rowPosition, const int colPosition );

    bool isRowMajorOrder () const;

    /// \brief Fills the fortran matrix with a specified value.
    void fill ( const FloatingPoint withTheValue );

    /// \brief Computes the matrix-vector product.
    ///
    /// \param vec The 
    /// \param result The result of the matrix-vector product.
    void matrixVectorProduct ( const Vector& vec,
                                     Vector& result ) const;

    /// \brief Returns the buffer containing all the entries of the matrix.
    FloatingPoint const* data () const;

    /// \brief Returns the buffer containing all the entries of the matrix.
    FloatingPoint* data ();

    /// \brief Get square sub-matrix as indicated by the inds array.
    /// 
    /// Fill the buffer linearly, in a column major order.
    void getSubMatrix ( const IntegerArray&  inds,
                              FloatingPoint* subMatrixBuffer ) const;


    /// \brief The maximum value in the matrix.
    FloatingPoint maxValue () const;

    /// \brief The maximum absolute value in the matrix.
    FloatingPoint maxAbsValue () const;

  protected :

    FortranMatrix ();

    /// \brief Allocates and initialises the matrix arrays.
    void initialise ( const int rowSize, const int colSize );

    /// \brief Low level array containing all the entries in the matrix.
    FloatingPoint*  allValues;

    /// \brief Low level array, containing pointers to the columns of the matrix.
    FloatingPoint** allColumns;

  };

  /** @} */

}


//------------------------------------------------------------//
//
// Inline functions from matrix class
//
//------------------------------------------------------------//

inline Numerics::FloatingPoint Numerics::FortranMatrix::operator ()( const int rowPosition,
                                                                     const int colPosition ) const {
  return allColumns [ colPosition ][ rowPosition ];
}

//------------------------------------------------------------//

inline Numerics::FloatingPoint& Numerics::FortranMatrix::operator ()( const int rowPosition,
                                                                      const int colPosition ) {
  return allColumns [ colPosition ][ rowPosition ];
}

//------------------------------------------------------------//

inline Numerics::FloatingPoint const* Numerics::FortranMatrix::data () const {
  return allValues;
}

//------------------------------------------------------------//

inline Numerics::FloatingPoint* Numerics::FortranMatrix::data () {
  return allValues;
}

//------------------------------------------------------------//

inline bool Numerics::FortranMatrix::isRowMajorOrder () const {
  return false;
}

//------------------------------------------------------------//


#endif // __Numerics_FortranMatrix_HH__
