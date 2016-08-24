#ifndef NUMERICS__SIMD_TRAITS__H
#define NUMERICS__SIMD_TRAITS__H

#ifdef __SSE2__
#include <xmmintrin.h>
#endif

#ifdef __AVX__
#include <immintrin.h>
#endif

namespace Numerics {

   /// \brief Enumeration of the instruction technology to be used.
   ///
   /// Here SSE means the sse2 update that includes double precision floating point arithmetic.
   ///
   /// Possible future technology additions include:
   ///    - AVX512   This extends the size of the AVX register to 512 bits, thus 8 doubles.
   ///
   enum SimdInstructionTechnology { NO_SIMD, SSE, AVX, AVXFMA };


   /// \brief Traits class for alignment and number of double packed into the packed-double.
   ///
   /// If no specialisation exists then NO_SIMD instructions will be used.
   template< const SimdInstructionTechnology SimdTechnology >
   struct SimdTraits {

      /// \brief Indicate which SIMD instruction technology is being used.
      static const SimdInstructionTechnology SimdInstructionUsed = NO_SIMD;

      /// \brief A single double.
      typedef double PackedDouble;

      /// \brief Alignment on the size of a double.
      static const int Alignment = sizeof ( PackedDouble );

      /// \brief The number of doubles contained in the PackedDouble.
      static const int DoubleStride = 1;

   };

#ifdef __SSE2__
   /// \brief Specialisation of SimdTraits for SSE instructions.
   template<>
   struct SimdTraits<SSE> {

      /// \brief Indicate which SIMD instruction technology is being used.
      static const SimdInstructionTechnology SimdInstructionUsed = SSE;

      /// \brief A pair of double values packed into the data type.
      typedef __m128d PackedDouble;

      /// \brief Alignment on the size of a SSE data type.
      static const int Alignment = sizeof ( PackedDouble );

      /// \brief The number of doubles contained in the PackedDouble.
      ///
      /// For SSE this will be 2 doubles.
      static const int DoubleStride = Alignment / sizeof ( double );

   };
#endif


#ifdef __AVX__
   /// \brief Specialisation of SimdTraits for AVX instructions.
   template<>
   struct SimdTraits<AVX> {

      /// \brief Indicate which SIMD instruction technology is being used.
      static const SimdInstructionTechnology SimdInstructionUsed = AVX;

      /// \brief Four double values packed into the data type.
      typedef __m256d PackedDouble;

      /// \brief Alignment on the size of a AVX data type.
      static const int Alignment = sizeof ( PackedDouble );

      /// \brief The number of doubles contained in the PackedDouble.
      ///
      /// For AVX this will be 4 doubles.
      static const int DoubleStride = Alignment / sizeof ( double );

   };

   /// \brief Specialisation of SimdTraits for AVX instructions.
   template<>
   struct SimdTraits<AVXFMA> {

      /// \brief Indicate which SIMD instruction technology is being used.
      static const SimdInstructionTechnology SimdInstructionUsed = AVXFMA;

      /// \brief Four double values packed into the data type.
      typedef __m256d PackedDouble;

      /// \brief Alignment on the size of a AVX data type.
      static const int Alignment = sizeof ( PackedDouble );

      /// \brief The number of doubles contained in the PackedDouble.
      ///
      /// For AVX this will be 4 doubles.
      static const int DoubleStride = Alignment / sizeof ( double );

   };
#endif

#ifdef _WIN32
   /// \brief The maximum simd instruction capability.
   static const SimdInstructionTechnology CurrentSimdTechnology = NO_SIMD;
#else
   /// \brief The maximum simd instruction capability.
   static const SimdInstructionTechnology CurrentSimdTechnology = AVX;
#endif


} // end namespace Numerics

#endif // NUMERICS__SIMD_TRAITS__H
