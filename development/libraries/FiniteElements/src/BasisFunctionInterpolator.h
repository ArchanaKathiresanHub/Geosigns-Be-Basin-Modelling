//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef FINITE_ELEMENT_METHOD__BASIS_FUNCTION_INTERPOLATOR_H
#define FINITE_ELEMENT_METHOD__BASIS_FUNCTION_INTERPOLATOR_H

// Access to STL
#ifdef _WIN32
#include <array>
#else
#include <tr1/array>
#include <cmath>
#include <xmmintrin.h>
#include <immintrin.h>
#endif

#include "AlignedDenseMatrix.h"
#include "SimdInstruction.h"
#include "CpuInfo.h"

namespace FiniteElementMethod {

   /// \brief Interpolate all properties at all quadrature points.
   ///
   /// The calculation is broken down into three stages.
   /// 1. The bulk of the calcualtion is performed first, here the values are blocked into 8x4 and 4x4 blocks.
   /// 2. Compute values for columns that are not a multiple of 4.
   /// 3. Compute vlaues for rows that are not a multiple of 4.
   class BasisFunctionInterpolator {

   public :

      /// \brief Interpolate all properties at all quadrature points at the same time.
      void compute ( const Numerics::AlignedDenseMatrix& basisFunctionsTranspose,
                     const Numerics::AlignedDenseMatrix& propertyVectors,
                           Numerics::AlignedDenseMatrix& interpolatedProperties,
                     const cpuInfo& cpuInfo );

      /// \brief Interpolate the properties at all quadrature points at the same time using a simple scheme.
      ///
      /// This is primarily for the unit tests.
      void simpleInterpolate ( const Numerics::AlignedDenseMatrix& basisFunctionsTranspose,
                               const Numerics::AlignedDenseMatrix& propertyVectors,
                                     Numerics::AlignedDenseMatrix& interpolatedProperties );

   private :

#ifndef _WIN32

      /// \brief An array of four vectors of doubles.
      typedef std::tr1::array<__m256d, 4> FourByFour;

      /// \brief An array of two vectors of doubles.
      typedef std::tr1::array<__m256d, 2> FourByTwo;

      /// \brief Set all four vectors to zero
      void zero ( FourByFour& avx );

      /// \brief Set all two vectors to zero
      void zero ( FourByTwo& avx );

      /// \brief Set a single vector to zero
      void zero ( __m256d& avx );

      /// \brief Load four vectors.
      void loadContiguous ( const double* mat, const int lda, FourByFour& avx );

      /// \brief Load two vectors.
      void loadContiguous ( const double* mat, const int lda, FourByTwo& avx );

      /// \brief Load four vectors and transpose them.
      ///
      /// The 4x4 block is loaded into the 4 vectors and transposed.
      ///
      /// \f\left( \begin{array}{cccc}
      /// a & e & i & m\                             \
      /// b & f & j & n\                             \
      /// c & g & k & o\                             \
      /// d & h & l & p\end{array}
      /// \right) \f
      ///
      /// After transposing
      ///
      /// \f\left( \begin{array}{cccc}
      /// a & b & c & d\                             \
      /// e & f & g & h\                             \
      /// i & j & k & l\                             \
      /// m & n & o & p\end{array}
      /// \right) \f
      void loadContiguousTrans4x4 ( const double* mat, const int lda, FourByFour& avx );

      /// \brief Load two vectors.
      ///
      /// The 4x2 block is loaded into the 4 vectors and the transposed.
      ///
      /// \f\left( \begin{array}{cc}
      /// a & e\                             \
      /// b & f\                             \
      /// c & g\                             \
      /// d & h\end{array}
      /// \right) \f
      ///
      /// After transposing
      ///
      /// \f\left( \begin{array}{cccc}
      /// a & b & c & d\                             \
      /// e & f & g & h\                             \
      /// a & b & c & d\                             \
      /// e & f & g & h\end{array}
      /// \right) \f
      void loadContiguousTrans4x2 ( const double* mat, const int lda, FourByFour& avx );

      /// \brief Load a single vector.
      ///
      /// The 4x1 block is loaded into the 4 vectors.
      ///
      /// \f\left( \begin{array}{c}
      /// a\                             \
      /// b\                             \
      /// c\                             \
      /// d\end{array}
      /// \right) \f
      ///
      /// After transposing
      ///
      /// \f\left( \begin{array}{cccc}
      /// a & b & c & d\                             \
      /// a & b & c & d\                             \
      /// a & b & c & d\                             \
      /// a & b & c & d\end{array}
      /// \right) \f
      void loadBroadcast4x1 ( const double* mat, FourByFour& avx );

