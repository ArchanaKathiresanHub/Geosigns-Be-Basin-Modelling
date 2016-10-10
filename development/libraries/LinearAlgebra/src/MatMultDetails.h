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
      template <Numerics::SimdInstructionTechnology simdTechnology>
      struct MatDetails
      {

         typedef Numerics::SimdInstruction<simdTechnology> SimdInstr;

         /// \brief Compute the Matrix-Matrix product.
         static void matMatProd( const double              alpha,
            const AlignedDenseMatrix& a,
            const AlignedDenseMatrix& b,
            const double              beta,
            AlignedDenseMatrix& c );

         /// \brief Compute the transpose(Matrix)-Matrix product.
         static void matTransMatProd( const double              alpha,
            const AlignedDenseMatrix& a,
            const AlignedDenseMatrix& b,
            const double              beta,
            AlignedDenseMatrix& c );

         /// \brief Compute the Matrix-transpose(Matrix) product.
         static void matMatTransProd( const double              alpha,
            const AlignedDenseMatrix& a,
            const AlignedDenseMatrix& b,
            const double              beta,
            AlignedDenseMatrix& c );

         /// \brief Compute the transpose(Matrix)-transpose(Matrix) product.
         static void matTransMatTransProd( const double              alpha,
            const AlignedDenseMatrix& a,
            const AlignedDenseMatrix& b,
            const double              beta,
            AlignedDenseMatrix& c );
      };

   } // end namespace details

} // end namespace Numerics

template < Numerics::SimdInstructionTechnology simdTechnology>
void Numerics::details::MatDetails<simdTechnology>::matMatProd( 
   const double              alpha,
   const AlignedDenseMatrix& a,
   const AlignedDenseMatrix& b,
   const double              beta,
   AlignedDenseMatrix& c )
{

   int i;
   int j;
   int k;

   typename SimdInstr::PackedDouble* ad128 = ( typename  SimdInstr::PackedDouble* )( a.data( ) );
   typename SimdInstr::PackedDouble* cd128 = ( typename  SimdInstr::PackedDouble* )( c.data( ) );

   const double* bd = b.data( );

   typename SimdInstr::PackedDouble zero = SimdInstr::set1( 0.0 );
   typename SimdInstr::PackedDouble alpha128 = SimdInstr::set1( alpha );
   typename SimdInstr::PackedDouble beta128 = SimdInstr::set1( beta );

   int amod = a.rows( ) % SimdInstr::DoubleStride;
   int asize = ( a.rows( ) - amod ) / SimdInstr::DoubleStride;

   int aColStride = a.leadingDimension( ) / SimdInstr::DoubleStride;
   int bColStride = b.leadingDimension( );
   int cColStride = c.leadingDimension( ) / SimdInstr::DoubleStride;


   bd = b.data( );

   cd128 = ( typename SimdInstr::PackedDouble* )( c.data( ) );

   for ( j = 0; j < c.cols( ); ++j, bd += bColStride, cd128 += cColStride ) {
      ad128 = ( typename SimdInstr::PackedDouble* )( a.data( ) );

      if ( beta == 0.0 ) {

         // Zero column in result matrix before adding to it.
#ifdef __INTEL_COMPILER
#pragma ivdep
#endif
         for ( i = 0; i < cColStride; i += 1 ) {
            cd128[i] = zero;
         }

         if ( amod > 0 ) {
            cd128[asize] = zero;
         }

      }
      else {

#ifdef __INTEL_COMPILER
#pragma ivdep
#endif
         for ( i = 0; i < cColStride; i += 1 ) {
            cd128[i] = SimdInstr::mul( cd128[i], beta128 );
         }

         if ( amod > 0 ) {
            cd128[asize] = SimdInstr::mul( cd128[i], beta128 );
         }

      }

      for ( k = 0; k < a.cols( ); ++k, ad128 += aColStride ) {
         typename SimdInstr::PackedDouble bVal128 = SimdInstr::mul( alpha128, SimdInstr::set1( bd[k] ) );

#ifdef __INTEL_COMPILER
#pragma ivdep
#endif
         for ( i = 0; i < asize; ++i ) {
            cd128[i] = SimdInstr::mulAdd( ad128[i], bVal128, cd128[i] );
         }

         if ( amod > 0 ) {
            cd128[asize] = SimdInstr::add( cd128[asize], SimdInstr::mul( ad128[asize], bVal128 ) );
         }

      }

   }

}

