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

/// \brief Simple allocator that allocates memory aligned on some address boundary.
///
/// The alignment must be a power of 2 and a multiple of the address size.
template<typename Type, const unsigned int Alignment>
struct AlignedMemoryAllocator {

   /// \brief Do allocation.
   ///
   /// \param [in] numberOfItems The size of the array to be allocated.
   static Type* allocate ( const unsigned int numberOfItems );

   /// \brief Free the memory allocated in the allocate function.
   ///
   /// \param [in out] buf The buffer to be freed.
   /// \pre The memory pointed to in buf was allocated using the allocate function above.
   static void free ( Type*& buf );

};


template<typename Type, const unsigned int Alignment>
Type* AlignedMemoryAllocator<Type, Alignment>::allocate ( const unsigned int numberOfItems ) {

#ifdef _WIN32
   // Since, at the moment, we do not run on 
   return new Type [ numberOfItems ];
#else
   void* buf;
   int error = posix_memalign ( &buf, Alignment, sizeof ( Type ) * numberOfItems );

   if ( error == 0 ) {
      return (Type*)(buf);
   } else {
      return nullptr;
   }
#endif

}

template<typename Type, const unsigned int Alignment>
void AlignedMemoryAllocator<Type, Alignment>::free ( Type*& buf ) {

#ifdef _WIN32
   delete [] buf;
   buf = nullptr;
#else
   if ( buf != nullptr ) {
      std::free ( buf );
      buf = nullptr;
   }
#endif
}



#endif // ALIGNED_MEMORY_ALLOCATOR__H