      /// \brief Write the four vectors to the matrix.
      ///
      /// Additional permutation operatations are required as the values are computed
      /// are not in the correct order.
      void store ( double* mat, const int lda, FourByFour& avx );

      /// \brief Write the two vectors to the matrix.
      ///
      /// Additional permutation operatations are required as the values are computed
      /// are not in the correct order.
      void store ( double* mat, const int lda, FourByTwo& avx );

      /// \brief Write a single vectors to the matrix.
      void store ( double* mat, __m256d avx );

      /// \brief Sum the product of the four vectors.
      ///
      /// \f$ c_i = \sum_j a_{i,j} * b_{i,j} \forall i \f$
      void product ( FourByFour& a, FourByFour& b, __m256d& c );

      /// \brief Compute the product of the 4x4 vectors.
      ///
      /// Consider each of the the four vectors to be a 4x4 matrix.
      /// This function computes the product of the 4x4 matrices.
      void product ( FourByFour& a, FourByFour& b, FourByFour& c );

      /// \brief Compute the product of the 2 sets of 4x4 vectors.
      ///
      /// Consider each of the the four vectors to be a 4x4 matrix.
      /// This function computes the product of the 4x4 matrices.
      /// \f$ c_1 = a_1 * b, c2 = a_2 * b \f$
      void product ( FourByFour& a1, FourByFour& a2, FourByFour& b, FourByFour& c1, FourByFour& c2 );

      /// \brief Compute the product of the 2 sets of 2x2 vectors.
      ///
      /// Consider the 4 vectors to store 2 2x2 matrices.
      ///
      /// \f\left( \begin{array}{cc}
      /// a & e\                             \
      /// b & f\                             \
      /// c & g\                             \
      /// d & h\end{array}
      /// \right) \f
      ///
      /// These vectors can be considered to be two 2x2 matrices.
      ///
      /// \f m1=\left( \begin{array}{cc}
      /// a & e\                             \
      /// b & f\end{array}
      /// \right) \f
      ///
      /// \f m2=\left( \begin{array}{cc}
      /// c & g\                             \
      /// d & h\end{array}
      /// \right) \f
      ///
      void product ( FourByFour& a, FourByFour& b, FourByTwo& c );

      /// \brief One step in the product of 4 vectors by 4 vectors.
      void product ( __m256d a, __m256d b, FourByFour& c );

      /// \brief One step in the product of 4 vectors by 2 vectors.
      void product ( __m256d a, __m256d b, FourByTwo& c );

      /// \brief One step in the product of 2 sets of 4 vectors by 4 vectors.
      void product ( __m256d a1, __m256d a2, __m256d b, FourByFour& c1, FourByFour& c2 );

      /// \brief Compute an inner product of two vectors.
      ///
      /// It is assumed that the number of values in each vector is the number of degrees of
      /// freedom in each element, which is 8.
      /// The first is a row of the 'a' matrix, values here will be separated by LDA values.
      /// The second vector is a column of b, values will be contiguous.
      double innerProduct ( const double* a, const int posA, const int LDA,
                            const double* b, const int posB );

      /// \brief Compute all contributions from the column in the range rows - rows mod 4 range + 1 to rows.
      void interpolatePropertiesRemainingRows ( const int NA, const int MA, const int MB,
                                                int rowsRemaining,
                                                const double* a, const int LDA,
                                                const double* b, const int LDB,
                                                      double* c, const int LDC );

      /// \brief Compute all contributions from the last column.
      ///
      /// Only called if the number of columns mod 4 = 1
      void interpolatePropertiesOneRemainingColumn ( const int rowBlocks,
                                                     const int rowBlocksRemaining,
                                                     const double* a, const int LDA,
                                                     const double* b, const int LDB,
                                                           double* c, const int LDC );

      /// \brief Compute all contributions from the last two columns.
      ///
      /// Only called if the number of columns mod 4 = 2
      void interpolatePropertiesTwoRemainingColumns ( const int rowBlocks,
                                                      const int rowBlocksRemaining,
                                                      const double*  a, const int LDA,
                                                      const double*& b, const int LDB,
                                                            double*& c, const int LDC );

      /// \brief Compute all contributions from the last three columns.
      ///
      /// Only called if the number of columns mod 4 = 3
      void interpolatePropertiesThreeRemainingColumns ( const int rowBlocks,
                                                        const int rowBlocksRemaining,
                                                        const double*  a, const int LDA,
                                                        const double*& b, const int LDB,
                                                              double*& c, const int LDC );

