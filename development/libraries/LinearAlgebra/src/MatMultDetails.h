#ifndef NUMERICS__MATMULT_DETAILS__H
#define NUMERICS__MATMULT_DETAILS__H

#include "AlignedDenseMatrix.h"
#include "SimdInstruction.h"

/// \file MatMultDetails.h
/// \brief Implementation of the four basic variants of the matrix-matrix product.
///
/// The four basic variants are:
///   - Matrix-Matrix product;
///   - Matrix-transpose(Matrix) product;
///   - transpose(Matrix)-Matrix product;
///   - transpose(Matrix)-transpose(Matrix) product.
///
/// There are four different function because of the different access patterns.
///

namespace Numerics {

   namespace details {

      /// \brief Instantiation of the SimdInstructions with the current simd technology definition.
      typedef SimdInstruction<CurrentSimdTechnology> SimdInstr;


      /// \brief Compute the Matrix-Matrix product.
      void matMatProd ( const double              alpha,
                        const AlignedDenseMatrix& a,
                        const AlignedDenseMatrix& b,
                        const double              beta,
                              AlignedDenseMatrix& c );

      /// \brief Compute the transpose(Matrix)-Matrix product.
      void matTransMatProd ( const double              alpha,
                             const AlignedDenseMatrix& a,
                             const AlignedDenseMatrix& b,
                             const double              beta,
                                   AlignedDenseMatrix& c );

      /// \brief Compute the Matrix-transpose(Matrix) product.
      void matMatTransProd ( const double              alpha,
                             const AlignedDenseMatrix& a,
                             const AlignedDenseMatrix& b,
                             const double              beta,
                                   AlignedDenseMatrix& c );

      /// \brief Compute the transpose(Matrix)-transpose(Matrix) product.
      void matTransMatTransProd ( const double              alpha,
                                  const AlignedDenseMatrix& a,
                                  const AlignedDenseMatrix& b,
                                  const double              beta,
                                        AlignedDenseMatrix& c );

   } // end namespace details

} // end namespace Numerics


#endif // NUMERICS__MATMULT_DETAILS__H
