#ifndef __Numerics_GenericFortranMatrixOperations_HH__
#define __Numerics_GenericFortranMatrixOperations_HH__

#include "Numerics.h"
#include "Vector.h"
#include "FortranMatrix.h"

namespace Numerics {

  
  /** @addtogroup LinearAlgebra
   *
   * @{
   */

  /// \file FortranMatrixOperations.h
  /// \brief Provides Blas-Lapack operations for the fortran matrix.

  /// \brief Factorise a general matrix.
  void luFactorise ( FortranMatrix& mat,
                     IntegerArray&  permutation );

  /// \brief Back solve a previously factortised matrix equation.
  void backSolve ( const FortranMatrix& mat,
                   const IntegerArray&  permutation,
                         Vector&        result );

  /// \brief Add the outer-product of two vectors to a matrix.
  void addOuterProduct ( FortranMatrix& mat,
                         const Vector&  vec1,
                         const Vector&  vec2 );


  /**
   * @}
   */

}




#endif // __Numerics_GenericFortranMatrixOperations_HH__
