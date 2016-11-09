//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef ALIGNED_MEMORY_ALLOCATOR__H
#define ALIGNED_MEMORY_ALLOCATOR__H

#include <stdlib.h>
#include <cstdlib>
#include <type_traits>


/// \brief Simple allocator that allocates memory aligned on some address boundary.
///
/// The alignment must be a power of 2 and a multiple of the address size.
template<typename Type, const unsigned int Alignment>
struct AlignedMemoryAllocator {

   /// \brief Compilation should fail if the type used to instatiate this class is not a fundamental type
   ///
   /// Fundamental types are: float, double, (signed or unsigned) char, short, int, long.
   static_assert ( std::is_fundamental<Type>::value,
                   "Type used to instantiate allocator class can be only: float, double, (signed or unsigned) char, short, int, long " );

   /// \brief Do allocation.
   ///
   /// \param [in] numberOfItems The size of the array to be allocated.
   static Type* allocate ( const unsigned int numberOfItems );

   /// \brief Free the memory allocated in the allocate function.
   ///
   /// \param [in out] buf The buffer to be freed.
   /// \pre The memory pointed to in buf was allocated using the allocate function above.
   static void free ( Type*& buf );

#ifndef _WIN32
   /// \brief Free the memory allocated in the allocate function.
   ///
   /// \param [in out] buf The buffer to be freed.
   /// \pre The memory pointed to in buf was allocated using the allocate function above.
   static void free ( Type* __restrict__ & buf );
#endif

};


template<typename Type, const unsigned int Alignment>
Type* AlignedMemoryAllocator<Type, Alignment>::allocate ( const unsigned int numberOfItems ) {

#ifdef _WIN32
   // Since, at the moment, we do not run on
   void* buf = _aligned_malloc ( sizeof ( Type ) * numberOfItems, Alignment );
   return static_cast<Type*>( buf );
   // PRIVATE::PostAllocationConstruction<Type, std::is_fundamental<Type>::value >::construct ( numberOfItems, buf );
#else
   void* buf;
   int error = posix_memalign ( &buf, Alignment, sizeof ( Type ) * numberOfItems );

   if ( error == 0 ) {
      return static_cast<Type*>( buf );
      // PRIVATE::PostAllocationConstruction<Type, std::is_fundamental<Type>::value >::construct ( numberOfItems, buf );
   } else {
      return nullptr;
   }
#endif

}

template<typename Type, const unsigned int Alignment>
void AlignedMemoryAllocator<Type, Alignment>::free ( Type*& buf ) {

#ifdef _WIN32
   void* voidBuf = static_cast<void*>( buf );
   _aligned_free ( voidBuf );
   buf = nullptr;
#else
   if ( buf != nullptr ) {
      std::free ( buf );
      buf = nullptr;
   }
#endif
}


#ifndef _WIN32
template<typename Type, const unsigned int Alignment>
void AlignedMemoryAllocator<Type, Alignment>::free ( Type* __restrict__ & buf ) {

   if ( buf != nullptr ) {
      std::free ( buf );
      buf = nullptr;
   }
}
#endif

#endif // ALIGNED_MEMORY_ALLOCATOR__H
