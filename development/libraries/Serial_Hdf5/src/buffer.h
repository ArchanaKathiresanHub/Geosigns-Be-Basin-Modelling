#ifndef __buffer__
#define __buffer__

#include "DllExport.h"
#include "h5_types.h"

///////////////////////////////////////
//                                   //
//     Buffer Abstract class         //
//                                   //
///////////////////////////////////////

typedef H5_Dimensions<hsize_t> BufferDimensions;

template <class Type>
class SERIALHDF5_DLL_EXPORT Buffer
{
public:
   // ctor / dtor
   Buffer () : ownMemory (false), bBuffer1d (0) {}
   virtual ~Buffer () { cleanUp (); }

   // virtual public methods
   virtual int   linearSize         (void) const = 0;
   virtual Type* get1dVersion       (void) = 0;
   virtual void  createInDataFrom1d (Type *buf1D, const BufferDimensions &dims) = 0;
   virtual bool  createDataFrom1d   (Type *buf1D, const BufferDimensions &dims) = 0;
   virtual void  saveAs1dVersion    (Type *buf1D, const BufferDimensions &dims) = 0;
 
   // public methods 
   void  setDimensions (const BufferDimensions &dims) { Buffer<Type>::bDims = dims; }
   const BufferDimensions& getDimensions (void) const { return Buffer<Type>::bDims; }

protected:
   // Buffer dimensions
   BufferDimensions bDims;

   bool ownMemory;

   Type *bBuffer1d;

   // virtual protected methods
   virtual void cleanUp (void) { Buffer<Type>::bDims = BufferDimensions (); }
};

///////////////////////////////////////
//                                   //
//     Buffer 1D class               //
//                                   //
///////////////////////////////////////

template <class Type>
class SERIALHDF5_DLL_EXPORT Buffer_1D : public Buffer<Type>
{
public:
   // ctor / dtor
   Buffer_1D (void) : Buffer<Type> () {}

   Buffer_1D (Type *buf1D, const BufferDimensions &dims)
   : Buffer<Type> () 
   {
      setFrom1d (buf1D, dims);
   }

   ~Buffer_1D (void) { cleanUp (); }

   // public methods
   int linearSize (void) const { return Buffer<Type>::bDims[0]; } 
   
   Type* get1dVersion (void) 
   {
      return Buffer<Type>::bBuffer1d;
   } 

   void setFrom1d (Type *buf1D, const BufferDimensions &dims)
   {
      if ( dataOkay (buf1D, dims) )
      {
         cleanUp ();

         Buffer<Type>::bBuffer1d = buf1D;
         Buffer<Type>::bDims     = dims;
      }
   }

   void createInDataFrom1d (Type *buf1D, const BufferDimensions &dims)
   {
      createDataFrom1d (buf1D, dims);
   }

   void saveAs1dVersion (Type *buf1D, const BufferDimensions &dims)
   {
      createDataFrom1d (buf1D, dims);
   }

   bool createDataFrom1d (Type *buf1D, const BufferDimensions &dims)
   {
      setFrom1d (buf1D, dims);
      return true;
   }

   Type* getBuffer (void) { return Buffer<Type>::bBuffer1d; }

private:
   bool dataOkay (const Type *buf, const BufferDimensions &dims)
   {
      if ( dims[0] < 1 || !buf ) 
      {
         cout << endl << "Error. Buffer::dataOkay" 
              << endl << "Array bounds negative" << endl;
         return false;
      }
   
      return true;
   }

   void cleanUp (void) 
   {
      if ( Buffer<Type>::ownMemory )
      {  
         delete [] Buffer<Type>::bBuffer1d; 
         Buffer<Type>::bBuffer1d = NULL; 
      }

      Buffer<Type>::cleanUp ();
   }

   //Type *bBuffer;
   
   // making copy ctor and operator= private because
   // they shouldn't be needed 
   Buffer_1D<Type>& operator= (const Buffer_1D<Type> &rhs);
   
   Buffer_1D (const Buffer_1D<Type> &rhs);
};

///////////////////////////////////////
//                                   //
//         Buffer 2D class           //
//                                   //
///////////////////////////////////////

template <class Type>
class SERIALHDF5_DLL_EXPORT Buffer_2D : public Buffer<Type>
{
public:
   // ctor / dtor
   Buffer_2D (void) : Buffer<Type> (), bBuffer2d (NULL) {}

   Buffer_2D (Type *buf1D, const BufferDimensions &dims)
     : Buffer<Type> () , bBuffer2d (NULL)
   {
      createDataFrom1d (buf1D, dims);
   }

