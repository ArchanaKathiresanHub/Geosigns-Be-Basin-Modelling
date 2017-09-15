//------------------------------------------------------------//

#ifndef __Numerics__BLASFunctions_HH__
#define __Numerics__BLASFunctions_HH__

//------------------------------------------------------------//

//  #include <acml.h>

//------------------------------------------------------------//


// If the AMD core math library can be used then this can be removed and the above include statement be uncommented.
// Cut from here
extern "C" {
  void dger_(int *m, int *n, double *alpha, double *x, int *incx, double *y, int *incy, double *a, int *lda);
  void dgemv_(char *trans, int *m, int *n, double *alpha, double *a, int *lda, double *x, int *incx, double *beta, double *y, int *incy, int trans_len);
  void daxpy_(int *n, double *alpha, double *x, int *incx, double *y, int *incy);

  void dswap_(int *n, double *x, int *incx, double *y, int *incy);
  void dcopy_(int *n, double *x, int *incx, double *y, int *incy);
  void dscal_(int *n, double *alpha, double *x, int *incx);
  double ddot_(int *n, double *x, int *incx, double *y, int *incy);
 void daxpy_(int *n, double *alpha, double *x, int *incx, double *y, int *incy);

  void sger_(int *m, int *n, float *alpha, float *x, int *incx, float *y, int *incy, float *a, int *lda);
  void sgemv_(char *trans, int *m, int *n, float *alpha, float *a, int *lda, float *x, int *incx, float *beta, float *y, int *incy, int trans_len);
  void saxpy_(int *n, float *alpha, float *x, int *incx, float *y, int *incy);
  void sswap_(int *n, float *x, int *incx, float *y, int *incy);
  void scopy_(int *n, float *x, int *incx, float *y, int *incy);
  void sscal_(int *n, float *alpha, float *x, int *incx);
  float sdot_(int *n, float *x, int *incx, float *y, int *incy);
  void saxpy_(int *n, float *alpha, float *x, int *incx, float *y, int *incy);


}


inline void dger(int m, int n, double alpha, double *x, int incx, double *y, int incy, double *a, int lda) {
  dger_ ( &m, &n, &alpha, x, &incx, y, &incy, a, &lda );
}

inline void sger(int m, int n, float alpha, float *x, int incx, float *y, int incy, float *a, int lda) {
  sger_ ( &m, &n, &alpha, x, &incx, y, &incy, a, &lda );
}
// end of cut


/*! \defgroup BLAS BLAS
 *
 */

namespace BLAS {

  static double oneDbl  = 1.0;
  static double zeroDbl = 0.0;

  static float oneFlt  = 1.0;
  static float zeroFlt = 0.0;

  static int oneInt  = 1;



  /** @addtogroup BLAS
   *
   * @{
   */

  // Level 1

  /// \brief Swaps the values of two single precision vectors.
  ///
  /// Returns \f$ x \leftrightarrow y \f$.
  void swap ( const int    size, 
                    float* x,
                    float* y );

  /// \brief Swaps the values of two double precision vectors.
  ///
  /// Returns \f$ x \leftrightarrow y \f$.
  void swap ( const int     size, 
                    double* x,
                    double* y );

  /// \brief Copies the values of one single precision vector to another.
  ///
  /// Returns \f$ x = y \f$.
  void copy ( const int    size, 
              const float* x,
                    float* y );

  /// \brief Copies the values of one double precision vector to another.
  ///
  /// Returns \f$ x = y \f$.
  void copy ( const int     size, 
              const double* x,
                    double* y );

  /// \brief Scales a single precision vector by some scalar value.
  ///
  /// Returns \f$ y = \alpha \cdot y \f$.
  void scale ( const int    size, 
               const float  alpha,
                     float* y );

  /// \brief Scales a double precision vector by some scalar value.
  ///
  /// Returns \f$ y = \alpha \cdot y \f$.
  void scale ( const int     size, 
               const double  alpha,
                     double* y );

  /// \brief Computes the inner product of two single precision vectors.
  ///
  /// Returns \f$ \sum x_i \cdot y_i \f$.
  float dot ( const int    size,
              const float* x,
              const float* y );

  /// \brief Computes the inner product of two double precision vectors.
  ///
  /// Returns \f$ \sum x_i \cdot y_i \f$.
  double dot ( const int     size,
               const double* x,
               const double* y );

  /// \brief Adds a multiple of a single precision vector to another.
  ///
  /// Returns \f$ y = \alpha x + y\f$
  void axpy ( const int    size, 
              const float  alpha,
              const float* x,
                    float* y );

  /// \brief Adds a multiple of a double precision vector to another.
  ///
  /// Returns \f$ y = \alpha x + y\f$
  void axpy ( const int     size, 
              const double  alpha,
              const double* x,
                    double* y );


