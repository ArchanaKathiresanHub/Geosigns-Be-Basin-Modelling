#ifndef __Numerics_DenseRowMatrix_HH__
#define __Numerics_DenseRowMatrix_HH__

#include "DenseMatrix.h"
#include "Vector.h"

namespace Numerics {

  
  /** @addtogroup LinearAlgebra
   *
   * @{
   */


  /// \file DenseRowMatrix.h
  /// \brief Provides a dense matrix with each row allocated separately.

  /// \brief Provides a dense matrix with each row allocated separately.
  ///
  /// The reason to include such a matrix removes the possibility of allocating
  /// a contiguous buffer of several giga-bytes, as would be required if the FortranMatrix
  /// were to be used.
  /// Entries are accessed in the ranges in the half open set [0,rows)x[0,columns).
  class DenseRowMatrix : public DenseMatrix {

  public :

    /// \brief Default constructor
    DenseRowMatrix ( const int rowCount,
                     const int colCount );


    /// \brief Default destructor
    ~DenseRowMatrix ();

    /// \brief Accessor function, returns entry at position rowPosition, colPosition.
    FloatingPoint  operator ()( const int rowPosition, const int colPosition ) const;

    /// \brief Accessor function, returns entry at position rowPosition, colPosition.
    FloatingPoint& operator ()( const int rowPosition, const int colPosition );

    /// \brief Fills the vector with a specified value.
    void fill ( const FloatingPoint withTheValue );

    void matrixVectorProduct ( const Vector& vec,
                                     Vector& result ) const;

    void getSubMatrix ( const IntegerArray&  inds,
                              FloatingPoint*  subMatrixBuffer ) const;


  protected :

    DenseRowMatrix () {}

    /// \brief A sequence containing all the rows in the matrix.
    VectorArray allRows;

  };

  /** @} */

}


//------------------------------------------------------------//

inline Numerics::FloatingPoint Numerics::DenseRowMatrix::operator ()( const int rowPosition,
                                                                      const int colPosition ) const {
  return (*allRows [ rowPosition ])( colPosition );
}

//------------------------------------------------------------//

inline Numerics::FloatingPoint& Numerics::DenseRowMatrix::operator ()( const int rowPosition,
                                                                       const int colPosition ) {
  return (*allRows [ rowPosition ])( colPosition );
}

//------------------------------------------------------------//

#endif // __Numerics_DenseRowMatrix_HH__
