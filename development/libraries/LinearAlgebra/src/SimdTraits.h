#ifndef NUMERICS__SIMD_TRAITS__H
#define NUMERICS__SIMD_TRAITS__H

#ifdef __SSE__
#include <xmmintrin.h>
#endif

#ifdef __AVX__
#include <immintrin.h>
#endif

namespace Numerics {

   /// \brief Enumeration of the instruction technology to be used.
   ///
   /// Possible future technology additions include:
   ///    - AVXFMA   This extends AVX with a fused-multiply-add instruction.
   ///    - AVX512   This extends the size of the AVX register to 512 bits, thus 8 doubles.
   ///
   enum SimdInstructionTechnology { NO_SIMD, SSE, AVX };


#ifdef __AVX__
  #undef SIMD_TECHNOLOGY
  #define SIMD_TECHNOLOGY AVX
#elif defined __SSE__
  #undef SIMD_TECHNOLOGY
  #define SIMD_TECHNOLOGY SSE
#else
  #undef SIMD_TECHNOLOGY
  #define SIMD_TECHNOLOGY NO_SIMD
#endif



   /// \brief Traits class for alignment and number of double packed into the packed-double.
   ///
   /// If no specialisation exists then NO_SIMD instructions will be used.
   template< const SimdInstructionTechnology SimdTechnology >
   struct SimdTraits {

      /// \brief Alignment on the size of a double.
      static const int Alignment = 8;

      /// \brief The number of doubles contained in the PackedDouble.
      static const int DoubleStride = 1;

      /// \brief A single double.
      typedef double PackedDouble;

   };

#ifdef __SSE__
   /// \brief Specialisation of SimdTraits for SSE instructions.
   template<>
   struct SimdTraits<SSE> {

      /// \brief Alignment on the size of a SSE data type.
      static const int Alignment = 16;

      /// \brief The number of doubles contained in the PackedDouble.
      /// 
      /// For SSE this will be 2 doubles.
      static const int DoubleStride = Alignment / sizeof ( double );

      /// \brief A pair of double values packed into the data type.
      typedef __m128d PackedDouble;

   };
#else
   /// \brief Specialisation of SimdTraits for SSE instructions.
   template<>
   struct SimdTraits<SSE> {
      // Should have a compile time assert
   };
#endif


#ifdef __AVX__
   /// \brief Specialisation of SimdTraits for AVX instructions.
   template<>
   struct SimdTraits<AVX> {

      /// \brief Alignment on the size of a AVX data type.
      static const int Alignment = 32;

      /// \brief The number of doubles contained in the PackedDouble.
      /// 
      /// For AVX this will be 4 doubles.
      static const int DoubleStride = Alignment / sizeof ( double );

      /// \brief Four double values packed into the data type.
      typedef __m256d PackedDouble;

   };
#else
   /// \brief Specialisation of SimdTraits for AVX instructions.
   template<>
   struct SimdTraits<AVX> {
   };
#endif


   /// \brief The maximum simd instruction capability.
   static const SimdInstructionTechnology CurrentSimdTechnology = SSE; //SIMD_TECHNOLOGY;


} // end namespace Numerics

#endif // NUMERICS__SIMD_TRAITS__H