      /// \brief Compute all contributions from the column in the range columns - columns mod 4 range + 1 to columns.
      void interpolatePropertiesRemainingColumns ( const int NA, const int MA, const int MB,
                                                   const int rowBlocks,
                                                   const int rowBlocksRemaining,
                                                   const int colsRemaining,
                                                   const double*  a, const int LDA,
                                                   const double*& b, const int LDB,
                                                         double*& c, const int LDC );

      /// \brief Compute the main part of the interpolation.
      ///
      /// Tha is all values in the rows - rows mod 4 and columns - columns mod 4 range.
      void interpolatePropertiesMain ( const int NA, const int MA, const int MB,
                                       const int colBlocks,
                                       const int rowBlocks,
                                       const int rowBlocksRemaining,
                                       const double*  a, const int LDA,
                                       const double*& b, const int LDB,
                                             double*& c, const int LDC );

#ifdef INTERPOLATOR_USE_FMA
      Numerics::SimdInstruction<Numerics::AVXFMA> instructions;
#else
      Numerics::SimdInstruction<Numerics::AVX> instructions;
#endif
#endif
   };

}

#ifndef _WIN32

inline void FiniteElementMethod::BasisFunctionInterpolator::zero ( FourByFour& avx ) {
   // Set all values to be zero.
   avx [ 0 ] = _mm256_setzero_pd ();
   avx [ 1 ] = _mm256_setzero_pd ();
   avx [ 2 ] = _mm256_setzero_pd ();
   avx [ 3 ] = _mm256_setzero_pd ();
}

inline void FiniteElementMethod::BasisFunctionInterpolator::zero ( FourByTwo& avx ) {
   // Set all values to be zero.
   avx [ 0 ] = _mm256_setzero_pd ();
   avx [ 1 ] = _mm256_setzero_pd ();
}

inline void FiniteElementMethod::BasisFunctionInterpolator::zero ( __m256d& avx ) {
   // Set all values to be zero.
   avx = _mm256_setzero_pd ();
}

inline void FiniteElementMethod::BasisFunctionInterpolator::loadContiguous ( const double* mat, const int lda, FourByFour& avx ) {

   // Load 4 values from aligned memory, starting at address 'mat'
   avx [ 0 ] = _mm256_load_pd ( mat );
   // Load 4 values from aligned memory, starting at address 'mat + lda'
   avx [ 1 ] = _mm256_load_pd ( mat + lda );
   avx [ 2 ] = _mm256_load_pd ( mat + 2 * lda );
   avx [ 3 ] = _mm256_load_pd ( mat + 3 * lda );
}

inline void FiniteElementMethod::BasisFunctionInterpolator::loadContiguous ( const double* mat, const int lda, FourByTwo& avx ) {
   avx [ 0 ] = _mm256_load_pd ( mat );
   avx [ 1 ] = _mm256_load_pd ( mat + lda );
}

inline void FiniteElementMethod::BasisFunctionInterpolator::loadContiguousTrans4x4 ( const double* mat, const int lda, FourByFour& avx ) {

   FourByFour avxt;

   // Load the 4 vectors.
   loadContiguous ( mat, lda, avx );

   // Then transpose them
   // Let
   // u = (a,b,c,d)
   // v = (f,g,h,i)
   //
   // w = _mm256_shuffle_pd (u,v,0) = (a,f,c,h)
   avxt [ 0 ] = _mm256_shuffle_pd ( avx [ 0 ], avx [ 1 ], 0 );
   // w = _mm256_shuffle_pd (u,v,15) = (b,g,d,i)
   avxt [ 1 ] = _mm256_shuffle_pd ( avx [ 0 ], avx [ 1 ], 15 );
   avxt [ 2 ] = _mm256_shuffle_pd ( avx [ 2 ], avx [ 3 ], 0 );
   avxt [ 3 ] = _mm256_shuffle_pd ( avx [ 2 ], avx [ 3 ], 15 );

   // w = _mm256_permute2f128_pd ( u, v, 32 ) = (a,b,f,g)
   avx [ 0 ] =_mm256_permute2f128_pd ( avxt [ 0 ], avxt [ 2 ], 32 );
   avx [ 1 ] =_mm256_permute2f128_pd ( avxt [ 1 ], avxt [ 3 ], 32 );

   // w = _mm256_permute2f128_pd ( u, v, 49 ) = (c,d,h,i)
   avx [ 2 ] =_mm256_permute2f128_pd ( avxt [ 0 ], avxt [ 2 ], 49 );
   avx [ 3 ] =_mm256_permute2f128_pd ( avxt [ 1 ], avxt [ 3 ], 49 );
}