  // Level 2

  /// \brief Computes the matrix vector product using single precision matrix and vectors.
  ///
  /// Returns \f$ r = A \cdot x \f$.
  void mvp ( const bool   rowMajorOrder,
             const int    m,
             const int    n,
             const float* matrixBuffer,
             const float* vec,
                   float* result );

  /// \brief Computes the matrix vector product using double precision matrix and vectors.
  ///
  /// Returns \f$ r = A \cdot x \f$.
  void mvp ( const bool    rowMajorOrder,
             const int     m,
             const int     n,
             const double* matrixBuffer,
             const double* vec,
                   double* result );

  /// \brief Computes the outer product of a single precision vector, with itself, adding
  /// the result to a matrix.
  ///
  /// Returns \f$ A=A + \alpha x x^T \f$
  void addOuterProduct ( const int    n,
                         const float  alpha,
                         const float* vec,
                               float* matrix );

  /// \brief Computes the outer product of a double precision vector, with itself, adding
  /// the result to a matrix.
  ///
  /// Returns \f$ A=A + \alpha x x^T \f$
  void addOuterProduct ( const int     n,
                         const double  alpha,
                         const double* vec,
                               double* matrix );

  /// \brief Computes the outer product of two single precision vectors adding
  /// the result to a matrix.
  ///
  /// Returns \f$ A=A + \alpha x x^T \f$
  /// The parameter @param rowMajorOrder indicates whether the matrix storage scheme
  /// is row major order or not.
  void addOuterProduct ( const bool   rowMajorOrder,
                         const int    m,
                         const int    n,
                         const float  alpha,
                         const float* vec1,
                         const float* vec2,
                               float* matrix );

  /// \brief Computes the outer product of two double precision vectors adding
  /// the result to a matrix.
  ///
  /// Returns \f$ A=A + \alpha x x^T \f$
  /// The parameter @param rowMajorOrder indicates whether the matrix storage scheme
  /// is row major order or not.
  void addOuterProduct ( const bool    rowMajorOrder,
                         const int     m,
                         const int     n,
                         const double  alpha,
                         const double* vec1,
                         const double* vec2,
                               double* matrix );
  /** @} */

}

//------------------------------------------------------------//

// Inline functions

inline void BLAS::swap ( const int    size, 
                               float* x,
                               float* y ) {

  int size_ = size;

  sswap_ ( &size_, x, &oneInt, y, &oneInt );

}

//------------------------------------------------------------//

inline void BLAS::swap ( const int     size, 
                               double* x,
                               double* y ) {

  int size_ = size;

  dswap_ ( &size_, x, &oneInt, y, &oneInt );

}

//------------------------------------------------------------//

inline void BLAS::copy ( const int    size, 
                         const float* x,
                               float* y ) {

  int size_ = size;
  float* xCopy = const_cast<float*>( x );

  scopy_ ( &size_, xCopy, &oneInt, y, &oneInt );

}

//------------------------------------------------------------//

inline void BLAS::copy ( const int     size, 
                         const double* x,
                               double* y ) {

  int size_ = size;
  double* xCopy = const_cast<double*>( x );

  dcopy_ ( &size_, xCopy, &oneInt, y, &oneInt );

}

//------------------------------------------------------------//

inline void BLAS::scale ( const int    size, 
                          const float  alpha,
                                float* y ) {

  int size_ = size;
  float alpha_ = alpha;

  sscal_ ( &size_, &alpha_, y, &oneInt );

}

//------------------------------------------------------------//

inline void BLAS::scale ( const int     size, 
                          const double  alpha,
                                double* y ) {

  int size_ = size;
  double alpha_ = alpha;

  dscal_ ( &size_, &alpha_, y, &oneInt );

}

//------------------------------------------------------------//

inline float BLAS::dot ( const int    size, 
                         const float* x,
                         const float* y ) {

  int size_ = size;
  float* xCopy = const_cast<float*>(x);
  float* yCopy = const_cast<float*>(y);

  return sdot_ ( &size_, xCopy, &oneInt, yCopy, &oneInt );
}

//------------------------------------------------------------//

inline double BLAS::dot ( const int     size, 
                          const double* x,
                          const double* y ) {


  int size_ = size;

  double* xCopy = const_cast<double*>(x);
  double* yCopy = const_cast<double*>(y);

  return ddot_ ( &size_, xCopy, &oneInt, yCopy, &oneInt );
}

//------------------------------------------------------------//

inline void BLAS::axpy ( const int    size, 
                         const float  alpha,
                         const float* x,
                               float* y ) {

  int size_ = size;
  float alpha_ = alpha;
  float* xCopy = const_cast<float*>(x);

  saxpy_ ( &size_, &alpha_, xCopy, &oneInt, y, &oneInt );

}

