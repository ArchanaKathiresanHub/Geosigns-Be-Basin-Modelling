#include "AlignedDenseMatrix.h"

#include <stdlib.h>

#include "SimdTraits.h"
#include "SimdInstruction.h"

Numerics::AlignedDenseMatrix::AlignedDenseMatrix ( const int m, const int n ) : m_rows ( m ), m_cols ( n ) {
   allocate ();
}

Numerics::AlignedDenseMatrix::AlignedDenseMatrix ( const AlignedDenseMatrix& mat ) : m_rows ( mat.rows ()), m_cols ( mat.cols ()) {
   allocate ();
   copy ( mat );
}

Numerics::AlignedDenseMatrix::~AlignedDenseMatrix  () {

   if ( m_values != 0 ) {
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

void Numerics::AlignedDenseMatrix::resize ( const AlignedDenseMatrix& mat ) {

   if ( m_values != 0 and m_rows == mat.m_rows and m_cols == mat.m_cols ) {
      // Matrix has same size already, so nothing to do.
      return;
   } else if ( m_values != 0 and m_rows != mat.m_rows and m_cols != mat.m_cols ) {
      SimdInstruction<CurrentSimdTechnology>::free ( m_values );
   }

   m_rows = mat.m_rows;
   m_cols = mat.m_cols;
   allocate ();
}

void Numerics::AlignedDenseMatrix::fill ( const double withTheValue ) {

   int i;
   int j;
   int count;

   for ( j = 0, count = 0; j < m_rows; ++j, count += m_leadingDimension - m_rows ) {

      for ( i = 0; i < m_cols; ++i, ++count ) {
         m_values [ count ] = withTheValue;
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
