#include "AlignedDenseMatrix.h"

#include <stdlib.h>

#include "SimdTraits.h"
#include "SimdInstruction.h"
#include "MatMultDetails.h"

#ifdef USE_BLAS_LIBRARY
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern void dgemm_(char *transa, char *transb, int *m, int *n, int *k, double *alpha, const double *a, int *lda, const double *b, int *ldb, double *beta, double *c, int *ldc);

#ifdef __cplusplus
}
#endif /* __cplusplus */



inline void dgemm(char transa, char transb, int m, int n, int k, double alpha, const double *a, int lda, const double *b, int ldb, double beta, double *c, int ldc) {
  dgemm_ ( &transa, &transb, &m, &n, &k, &alpha, a, &lda, b, &ldb, &beta, c, &ldc);
}
#endif


Numerics::AlignedDenseMatrix::AlignedDenseMatrix () : m_rows ( 0 ), m_cols ( 0 ) {
   allocate ();
}


Numerics::AlignedDenseMatrix::AlignedDenseMatrix ( const int m, const int n ) : m_rows ( m ), m_cols ( n ) {
   allocate ();
}

Numerics::AlignedDenseMatrix::AlignedDenseMatrix ( const AlignedDenseMatrix& mat ) : m_rows ( mat.rows ()), m_cols ( mat.cols ()) {
   allocate ();
   copy ( mat );
}

Numerics::AlignedDenseMatrix::~AlignedDenseMatrix  () {

   if ( m_values != nullptr ) {
      SimdInstruction<CurrentSimdTechnology>::free ( m_values );
   }

}

Numerics::AlignedDenseMatrix& Numerics::AlignedDenseMatrix::operator= ( const AlignedDenseMatrix& mat ) {

   if ( this == &mat ) {
      return *this;
   }

   if ( m_rows != mat.rows () or m_cols != mat.cols ()) {
      resize ( mat );
   }

   copy ( mat );
   return *this;
}

void Numerics::AlignedDenseMatrix::copy ( const AlignedDenseMatrix& mat ) {

   int i;

   for ( i = 0; i < m_leadingDimension * m_cols; ++i ) {
      m_values [ i ] = mat.m_values [ i ];
   }

}

void Numerics::AlignedDenseMatrix::allocate () {

   const int Alignment = SimdTraits<CurrentSimdTechnology>::Alignment;

   if ( Alignment != 0 and m_rows % ( Alignment / sizeof ( double )) != 0 ) {
      m_leadingDimension = m_rows + Alignment / sizeof ( double ) - m_rows % ( Alignment / sizeof ( double ));
   } else {
      m_leadingDimension = m_rows;
   }

   m_values = SimdInstruction<CurrentSimdTechnology>::allocate ( m_leadingDimension * m_cols );
}

void Numerics::AlignedDenseMatrix::resize ( const int m, const int n ) {

   if ( m_values != nullptr and m_rows == m and m_cols == n ) {
      // Matrix has same size already, so nothing to do.
      return;
   } else if ( m_values != nullptr ) {
      SimdInstruction<CurrentSimdTechnology>::free ( m_values );
   }

   m_rows = m;
   m_cols = n;
   allocate ();
}

void Numerics::AlignedDenseMatrix::resize ( const AlignedDenseMatrix& mat ) {
   resize( mat.m_rows, mat.m_cols );
}

void Numerics::AlignedDenseMatrix::fill ( const double withTheValue ) {

   int i;
   int j;

   // Fill the usable part of the matrix with the value.
   for ( j = 0; j < cols (); ++j ) {

      for ( i = 0; i < rows (); ++i ) {
         operator ()( i, j ) = withTheValue;
      }

   }

   // Now fill the unused part with zeros.
   // The used part is used in order to get correct alignment of columns.
   for ( j = 0; j < cols (); ++j ) {

      for ( i = rows (); i < leadingDimension (); ++i ) {
         operator ()( i, j ) = 0.0;
      }

   }

}

void Numerics::AlignedDenseMatrix::print ( const std::string& name,
                                           std::ostream& o ) const {

   int i;
   int j;

   for ( i = 0; i < rows (); ++i ) {

      for ( j = 0; j < cols (); ++j ) {
         o << name << " ( "  << i + 1 << ", " << j + 1 << " ) = " << operator ()( i, j ) << ";" << std::endl;
      }

   }

}

void Numerics::transpose ( const AlignedDenseMatrix& mat,
                                 AlignedDenseMatrix& transpose ) {

   if ( mat.rows () != transpose.cols () or
        mat.cols () != transpose.rows ()) {
      // Error.
   }

   transpose.fill ( 0.0 );

   for ( int i = 0; i < mat.rows (); ++i ) {

      for ( int j = 0; j < mat.cols (); ++j ) {
         transpose ( j, i ) = mat ( i, j );
      }

   }

}


void Numerics::matmult ( const MatrixTransposeType transposeA,
                         const MatrixTransposeType transposeB,
                         const double              alpha,
                         const AlignedDenseMatrix& a,
                         const AlignedDenseMatrix& b,
                         const double              beta,
                               AlignedDenseMatrix& c ) {

#ifdef USE_BLAS_LIBRARY
   static const char transChar [ 2 ] = {'N', 'T'};

   int m;
   int n;
   int k;

   if ( transposeA == NO_TRANSPOSE ) {
      m = a.rows ();
      k = a.cols ();
   } else {
      m = a.cols ();
      k = a.rows ();
   }

   if ( transposeB == NO_TRANSPOSE ) {
      n = b.cols ();
   } else {
      n = b.rows ();
   }

   dgemm ( transChar [ transposeA ], transChar [ transposeB ],
           m, n, k, alpha,
           a.data (), a.leadingDimension (),
           b.data (), b.leadingDimension (),
           beta, c.data (), c.leadingDimension ());

#else
   if ( transposeA == NO_TRANSPOSE and transposeB == NO_TRANSPOSE ) {
      details::matMatProd ( alpha, a, b, beta, c );
   } else if ( transposeA == NO_TRANSPOSE and transposeB == TRANSPOSE ) {
      details::matMatTransProd ( alpha, a, b, beta, c );
   } else if ( transposeA == TRANSPOSE and transposeB == NO_TRANSPOSE ) {
      details::matTransMatProd ( alpha, a, b, beta, c );
   } else { // if ( transposeA == TRANSPOSE and transposeB == TRANSPOSE ) {
      details::matTransMatTransProd ( alpha, a, b, beta, c );
   }
#endif

}