//------------------------------------------------------------//

inline void BLAS::axpy ( const int     size, 
                         const double  alpha,
                         const double* x,
                               double* y ) {

  int size_ = size;
  double alpha_ = alpha;
  double* xCopy = const_cast<double*>(x);

  daxpy_ ( &size_, &alpha_, xCopy, &oneInt, y, &oneInt );

}

//------------------------------------------------------------//

inline void BLAS::mvp ( const bool   rowMajorOrder,
                        const int    m,
                        const int    n,
                        const float* matrixBuffer,
                        const float* vec,
                              float* result ) {

  static const char transposeCharacter [ 2 ] = { 'N', 'T' };

  float* matrixBufferTemp = const_cast<float*>( matrixBuffer );
  float* vectorBufferTemp = const_cast<float*>( vec );
  char   t = transposeCharacter [ rowMajorOrder ];
  int    rows = m;
  int    cols = n;

  sgemv_ ( &t, &rows, &cols,
          &oneFlt, matrixBufferTemp, &rows,
          vectorBufferTemp, &oneInt,
          &zeroFlt,
          result, &oneInt, 1 );

//    sgemv ( transposeCharacter [ rowMajorOrder ],
//            m, n,
//            1.0, matrixBufferTemp, m,
//            vectorBufferTemp, 1, 
//            0.0,
//            result, 1 );


}

//------------------------------------------------------------//

inline void BLAS::mvp ( const bool    rowMajorOrder,
                        const int     m,
                        const int     n,
                        const double* matrixBuffer,
                        const double* vec,
                              double* result ) {

  static const char transposeCharacter [ 2 ] = { 'N', 'T' };

  double* matrixBufferTemp = const_cast<double*>( matrixBuffer );
  double* vectorBufferTemp = const_cast<double*>( vec );
  char    t = transposeCharacter [ rowMajorOrder ];
  int     rows = m;
  int     cols = n;


//    dgemv_ ( &t, &cols, &rows,
//            &oneDbl, matrixBufferTemp, &rows,
//            vectorBufferTemp, &oneInt,
//            &zeroDbl,
//            result, &oneInt, 1 );

  dgemv_ ( &t, &rows, &cols,
          &oneDbl, matrixBufferTemp, &rows,
          vectorBufferTemp, &oneInt,
          &zeroDbl,
          result, &oneInt, 1 );


}

//------------------------------------------------------------//

inline void BLAS::addOuterProduct ( const int    n,
                                    const float  alpha,
                                    const float* vec,
                                          float* matrix ) {

  addOuterProduct ( false, n, n, alpha, vec, vec, matrix );
}

//------------------------------------------------------------//

inline void BLAS::addOuterProduct ( const int     n,
                                    const double  alpha,
                                    const double* vec,
                                          double* matrix ) {


  addOuterProduct ( false, n, n, alpha, vec, vec, matrix );
}

//------------------------------------------------------------//

inline void BLAS::addOuterProduct ( const bool   rowMajorOrder,
                                    const int    m,
                                    const int    n,
                                    const float  alpha,
                                    const float* vec1,
                                    const float* vec2,
                                          float* matrix ) {

  int   rows = m;
  int   cols = n;
  float alpha_ = alpha;

  float* vec1Copy = const_cast<float*>(vec1);
  float* vec2Copy = const_cast<float*>(vec2);


  if ( rowMajorOrder ) {
    sger_ ( &rows, &cols, &alpha_, vec2Copy, &oneInt, vec1Copy, &oneInt, matrix, &rows );
  } else {
    sger_ ( &cols, &rows, &alpha_, vec1Copy, &oneInt, vec2Copy, &oneInt, matrix, &cols );
  }

}

//------------------------------------------------------------//

inline void BLAS::addOuterProduct ( const bool    rowMajorOrder,
                                    const int     m,
                                    const int     n,
                                    const double  alpha,
                                    const double* vec1,
                                    const double* vec2,
                                          double* matrix ) {

  int    rows = m;
  int    cols = n;
  double alpha_ = alpha;

  double* vec1Copy = const_cast<double*>(vec1);
  double* vec2Copy = const_cast<double*>(vec2);

  if ( rowMajorOrder ) {
    dger_ ( &rows, &cols, &alpha_, vec2Copy, &oneInt, vec1Copy, &oneInt, matrix, &rows );
  } else {
    dger_ ( &cols, &rows, &alpha_, vec1Copy, &oneInt, vec2Copy, &oneInt, matrix, &cols );
  }

}

//------------------------------------------------------------//

#endif // __Numerics__BLASFunctions_HH__

//------------------------------------------------------------//
