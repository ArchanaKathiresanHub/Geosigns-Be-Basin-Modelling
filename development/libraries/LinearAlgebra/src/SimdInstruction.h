#ifndef NUMERICS__SIMD_INSTRUCTION__H
#define NUMERICS__SIMD_INSTRUCTION__H

#include <stdlib.h>
#include "SimdTraits.h"

namespace Numerics {


   template< const SimdInstructionTechnology SimdTechnology >
   struct SimdInstruction {

      /// \brief The packed double for NO_SIMD is a double.
      typedef SimdTraits<NO_SIMD>::PackedDouble PackedDouble;

      /// \brief Required alignment for PackedDouble.
      static const int Alignment = SimdTraits<NO_SIMD>::Alignment;

      /// \brief The number of doubles in the PackedDouble.
      static const int DoubleStride = SimdTraits<NO_SIMD>::DoubleStride;


      /// \brief Allocate an array of size numberOfDoubles.
      ///
      /// For NO_SIMD there are no memory alignment requirements.
      static double* allocate ( const int numberOfDoubles );

      /// \brief Deallocate a buffer of double.
      ///
      /// The memory freed here should have been allocated with the allocate function.
      static void free ( double*& buf );

      /// \brief Copy a double.
      static PackedDouble set1 ( const double a );

      /// \brief Store valued from the PackedDouble of b to memory pointed to in a.
      ///
      /// In this case there are no alignment requirements.
      ///
      /// a[0] = b[0]
      static void store ( double* a, const PackedDouble& b );

      /// \brief Store valued from the PackedDouble of b to memory pointed to in a.
      ///
      /// In this case there are no alignment requirements.
      ///
      /// a[0] = b[0]
      static void storeu ( double* a, const PackedDouble& b );

      /// \brief Add two doubles
      static PackedDouble add ( const PackedDouble& a, const PackedDouble& b );

      /// \brief Multiply two doubles
      static PackedDouble mul ( const PackedDouble& a, const PackedDouble& b );

      /// \brief Compute a multiply and add, i.e. a * b + c
      static PackedDouble mulAdd ( const PackedDouble& a, const PackedDouble& b, const PackedDouble& c );

   };



#ifdef __SSE__
   /// \brief Specialisation of SimdInstruction with SSE instruction set.
   template<>
   struct SimdInstruction<SSE> {

      /// \brief The simd data-type for SSE.
      typedef SimdTraits<SSE>::PackedDouble PackedDouble;

      /// \brief Required alignment for SSE PackedDouble.
      static const int Alignment = SimdTraits<SSE>::Alignment;

      /// \brief The number of doubles in the SSE PackedDouble.
      static const int DoubleStride = SimdTraits<SSE>::DoubleStride;

      /// \brief Allocate an array of size numberOfDoubles.
      ///
      /// For SSE the memory allocated will be aligned on a 16 byte address boundary.
      /// \param [in] numberOfDoubles The number of double required.
      /// \pre numberOfDoubles > 0.
      /// \post If it was possible to allocate memory with the required alignment then
      /// this buffer will be returned otherwise a null pointer will be returned.
      static double* allocate ( const int numberOfDoubles );

      /// \brief Deallocate a buffer of doubles.
      ///
      /// The memory freed here should have been allocated with the allocate function.
      static void free ( double*& buf );

      /// \brief Fill a PackedDouble with a single value.
      ///
      /// res[i] = a, for i = 1, 2
      static PackedDouble set1 ( const double a );

      /// \brief Store valued from the PackedDouble of b to memory pointed to in a.
      ///
      /// The memory in b must have the correct alignment.
      /// It must be aligned on the address specified in the variable Alignment defined above.
      ///
      /// a[i] = b[i], for i = 1, 2
      /// \pre The memory pointed to in a must be alligned on a 16 byte address boundary.
      static void store ( double* a, const PackedDouble& b );

      /// \brief Store valued from the PackedDouble of b to memory pointed to in a.
      ///
      /// The memory in b need not have the correct alignment.
      /// It can be aligned on any address.
      ///
      /// a[i] = b[i], for i = 1, 2
      static void storeu ( double* a, const PackedDouble& b );

      /// \brief Add two packed doubles together.
      ///
      /// res[i] = a[i] + b[i], for i = 1, 2
      static PackedDouble add ( const PackedDouble& a, const PackedDouble& b );

      /// \brief Multiply two packed doubles together.
      ///
      /// res[i] = a[i] * b[i], for i = 1, 2
      static PackedDouble mul ( const PackedDouble& a, const PackedDouble& b );

