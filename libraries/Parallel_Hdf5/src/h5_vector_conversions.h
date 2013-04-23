//
// Class to create dataset boundaries 
// based on DA Local Vectors
//

#ifndef __h5_vector_conversions__
#define __h5_vector_conversions__

#include "PetscVectors.h"
#include "h5_types.h"

class H5_VectorBoundaries
{
public:
   // public typedefs
   typedef H5_Dimensions<hsize_t>  Size; 

#if H5_VERS_MAJOR > 1 || H5_VERS_MINOR > 4
   typedef H5_Dimensions<hsize_t> Start;
#else
   typedef H5_Dimensions<hssize_t> Start;
#endif

   // ctor / dtor
   H5_VectorBoundaries (DMDALocalInfo &vecInfo);
  
   // public methods
   Size&  globalBounds (void) { return hGlobal; }
   Size&  localBounds  (void) { return hLocal; }
   Start& offsetBounds (void) { return hOffset; }
       
private:   
   Size  hGlobal;
   Size  hLocal;
   Start hOffset;
};

//
// Convert between linear buffer and petsc vector
//
template <class BufferType>
class VectorToBuffer
{
public:
   // ctor / dtor
   VectorToBuffer  () : vBuffer(NULL) {}
   ~VectorToBuffer () { freeBuffer (); }
   
   // convert from petsc vector to buffer
   BufferType* convert (Petsc_Array *localVec)
   {
      createBuffer (localVec->linearSize());     
    
      // cerr << "size = " << localVec->linearSize() << endl;
      for ( localVec->begin(); ! localVec->end(); localVec->inc() )
      {
	 size_t index = localVec->convertCurrentToLinear();
	 // cerr << "currentPos = " << localVec->currentPos () << ", index = " << index << endl;
         vBuffer[index] = localVec->currentPos();
      }

      return vBuffer;
   }

private:
   BufferType *vBuffer;

   void createBuffer (int size)
   {
      freeBuffer ();
      vBuffer = new BufferType [size];
   }

   void freeBuffer (void)
   {
      if ( vBuffer != NULL ) delete [] vBuffer;
   }
};

template <class BufferType>
class BufferToVector
{
public:
   // convert from buffer to petsc vector
   void convert (BufferType *buffer, 
		 Petsc_Array *localVec)
   {
      if ( buffer != NULL )
      {
         for ( localVec->begin(); ! localVec->end(); localVec->inc() )
         {
	    localVec->currentPos() = buffer[localVec->convertCurrentToLinear()];
	 }    
      }			    
   }
};

#endif
