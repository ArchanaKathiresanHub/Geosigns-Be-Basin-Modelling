#ifndef __Numerics__LAPACKFunctions_HH__
#define __Numerics__LAPACKFunctions_HH__

//#include <acml.h>

/*! \defgroup LAPACK LAPACK
 * \ingroup LinearAlgebra
 *
 */

#include "BLASFunctions.h"

// If the AMD core math library can be used then this can be removed and the above include statement be uncommented.
// Cut from here
extern "C" {
  void dgetrf_(int *m, int *n, double *a, int *lda, int *ipiv, int *info);
  void sgetrf_(int *m, int *n, float *a, int *lda, int *ipiv, int *info);
  void dgetrs_(char *trans, int *n, int *nrhs, double *a, int *lda, int *ipiv, double *b, int *ldb, int *info, int trans_len);
  void sgetrs_(char *trans, int *n, int *nrhs, float *a, int *lda, int *ipiv, float *b, int *ldb, int *info, int trans_len);

}

void dgetrf (int m, int n, double *a, int lda, int *ipiv, int* info) {

  dgetrf_ ( &m, &n, a, &lda, ipiv, info );
}

void sgetrf (int m, int n, float *a, int lda, int *ipiv, int* info ) {
  sgetrf_ ( &m, &n, a, &lda, ipiv, info );
}

void dgetrs (char trans, int n, int /*nrhs*/, double *a, int lda, int *ipiv, double *b, int ldb, int* info ) {

  int trans_len = 1;
  int one = 1;

  dgetrs_ ( &trans, &n, &one, a, &lda, ipiv, b, &ldb, info, trans_len );

}

void sgetrs (char trans, int n, int /*nrhs*/, float *a, int lda, int *ipiv, float *b, int ldb, int *info ) {

  int trans_len = 1;
  int one = 1;

  sgetrs_ ( &trans, &n, &one, a, &lda, ipiv, b, &ldb, info, trans_len );

}
// end of cut

namespace LAPACK {

  /** @addtogroup LAPACK
   *
   * @{
   */


  /// \namespace LAPACK
  /// \brief Provides limited access to the LAPACK library.

  /// \brief Factorise a general double precision matrix.
  void luFactorise ( const int dimension,
                     double*   mat,
                     int*      permutation );

  /// \brief Factorise a general single precision matrix.
  void luFactorise ( const int dimension,
                     float*    mat,
                     int*      permutation );

  /// \brief Back solve a previously factorised double precision matrix equation.
  void backSolve ( const int     dimension,
                   const double* mat,
                   const int*    permutation,
                         double* result );

  /// \brief Back solve a previously factorised single precision matrix equation.
  void backSolve ( const int     dimension,
                   const float*  mat,
                   const int*    permutation,
                         float*  result );

  /// \brief Add the outer-product of two double precision vectors to a double precision matrix.
  void addOuterProduct ( const int     dim1,
                         const int     dim2,
                         const double* vec1,
                         const double* vec2,
                               double* mat );

  /// \brief Add the outer-product of two single precision vectors to a single precision matrix.
  void addOuterProduct ( const int    dim1,
                         const int    dim2,
                         const float* vec1,
                         const float* vec2,
                               float* mat );

  /**
   * @}
   */


}

//------------------------------------------------------------//
//
// Inline functions.
//

void LAPACK::luFactorise ( const int     dimension,
                                 double* mat,
                                 int*    permutation ) {

  int error;

  dgetrf ( dimension, dimension, mat, dimension, permutation, &error );

  if ( error != 0 ) {
    /// error
  }

}

//------------------------------------------------------------//

void LAPACK::luFactorise ( const int    dimension,
                                 float* mat,
                                 int*   permutation  ) {

  int error;

  sgetrf ( dimension, dimension, mat, dimension, permutation, &error );

  if ( error != 0 ) {
    /// error
  }

}

//------------------------------------------------------------//

void LAPACK::backSolve (  const int     dimension,
                          const double* mat,
                          const int*    permutation,
                                double* result ) {


  int error;
  double* matrixCopy = const_cast<double*>(mat);
  int*    permutationCopy = const_cast<int*>(&permutation[0]);

  dgetrs ( 'N', dimension, 1, matrixCopy, dimension, permutationCopy, result, dimension, &error );

}

//------------------------------------------------------------//

void LAPACK::backSolve ( const int    dimension,
                         const float* mat,
                         const int*   permutation,
                                float* result ) {


  int error;
  float* matrixCopy = const_cast<float*>(mat);
  int*   permutationCopy = const_cast<int*>(&permutation[ 0 ]);

  sgetrs ( 'N', dimension, 1, matrixCopy, dimension, permutationCopy, result, dimension, &error );

}

//------------------------------------------------------------//

void LAPACK::addOuterProduct ( const int    dim1,
                               const int    dim2,
                               const float* vec1,
                               const float* vec2,
                                     float* mat ) {

  float* v1Buffer = const_cast<float*>( vec1 );
  float* v2Buffer = const_cast<float*>( vec2 );

  sger ( dim1, dim2, 1.0, v1Buffer, 1, v2Buffer, 1, mat, dim1 );

}

//------------------------------------------------------------//

void LAPACK::addOuterProduct ( const int     dim1,
                               const int     dim2,
                               const double* vec1,
                               const double* vec2,
                                     double* mat ) {

  double* v1Buffer = const_cast<double*>( vec1 );
  double* v2Buffer = const_cast<double*>( vec2 );

  dger ( dim1, dim2, 1.0, v1Buffer, 1, v2Buffer, 1, mat, dim1 );

}

//------------------------------------------------------------//

#endif // __Numerics__LAPACKFunctions_HH__