      /// \brief Add two packed doubles together and add to another.
      ///
      /// res[i] = a[i] * b[i] + c[i], for i = 1, 2
      static PackedDouble mulAdd ( const PackedDouble& a, const PackedDouble& b, const PackedDouble& c );

   };
#endif



#ifdef __AVX__
   /// \brief Specialisation of SimdInstruction with AVX instruction set.
   template<>
   struct SimdInstruction<AVX> {

      /// \brief The simd data-type for AVX.
      typedef SimdTraits<AVX>::PackedDouble PackedDouble;

      /// \brief Required alignment for AVX PackedDouble.
      static const int Alignment = SimdTraits<AVX>::Alignment;

      /// \brief The number of doubles in the AVX PackedDouble.
      static const int DoubleStride = SimdTraits<AVX>::DoubleStride;

      /// \brief Allocate an array of size numberOfDoubles.
      ///
      /// For AVX the memory allocated will be aligned on a 32 byte address boundary.
      /// \param [in] numberOfDoubles The number of double required.
      /// \pre numberOfDoubles > 0.
      /// \post If it was possible to allocate memory with the required alignment then
      /// this buffer will be returned otherwise a null pointer will be returned.
      static double* allocate ( const int numberOfDoubles );

      /// \brief Deallocate a buffer of double.
      ///
      /// The memory freed here should have been allocated with the allocate function.
      /// \param [inout] buf The buffer that is to be freed.
      /// \pre The buffer must have been allocated using the allocate function defined in this class.
      static void free ( double*& buf );

      /// \brief Fill a PackedDouble with a single value.
      ///
      /// res(i)=a, where i = 0, 1, 2, 3
      static PackedDouble set1 ( const double a );

      /// \brief Store valued from the PackedDouble of b to memory pointed to in a.
      ///
      /// The memory in b must have the correct alignment.
      /// It must be aligned on the address specified in the variable Alignment defined above.
      ///
      /// a[i] = b[i], for i = 1, 2, 3, 4
      /// \pre The memory pointed to in a must be alligned on a 32 byte address boundary.
      static void store ( double* a, const PackedDouble& b );

      /// \brief Store valued from the PackedDouble of b to memory pointed to in a.
      ///
      /// The memory in b need not have the correct alignment.
      /// It can be aligned on any address.
      ///
      /// a[i] = b[i], for i = 1, 2, 3, 4
      static void storeu ( double* a, const PackedDouble& b );

      /// \brief Add two packed doubles together.
      ///
      /// res(i)=a(i)+b(i), where i = 0, 1, 2, 3
      static PackedDouble add ( const PackedDouble& a, const PackedDouble& b );

      /// \brief Multiply two packed doubles together.
      ///
      /// res(i)=a(i)*b(i), where i = 0, 1, 2, 3
      static PackedDouble mul ( const PackedDouble& a, const PackedDouble& b );

      /// \brief Add two packed doubles together and add to another.
      ///
      /// res(i)=a(i)*b(i)+c(i), where i = 0, 1, 2, 3
      static PackedDouble mulAdd ( const PackedDouble& a, const PackedDouble& b, const PackedDouble& c );

   };
#endif


} // end namespace Numerics

//--------------------------------
// Inline functions
//--------------------------------

//--------------------------------
// NO_SIMD
//--------------------------------

template< const Numerics::SimdInstructionTechnology SimdTechnology >
inline double* Numerics::SimdInstruction<SimdTechnology>::allocate ( const int numberOfDoubles ) {
   return new double [ numberOfDoubles ];
}

template< const Numerics::SimdInstructionTechnology SimdTechnology >
inline void Numerics::SimdInstruction<SimdTechnology>::free ( double*& buf ) {
   delete [] buf;
   buf = 0;
}

template< const Numerics::SimdInstructionTechnology SimdTechnology >
inline typename Numerics::SimdInstruction<SimdTechnology>::PackedDouble
Numerics::SimdInstruction<SimdTechnology>::set1 ( const double a ) {
   return a;
}

template< const Numerics::SimdInstructionTechnology SimdTechnology >
inline void Numerics::SimdInstruction<SimdTechnology>::store ( double* b, const PackedDouble& a ) {
   b[0] = a;
}

template< const Numerics::SimdInstructionTechnology SimdTechnology >
inline void Numerics::SimdInstruction<SimdTechnology>::storeu ( double* b, const PackedDouble& a ) {
   b[0] = a;
}

template< const Numerics::SimdInstructionTechnology SimdTechnology >
inline typename Numerics::SimdInstruction<SimdTechnology>::PackedDouble
Numerics::SimdInstruction<SimdTechnology>::add ( const PackedDouble& a, const PackedDouble& b ) {
   return a + b;
}