template <Numerics::SimdInstructionTechnology simdTechnology>
void Numerics::details::MatDetails<simdTechnology>::matTransMatProd( 
   const double              alpha,
   const AlignedDenseMatrix& a,
   const AlignedDenseMatrix& b,
   const double              beta,
   AlignedDenseMatrix& c ) {

   double ip1;
   double ip2;
   int i;
   int j;
   int k;
   int colsMod2 = c.cols( ) % 2;

   for ( i = 0; i < a.cols( ); ++i ) {

      for ( j = 0; j < c.cols( ) - colsMod2; j += 2 ) {
         ip1 = 0.0;
         ip2 = 0.0;

#ifdef __INTEL_COMPILER
#pragma ivdep
#endif
         for ( k = 0; k < a.rows( ); ++k ) {
            ip1 += a( k, i ) * b( k, j );
            ip2 += a( k, i ) * b( k, j + 1 );
         }

         c( i, j ) = alpha * ip1 + beta * c( i, j );
         c( i, j + 1 ) = alpha * ip2 + beta * c( i, j + 1 );
      }

      if ( colsMod2 == 1 ) {
         ip1 = 0.0;

#ifdef __INTEL_COMPILER
#pragma ivdep
#endif
         for ( k = 0; k < a.rows( ); ++k ) {
            ip1 += a( k, i ) * b( k, c.cols( ) - 1 );
         }

         c( i, c.cols( ) - 1 ) = alpha * ip1 + beta * c( i, c.cols( ) - 1 );
      }

   }

}


template <Numerics::SimdInstructionTechnology simdTechnology>
void Numerics::details::MatDetails<simdTechnology>::matMatTransProd( 
   const double              alpha,
   const AlignedDenseMatrix& a,
   const AlignedDenseMatrix& b,
   const double              beta,
   AlignedDenseMatrix& c ) {

   int i;
   int j;
   int k;

   typename SimdInstr::PackedDouble* ad128 = ( typename SimdInstr::PackedDouble* )( a.data( ) );
   typename SimdInstr::PackedDouble* cd128 = ( typename SimdInstr::PackedDouble* )( c.data( ) );

   const double* bd = b.data( );

   typename SimdInstr::PackedDouble zero = SimdInstr::set1( 0.0 );
   typename SimdInstr::PackedDouble alpha128 = SimdInstr::set1( alpha );
   typename SimdInstr::PackedDouble beta128 = SimdInstr::set1( beta );

   int amod = a.rows( ) % SimdInstr::DoubleStride;
   int asize = ( a.rows( ) - amod ) / SimdInstr::DoubleStride;

   int aColStride = a.leadingDimension( ) / SimdInstr::DoubleStride;
   int bColStride = b.leadingDimension( );
   int cColStride = c.leadingDimension( ) / SimdInstr::DoubleStride;

   int bPos;

   bd = b.data( );

   cd128 = ( typename SimdInstr::PackedDouble* )( c.data( ) );

   for ( j = 0; j < c.cols( ); ++j, bd += 1, cd128 += cColStride ) {
      ad128 = ( typename SimdInstr::PackedDouble* )( a.data( ) );

      if ( beta == 0 ) {

#ifdef __INTEL_COMPILER
#pragma ivdep
#endif
         // Zero column in result matrix before adding to it.
         for ( i = 0; i < cColStride; ++i ) {
            cd128[i] = zero;
         }

      }
      else {

#ifdef __INTEL_COMPILER
#pragma ivdep
#endif
         for ( i = 0; i < cColStride; ++i ) {
            cd128[i] = SimdInstr::mul( cd128[i], beta128 );
         }

      }

      for ( k = 0, bPos = 0; k < a.cols( ); ++k, ad128 += aColStride, bPos += bColStride ) {
         typename SimdInstr::PackedDouble bVal128 = SimdInstr::mul( alpha128, SimdInstr::set1( bd[bPos] ) );

#ifdef __INTEL_COMPILER
#pragma ivdep
#endif
         for ( i = 0; i < asize; ++i ) {
            cd128[i] = SimdInstr::mulAdd( ad128[i], bVal128, cd128[i] );
         }

         if ( amod > 0 ) {
            cd128[asize] = SimdInstr::add( cd128[asize], SimdInstr::mul( ad128[asize], bVal128 ) );
         }

      }

   }

}


template < Numerics::SimdInstructionTechnology simdTechnology >
void Numerics::details::MatDetails<simdTechnology>::matTransMatTransProd( 
   const double              alpha,
   const AlignedDenseMatrix& a,
   const AlignedDenseMatrix& b,
   const double              beta,
   AlignedDenseMatrix& c ) {

   int i;
   int j;
   int k;
   int cColStride = c.leadingDimension( );

   double* cd = c.data( );

   for ( j = 0; j < c.cols( ); ++j, cd += cColStride ) {

      if ( beta == 0 ) {

#ifdef __INTEL_COMPILER
#pragma ivdep
#endif
         for ( i = 0; i < c.rows( ); ++i ) {
            cd[i] = 0.0;
         }

      }
      else {

#ifdef __INTEL_COMPILER
#pragma ivdep
#endif
         for ( i = 0; i < c.rows( ); ++i ) {
            cd[i] *= beta;
         }

      }

      for ( k = 0; k < a.rows( ); ++k ) {
         double bjk = alpha * b( j, k );

#ifdef __INTEL_COMPILER
#pragma ivdep
#endif
         for ( i = 0; i < a.cols( ); ++i ) {
            cd[i] += a( k, i ) * bjk;
         }

      }

   }

}


#endif // NUMERICS__MATMULT_DETAILS__H