inline void FiniteElementMethod::BasisFunctionInterpolator::loadContiguousTrans4x2 ( const double* mat, const int lda, FourByFour& avx ) {

   __m256d a1;
   __m256d a2;

   avx [ 0 ] = _mm256_load_pd ( mat );
   avx [ 1 ] = _mm256_load_pd ( mat + lda );

   // u = (a,b,c,d)
   // v = (f,g,h,i)
   //
   // w = _mm256_unpacklo_pd (u,v) = (a,f,c,h)
   a1 = _mm256_unpacklo_pd ( avx [ 0 ], avx [ 1 ]);
   // w = _mm256_unpackhi_pd (u,v) = (b,g,d,i)
   a2 = _mm256_unpackhi_pd ( avx [ 0 ], avx [ 1 ]);

   // w = _mm256_permute2f128_pd (u,u,0) = (a,b,a,b)
   avx [ 0 ] = _mm256_permute2f128_pd ( a1, a1, 0 );
   avx [ 1 ] = _mm256_permute2f128_pd ( a2, a2, 0 );

   // w = _mm256_permute2f128_pd (u,u,1+16) = (c,d,c,d)
   avx [ 2 ] = _mm256_permute2f128_pd ( a1, a1, 1 + 16 );
   avx [ 3 ] = _mm256_permute2f128_pd ( a2, a2, 1 + 16 );
}

inline void FiniteElementMethod::BasisFunctionInterpolator::loadBroadcast4x1 ( const double* mat, FourByFour& avx ) {
   // v = (mat [0], mat [0], mat [0], mat [0])
   avx [ 0 ] = _mm256_broadcast_sd ( mat );
   // v = (mat [1], mat [1], mat [1], mat [1])
   avx [ 1 ] = _mm256_broadcast_sd ( mat + 1 );
   avx [ 2 ] = _mm256_broadcast_sd ( mat + 2 );
   avx [ 3 ] = _mm256_broadcast_sd ( mat + 3 );
}

inline void FiniteElementMethod::BasisFunctionInterpolator::store ( double* mat, const int lda, FourByFour& avx ) {

   // u = (a,b,c,d)
   // v = (f,g,h,i)
   //
   // w = _mm256_shuffle_pd (u,v,10) = (a,g,c,i)
   __m256d a1 = _mm256_shuffle_pd ( avx [ 0 ], avx [ 1 ], 10 );
   // w = _mm256_shuffle_pd (v,u,10) = (f,b,h,d)
   __m256d a2 = _mm256_shuffle_pd ( avx [ 1 ], avx [ 0 ], 10 );
   __m256d a3 = _mm256_shuffle_pd ( avx [ 3 ], avx [ 2 ], 10 );
   __m256d a4 = _mm256_shuffle_pd ( avx [ 2 ], avx [ 3 ], 10 );

   // w = _mm256_permute2f128_pd (u,v,48) = (a,b,h,i)
  avx [ 0 ] = _mm256_permute2f128_pd ( a1, a3, 48 );
   // w = _mm256_permute2f128_pd (u,v,18) = (f,g,c,d)
  avx [ 2 ] = _mm256_permute2f128_pd ( a1, a3, 18 );
  avx [ 1 ] = _mm256_permute2f128_pd ( a2, a4, 48 );
  avx [ 3 ] = _mm256_permute2f128_pd ( a2, a4, 18 );

  _mm256_store_pd ( mat, avx [ 0 ] );
  _mm256_store_pd ( mat + lda, avx [ 1 ] );
  _mm256_store_pd ( mat + 2 * lda, avx [ 2 ] );
  _mm256_store_pd ( mat + 3 * lda, avx [ 3 ] );

}

inline void FiniteElementMethod::BasisFunctionInterpolator::store ( double* mat, const int lda, FourByTwo& avx ) {

   // u = (a,b,c,d)
   // v = (f,g,h,i)
   //
   // w = _mm256_blend_pd (u,v,10) = (a,g,c,d)
  __m256d a1 = _mm256_blend_pd ( avx [ 0 ], avx [ 1 ], 10 );
   // w = _mm256_blend_pd (u,v,5) = (f,b,h,d)
  __m256d a2 = _mm256_blend_pd ( avx [ 0 ], avx [ 1 ],  5 );

  _mm256_store_pd ( mat, a1 );
  _mm256_store_pd ( mat + lda, a2 );

}

