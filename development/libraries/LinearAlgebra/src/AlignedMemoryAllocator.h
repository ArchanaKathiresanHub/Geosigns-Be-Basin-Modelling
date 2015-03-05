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
      return 0;
   }
#endif

}

template<typename Type, const unsigned int Alignment>
void AlignedMemoryAllocator<Type, Alignment>::free ( Type*& buf ) {

#ifdef _WIN32
   delete [] buf;
   buf = 0;
#else
   if ( buf != 0 ) {
      std::free ( buf );
      buf = 0;
   }
#endif
}



#endif // ALIGNED_MEMORY_ALLOCATOR__H