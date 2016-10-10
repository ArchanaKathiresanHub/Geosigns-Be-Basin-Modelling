#ifndef NUMERICS__SIMD_INSTRUCTION__H
#define NUMERICS__SIMD_INSTRUCTION__H

#include <stdlib.h>
#include "SimdTraits.h"
#include "AlignedMemoryAllocator.h"

namespace Numerics {


   template< const SimdInstructionTechnology SimdTechnology >
   struct SimdInstruction {

      /// \brief Instantiation of SimdTraits with the NO_SIMD configuration.
      typedef SimdTraits<NO_SIMD> Traits;

      /// \brief The packed double for NO_SIMD is a double.
      typedef Traits::PackedDouble PackedDouble;

      /// \brief Required alignment for PackedDouble.
      static const int Alignment = Traits::Alignment;

      /// \brief The number of doubles in the PackedDouble.
      static const int DoubleStride = Traits::DoubleStride;


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


   /// \brief Specialisation of SimdInstruction with SSE instruction set.
   template<>
   struct SimdInstruction<SSE> {

      /// \brief Instantiation of SimdTraits with the SSE configuration.
      typedef SimdTraits<SSE> Traits;

      /// \brief The simd data-type for SSE.
      typedef Traits::PackedDouble PackedDouble;

      /// \brief Required alignment for SSE PackedDouble.
      static const int Alignment = Traits::Alignment;

      /// \brief The number of doubles in the SSE PackedDouble.
      static const int DoubleStride = Traits::DoubleStride;

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

      /// \brief Multiply two packed doubles together and add to another.
      ///
      /// res[i] = a[i] * b[i] + c[i], for i = 1, 2
      static PackedDouble mulAdd ( const PackedDouble& a, const PackedDouble& b, const PackedDouble& c );

   };



   /// \brief Specialisation of SimdInstruction with AVX instruction set.
   template<>
   struct SimdInstruction<AVX> {

      /// \brief Instantiation of SimdTraits with the AVX configuration.
      typedef SimdTraits<AVX> Traits;

      /// \brief The simd data-type for AVX.
      typedef Traits::PackedDouble PackedDouble;

      /// \brief Required alignment for AVX PackedDouble.
      static const int Alignment = Traits::Alignment;

      /// \brief The number of doubles in the AVX PackedDouble.
      static const int DoubleStride = Traits::DoubleStride;

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

      /// \brief Multiply two packed doubles together and add to another.
      ///
      /// res(i)=a(i)*b(i)+c(i), where i = 0, 1, 2, 3
      static PackedDouble mulAdd ( const PackedDouble& a, const PackedDouble& b, const PackedDouble& c );

   };



   /// \brief Specialisation of SimdInstruction with AVX-FMA instruction set.
   template<>
   struct SimdInstruction<AVXFMA> {

      /// \brief Instantiation of SimdTraits with the AVX-FMA configuration.
      typedef SimdTraits<AVXFMA> Traits;

      /// \brief The simd data-type for AVX-FMA.
      typedef Traits::PackedDouble PackedDouble;

      /// \brief Required alignment for AVX PackedDouble.
      static const int Alignment = Traits::Alignment;

      /// \brief The number of doubles in the AVX PackedDouble.
      static const int DoubleStride = Traits::DoubleStride;

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

      /// \brief Multiply two packed doubles together and add to another.
      ///
      /// res(i)=a(i)*b(i)+c(i), where i = 0, 1, 2, 3
      static PackedDouble mulAdd ( const PackedDouble& a, const PackedDouble& b, const PackedDouble& c );

   };


} // end namespace Numerics

//--------------------------------
// Inline functions
//--------------------------------

//--------------------------------
// NO_SIMD
//--------------------------------

template< const Numerics::SimdInstructionTechnology SimdTechnology >
inline double* Numerics::SimdInstruction<SimdTechnology>::allocate ( const int numberOfDoubles ) {
   return AlignedMemoryAllocator<double, Alignment>::allocate ( numberOfDoubles );
}

template< const Numerics::SimdInstructionTechnology SimdTechnology >
inline void Numerics::SimdInstruction<SimdTechnology>::free ( double*& buf ) {
   AlignedMemoryAllocator<double, Alignment>::free ( buf );
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


inline double* Numerics::SimdInstruction<Numerics::SSE>::allocate ( const int numberOfDoubles ) {
   return AlignedMemoryAllocator<double, Alignment>::allocate ( numberOfDoubles );
}

inline void Numerics::SimdInstruction<Numerics::SSE>::free ( double*& buf ) {
   AlignedMemoryAllocator<double, Alignment>::free ( buf );
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


//--------------------------------
// AVX
//--------------------------------


inline double* Numerics::SimdInstruction<Numerics::AVX>::allocate ( const int numberOfDoubles ) {
   return AlignedMemoryAllocator<double, Alignment>::allocate ( numberOfDoubles );
}

inline void Numerics::SimdInstruction<Numerics::AVX>::free ( double*& buf ) {
   AlignedMemoryAllocator<double, Alignment>::free ( buf );
}

inline Numerics::SimdInstruction<Numerics::AVX>::PackedDouble
Numerics::SimdInstruction<Numerics::AVX>::set1 ( const double a ) {
   return _mm256_set1_pd ( a );
}

inline void Numerics::SimdInstruction<Numerics::AVX>::store ( double* a, const PackedDouble& b ) {
   _mm256_store_pd ( a, b );
}

inline void Numerics::SimdInstruction<Numerics::AVX>::storeu ( double* a, const PackedDouble& b ) {
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


//--------------------------------
// AVX-FMA
//--------------------------------



inline double* Numerics::SimdInstruction<Numerics::AVXFMA>::allocate ( const int numberOfDoubles ) {
   return AlignedMemoryAllocator<double, Alignment>::allocate ( numberOfDoubles );
}

inline void Numerics::SimdInstruction<Numerics::AVXFMA>::free ( double*& buf ) {
   AlignedMemoryAllocator<double, Alignment>::free ( buf );
}

inline Numerics::SimdInstruction<Numerics::AVXFMA>::PackedDouble
Numerics::SimdInstruction<Numerics::AVXFMA>::set1 ( const double a ) {
   return _mm256_set1_pd ( a );
}

inline void Numerics::SimdInstruction<Numerics::AVXFMA>::store ( double* a, const PackedDouble& b ) {
   _mm256_store_pd ( a, b );
}

inline void Numerics::SimdInstruction<Numerics::AVXFMA>::storeu ( double* a, const PackedDouble& b ) {
   _mm256_storeu_pd ( a, b );
}

inline Numerics::SimdInstruction<Numerics::AVXFMA>::PackedDouble
Numerics::SimdInstruction<Numerics::AVXFMA>::add ( const PackedDouble& a, const PackedDouble& b ) {
   return _mm256_add_pd ( a, b );
}

inline Numerics::SimdInstruction<Numerics::AVXFMA>::PackedDouble
Numerics::SimdInstruction<Numerics::AVXFMA>::mul ( const PackedDouble& a, const PackedDouble& b ) {
   return _mm256_mul_pd ( a, b );
}

inline Numerics::SimdInstruction<Numerics::AVXFMA>::PackedDouble
Numerics::SimdInstruction<Numerics::AVXFMA>::mulAdd ( const PackedDouble& a, const PackedDouble& b, const PackedDouble& c ) {
   // comptute a * b + c
   return _mm256_fmadd_pd ( a, b, c );
}



#endif // NUMERICS__SIMD_INSTRUCTION__H