template< const Numerics::SimdInstructionTechnology SimdTechnology >
inline typename Numerics::SimdInstruction<SimdTechnology>::PackedDouble
Numerics::SimdInstruction<SimdTechnology>::mul ( const PackedDouble& a, const PackedDouble& b ) {
   return a * b;
}

template< const Numerics::SimdInstructionTechnology SimdTechnology >
inline typename Numerics::SimdInstruction<SimdTechnology>::PackedDouble
Numerics::SimdInstruction<SimdTechnology>::mulAdd ( const PackedDouble& a, const PackedDouble& b, const PackedDouble& c ) {
   return a * b + c;
}

//--------------------------------
// SSE
//--------------------------------

#ifdef __SSE__
inline double* Numerics::SimdInstruction<Numerics::SSE>::allocate ( const int numberOfDoubles ) {

   void* buf;
   int error = posix_memalign ( &buf, Alignment, sizeof ( double ) * numberOfDoubles );

   if ( error == 0 ) {
      return (double*)(buf);
   } else {
      return 0;
   }

}

inline void Numerics::SimdInstruction<Numerics::SSE>::free ( double*& buf ) {

   if ( buf != 0 ) {
      ::free ( buf );
      buf = 0;
   }

}

inline Numerics::SimdInstruction<Numerics::SSE>::PackedDouble
Numerics::SimdInstruction<Numerics::SSE>::set1 ( const double a ) {
   return _mm_set1_pd ( a );
}

inline void Numerics::SimdInstruction<Numerics::SSE>::store ( double* b, const PackedDouble& a ) {
   _mm_store_pd ( b, a );
}

inline void Numerics::SimdInstruction<Numerics::SSE>::storeu ( double* b, const PackedDouble& a ) {
   _mm_storeu_pd ( b, a );
}

inline Numerics::SimdInstruction<Numerics::SSE>::PackedDouble
Numerics::SimdInstruction<Numerics::SSE>::add ( const PackedDouble& a, const PackedDouble& b ) {
   return _mm_add_pd ( a, b );
}

inline Numerics::SimdInstruction<Numerics::SSE>::PackedDouble
Numerics::SimdInstruction<Numerics::SSE>::mul ( const PackedDouble& a, const PackedDouble& b ) {
   return _mm_mul_pd ( a, b );
}

inline Numerics::SimdInstruction<Numerics::SSE>::PackedDouble
Numerics::SimdInstruction<Numerics::SSE>::mulAdd ( const PackedDouble& a, const PackedDouble& b, const PackedDouble& c ) {
   return _mm_add_pd ( c, _mm_mul_pd ( a, b ));
}
#endif

//--------------------------------
// AVX
//--------------------------------

#ifdef __AVX__
inline double* Numerics::SimdInstruction<Numerics::AVX>::allocate ( const int numberOfDoubles ) {

   void* buf;
   int error = posix_memalign ( &buffer, Alignment, sizeof ( double ) * numberOfDoubles );

   if ( error == 0 ) {
      return (double*)(buf);
   } else {
      return 0;
   }

}

inline void Numerics::SimdInstruction<Numerics::AVX>::free ( double*& buf ) {

   if ( buf != 0 ) {
      ::free ( buf );
      buf = 0;
   }

}

inline Numerics::SimdInstruction<Numerics::AVX>::PackedDouble
Numerics::SimdInstruction<Numerics::AVX>::set1 ( const double a ) {
   return _mm256_set1_pd ( a );
}

inline void Numerics::SimdInstruction<AVX>::store ( double* a, const PackedDouble& b ) {
   _mm256_store_pd ( a, b );
}

inline void Numerics::SimdInstruction<AVX>::storeu ( double* a, const PackedDouble& b ) {
   _mm256_storeu_pd ( a, b );
}

inline Numerics::SimdInstruction<Numerics::AVX>::PackedDouble
Numerics::SimdInstruction<Numerics::AVX>::add ( const PackedDouble& a, const PackedDouble& b ) {
   return _mm256_add_pd ( a, b );
}

inline Numerics::SimdInstruction<Numerics::AVX>::PackedDouble
Numerics::SimdInstruction<Numerics::AVX>::mul ( const PackedDouble& a, const PackedDouble& b ) {
   return _mm256_mul_pd ( a, b );
}

inline Numerics::SimdInstruction<Numerics::AVX>::PackedDouble
Numerics::SimdInstruction<Numerics::AVX>::mulAdd ( const PackedDouble& a, const PackedDouble& b, const PackedDouble& c ) {
   return _mm256_add_pd ( c, _mm256_mul_pd ( a, b ));
}
#endif



#endif // NUMERICS__SIMD_INSTRUCTION__H