inline void FiniteElementMethod::BasisFunctionInterpolator::store ( double* mat,  __m256d avx ) {
   _mm256_store_pd ( mat, avx );
}

inline void FiniteElementMethod::BasisFunctionInterpolator::product ( __m256d a1, __m256d a2, __m256d b, FourByFour& c1, FourByFour& c2 ) {

   //b = (a,b,c,d)
   c1 [ 0 ] = instructions.mulAdd ( a1, b, c1 [ 0 ]);
   c2 [ 0 ] = instructions.mulAdd ( a2, b, c2 [ 0 ]);

   // Permute b vector
   // (a,b,c,d)->(b,a,d,c)
   b = _mm256_permute_pd ( b, 5 );
   c1 [ 1 ] = instructions.mulAdd ( a1, b, c1 [ 1 ]);
   c2 [ 1 ] = instructions.mulAdd ( a2, b, c2 [ 1 ]);

   // Permute b vector again
   // (b,a,d,c)->(d,c,b,a)
   b = _mm256_permute2f128_pd ( b, b, 1 );
   c1 [ 2 ] = instructions.mulAdd ( a1, b, c1 [ 2 ]);
   c2 [ 2 ] = instructions.mulAdd ( a2, b, c2 [ 2 ]);

   // Permute b vector again
   // (d,c,b,a)->(c,d,a,b)
   b = _mm256_permute_pd ( b, 5 );
   c1 [ 3 ] = instructions.mulAdd ( a1, b, c1 [ 3 ]);
   c2 [ 3 ] = instructions.mulAdd ( a2, b, c2 [ 3 ]);

}

inline void FiniteElementMethod::BasisFunctionInterpolator::product ( __m256d a, __m256d b, FourByFour& c ) {

   // c = a * b + c
   c [ 0 ] = instructions.mulAdd ( a, b, c [ 0 ]);

   b = _mm256_permute_pd ( b, 5 );
   c [ 1 ] = instructions.mulAdd ( a, b, c [ 1 ]);

   b = _mm256_permute2f128_pd ( b, b, 1 );
   c [ 2 ] = instructions.mulAdd ( a, b, c [ 2 ]);

   b = _mm256_permute_pd ( b, 5 );
   c [ 3 ] = instructions.mulAdd ( a, b, c [ 3 ]);

}

inline void FiniteElementMethod::BasisFunctionInterpolator::product ( __m256d a, __m256d b, FourByTwo& c ) {
   c [ 0 ] = instructions.mulAdd ( a, b, c [ 0 ]);
   b = _mm256_permute_pd ( b, 5 );
   c [ 1 ] = instructions.mulAdd ( a, b, c [ 1 ]);
}

inline void FiniteElementMethod::BasisFunctionInterpolator::product ( FourByFour& a, FourByFour& b, __m256d& c ) {
   c = instructions.mulAdd ( a [ 0 ], b [ 0 ], c );
   c = instructions.mulAdd ( a [ 1 ], b [ 1 ], c );
   c = instructions.mulAdd ( a [ 2 ], b [ 2 ], c );
   c = instructions.mulAdd ( a [ 3 ], b [ 3 ], c );
}

inline void FiniteElementMethod::BasisFunctionInterpolator::product ( FourByFour& a1, FourByFour& a2, FourByFour& b, FourByFour& c1, FourByFour& c2 ) {
   product ( a1 [ 0 ], a2 [ 0 ], b [ 0 ], c1, c2 );
   product ( a1 [ 1 ], a2 [ 1 ], b [ 1 ], c1, c2 );
   product ( a1 [ 2 ], a2 [ 2 ], b [ 2 ], c1, c2 );
   product ( a1 [ 3 ], a2 [ 3 ], b [ 3 ], c1, c2 );
}

inline void FiniteElementMethod::BasisFunctionInterpolator::product ( FourByFour& a, FourByFour& b, FourByFour& c ) {
   product ( a [ 0 ], b [ 0 ], c );
   product ( a [ 1 ], b [ 1 ], c );
   product ( a [ 2 ], b [ 2 ], c );
   product ( a [ 3 ], b [ 3 ], c );
}

inline void FiniteElementMethod::BasisFunctionInterpolator::product ( FourByFour& a, FourByFour& b, FourByTwo& c ) {
   product ( a [ 0 ], b [ 0 ], c );
   product ( a [ 1 ], b [ 1 ], c );
   product ( a [ 2 ], b [ 2 ], c );
   product ( a [ 3 ], b [ 3 ], c );
}

#endif

#endif // FINITE_ELEMENT_METHOD__BASIS_FUNCTION_INTERPOLATOR_H