   Buffer_2D (Type **buf2d, const BufferDimensions &dims)
     : Buffer<Type> () , bBuffer2d (NULL)
   {
      setFrom2D (buf2d, dims);
   }

   ~Buffer_2D (void) 
   {
      cleanUp (); 
      
      //if ( b1dCopy ) delete [] b1dCopy;
   }

   // public methods
   int linearSize (void) const { return Buffer<Type>::bDims[0] * Buffer<Type>::bDims[1]; } 
   
   Type* get1dVersion (void) 
   { 
     //if ( bBuffer2d == NULL ) return NULL;

      if ( ! Buffer<Type>::bBuffer1d )
      {
	 if ( bBuffer2d == NULL ) return NULL;

         Buffer<Type>::bBuffer1d = new Type [linearSize()];

         int i, j;
         for (i=0; i < Buffer<Type>::bDims[0]; ++i )
         {
            for (j=0; j < Buffer<Type>::bDims[1]; ++j )
            {
               Buffer<Type>::bBuffer1d [convertToLinear (i,j,Buffer<Type>::bDims)] = bBuffer2d [i][j];
            }
         }
      }

      return Buffer<Type>::bBuffer1d;
   } 

   void createInDataFrom1d (Type *buf1D, const BufferDimensions &dims)
   {  
      if ( createDataFrom1d (buf1D, dims) )
      {
         // because this 1d array has been read in, delete it as
         // no longer needed
         delete [] buf1D;
      }
   }

   bool createDataFrom1d (Type *buf1D, const BufferDimensions &dims)
   {
      if ( ! buf1D ) return false;

      Type **tempBuf = new Type* [dims[0]];

      int i,j;
      for ( i=0; i < dims[0]; ++i )
      {
         tempBuf[i] = new Type[dims[1]];
         for ( j=0; j < dims[1]; ++j )
         {
            tempBuf[i][j] = buf1D [convertToLinear (i,j, dims)];
         }
      }

      cleanUp ();

      Buffer<Type>::bDims     = dims;
      bBuffer2d = tempBuf;

      return true;
   }

   void setFrom2D (Type **buf2D, const BufferDimensions &dims)
   {
      if ( dataOkay (buf2D, dims) ) 
      {
         bBuffer2d = buf2D;
         Buffer<Type>::bDims     = dims;
      }
   }

   void saveAs1dVersion (Type *buf1D, const BufferDimensions &dims)
   {
      if ( buf1D )
      {
         Buffer<Type>::bBuffer1d = buf1D;
         Buffer<Type>::bDims     = dims;
      }
   }

   Type** getBuffer (void) 
   {
      if ( ! bBuffer2d )
      {
	 // if no 2d buffer, but data exists as 1d, then convert
	 if ( Buffer<Type>::bBuffer1d )
	 {
	    createDataFrom1d (Buffer<Type>::bBuffer1d, Buffer<Type>::bDims);
	 }
      }

      return bBuffer2d; 
   }

private:
   Type **bBuffer2d;
   //Type *b1dCopy;

   void cleanUp (void) 
   {  
      if ( Buffer<Type>::ownMemory ) 
      {
         for ( int i=0; i < Buffer<Type>::bDims[0]; ++i )
         {
            delete [] bBuffer2d[i];
         }

         delete []bBuffer2d;

         if ( Buffer<Type>::bBuffer1d ) delete [] Buffer<Type>::bBuffer1d;
        
         bBuffer2d = 0;
         Buffer<Type>::bBuffer1d = 0; 
      }

      Buffer<Type>::cleanUp ();
   }

   static int convertToLinear (int x, int y, const BufferDimensions &dims) 
   {
      return (y * dims[0]) + x;
   }

   bool dataOkay (const Type **buf, const BufferDimensions &dims)
   {
      if ( dims[0] < 1 || dims[1] < 1 || ! buf )
      {
         cout << endl << "Error. Buffer_2D::dataOkay"
              << endl << "Array bounds negative or null buffer" << endl;
         return false;
      }
      
      return true;
   }
   
   // making copy ctor and operator= private because
   // they shouldn't be needed 

   Buffer_2D<Type>& operator= (const Buffer_2D<Type> &rhs);

   Buffer_2D (const Buffer_2D<Type> &rhs);
};

///////////////////////////////////////
//                                   //
//          Buffer 3D class          //
//                                   //
///////////////////////////////////////

template <class Type>
class SERIALHDF5_DLL_EXPORT Buffer_3D : public Buffer<Type>
{
public:
   // ctor / dtor
   Buffer_3D (void) : Buffer<Type> (), bBuffer3d (NULL) {}

   Buffer_3D (Type *buf1D, const BufferDimensions &dims)
   : Buffer<Type> (), bBuffer3d (NULL)
   {
      createDataFrom1d (buf1D, dims);
   }

   Buffer_3D (Type ***buf3d, const BufferDimensions &dims)
   : Buffer<Type> (), bBuffer3d (NULL)
   {
      setFrom3D (buf3d, dims);
   }

   ~Buffer_3D (void) 
   { 
      cleanUp (); 
   
      //if ( b1dCopy ) delete [] b1dCopy;
   }

   // public methods
   int linearSize (void) const { return Buffer<Type>::bDims[0] * Buffer<Type>::bDims[1] * Buffer<Type>::bDims[2]; } 
   
   Type* get1dVersion (void) 
   {
     //if ( bBuffer == NULL ) return NULL;
 
      if ( ! Buffer<Type>::bBuffer1d )
      {
         if ( bBuffer3d == NULL ) return NULL;

         Buffer<Type>::bBuffer1d = new Type [linearSize()];
      
         int i, j, k;
         for (i=0; i < Buffer<Type>::bDims[0]; ++i )
         {
            for (j=0; j < Buffer<Type>::bDims[1]; ++j )
            {
               for (k=0; k < Buffer<Type>::bDims[2]; ++k )
               {
                  Buffer<Type>::bBuffer1d [convertToLinear (i,j,k,Buffer<Type>::bDims)] = bBuffer3d [i][j][k];
               }
            }
         }
      }

      return Buffer<Type>::bBuffer1d;
   } 
   
   void createInDataFrom1d (Type *buf1D, const BufferDimensions &dims)
   {
      if ( createDataFrom1d (buf1D, dims) )
      {
         // no longer needed as only used to read in data
         delete [] buf1D;
      }
   }

   bool createDataFrom1d (Type *buf1D, const BufferDimensions &dims)
   {
      if ( ! buf1D ) return false;
      
      Type ***tempBuf = new Type** [dims[0]];

      int i,j,k;
      for ( i=0; i < dims[0]; ++i )
      {
         tempBuf[i] = new Type*[dims[1]];
         for ( j=0; j < dims[1]; ++j )
         {
            tempBuf[i][j] = new Type [dims[2]];
            for ( k=0; k < dims[2]; ++k )
            {
               tempBuf[i][j][k] = buf1D [convertToLinear (i,j,k, dims)];
            }
         }
      }

      cleanUp ();

      Buffer<Type>::bDims     = dims;
      bBuffer3d = tempBuf;

      return true;
   }

   void saveAs1dVersion (Type *buf1d, const BufferDimensions &dims)
   {
      if ( buf1d )
      {
	 Buffer<Type>::bDims = dims;
	 Buffer<Type>::bBuffer1d = buf1d;
      }
   }

   void setFrom3D (Type ***buf3d, const BufferDimensions &dims)
   {
      if ( dataOkay (buf3d, dims) )
      {
         Buffer<Type>::bDims     = dims;
         bBuffer3d = buf3d;
      }
   }

   Type*** getBuffer (void) 
   { 
      if ( ! bBuffer3d )
      {
	 // if no 2d buffer, but data exists as 1d, then convert
	 if ( Buffer<Type>::bBuffer1d )
	 {
	    createDataFrom1d (Buffer<Type>::bBuffer1d, Buffer<Type>::bDims);
	 }
      }

      return bBuffer3d; 
   }

private:
   Type ***bBuffer3d;
   //Type *b1dCopy;

   void cleanUp (void) 
   {  
      if ( Buffer<Type>::ownMemory ) 
      {
         for ( int i=0; i < Buffer<Type>::bDims[0]; ++i )
         {
            for ( int j=0; j < Buffer<Type>::bDims[1]; ++j )
            {          
               delete [] bBuffer3d[i][j];
            }

            delete [] bBuffer3d[i];
         }
         
         delete []bBuffer3d;
         if ( Buffer<Type>::bBuffer1d ) delete [] Buffer<Type>::bBuffer1d;
 
         bBuffer3d = 0;
         Buffer<Type>::bBuffer1d = 0; 
      }

      Buffer<Type>::cleanUp ();
   }

   static int convertToLinear (int x, int y, int z, const BufferDimensions &dims) 
   {
      return (z * dims[1] * dims[0]) + (y * dims[0]) + x;
   }
   
   bool dataOkay (Type ***buf, const BufferDimensions &dims)
   {
      if ( dims[0] < 1 || dims[1] < 1 || dims[2] < 1 || ! buf )
      {
         cout << endl << "Error. Buffer_3D::dataOkay"
              << endl << "Array bounds negative or null buffer" << endl;
         return false;
      }
      
      return true;
   }
};

#endif
