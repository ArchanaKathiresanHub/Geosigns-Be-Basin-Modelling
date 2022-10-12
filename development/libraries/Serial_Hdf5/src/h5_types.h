//
// Creates H5 compound types for Cauldron
//

#ifndef __H5_TYPES__
#define __H5_TYPES__

#include "hdf5.h"

#include <cstdlib>

const int MAX_DIMS = 4;


//
// Dimensions
//
template <class SizeType>
class H5_Dimensions
{
public:
   // ctor / dtor
   H5_Dimensions (void)
   {
      SizeType null = 0;
      deepCopy (0, null, null, null, null);
   }

   H5_Dimensions (int dims, SizeType d1=0, SizeType d2=0, SizeType d3=0, SizeType d4=0)
   {
     deepCopy (dims, d1, d2, d3, d4);
   }

   H5_Dimensions (const H5_Dimensions &rhs)
   {
      deepCopy (rhs.hNumDimensions, rhs.hDimensions[0], rhs.hDimensions[1],
                                    rhs.hDimensions[2], rhs.hDimensions[3]);
   }
      
   ~H5_Dimensions (void) { delete [] hDimensions; }

   // public methods
   bool sizeEqual (const H5_Dimensions<SizeType> &rhs) const
   {
      if ( hNumDimensions != rhs.hNumDimensions ) return false;
   
      int i;   
      for ( i=0; i < hNumDimensions && hDimensions[i] == rhs.hDimensions[i]; ++i );

      return i == hNumDimensions;
   }

   // cannot return const ref to dimensions as incompatable with hdf5
   SizeType* dimensions    (void) const { return hDimensions; }
   int       numDimensions (void) const { return hNumDimensions; } 
 
   void setNumDimensions (int numDims) 
   {
      hNumDimensions = (numDims < 0 || numDims > MAX_DIMS) ? 0 : numDims; 
   }
   
   void setDimensions    (int dims, SizeType d1=0, SizeType d2=0, SizeType d3=0, SizeType d4=0)
   {
      if ( hDimensions != NULL ) delete [] hDimensions;
      deepCopy (dims, d1, d2, d3, d4);
   }

   // operators
   H5_Dimensions& operator= (const H5_Dimensions& rhs)
   {
      if ( this == &rhs ) return *this;

      if ( hDimensions != NULL ) delete [] hDimensions;
      
      return deepCopy (rhs.hNumDimensions, rhs.hDimensions[0], rhs.hDimensions[1],
                                           rhs.hDimensions[2], rhs.hDimensions[3]);
   }

   SizeType&       operator [] (int index) { return hDimensions[index]; }
   const SizeType& operator [] (int index) const { return hDimensions[index]; }

   int numDims () const { return hNumDimensions; }

private:
   int      hNumDimensions;
   SizeType *hDimensions;

   // private methods
   H5_Dimensions& deepCopy (int dims, SizeType &d1, SizeType &d2, SizeType &d3, SizeType &d4)
   {
      hDimensions    = new SizeType[MAX_DIMS];
      hNumDimensions = dims;

      hDimensions[0] = d1;
      hDimensions[1] = d2;
      hDimensions[2] = d3;
      hDimensions[3] = d4;

      return *this;
   }
};

//
// H5_FixedSpace
//
class H5_FixedSpace
{
public:
   typedef H5_Dimensions<hsize_t>  Dimensions;

   typedef H5_Dimensions<hsize_t> OffsetSize;

   // ctor / dtor
   H5_FixedSpace  (int numDims, hsize_t d1, hsize_t d2=0, hsize_t d3=0, hsize_t d4=0);
   H5_FixedSpace  (hid_t space);
   H5_FixedSpace  (const Dimensions& dims);
   H5_FixedSpace  (const H5_FixedSpace& rhs);
   ~H5_FixedSpace ();

   // public methods
   H5_FixedSpace&    operator=     (const H5_FixedSpace& rhs);
   hid_t             space_id      (void)      const { return hSpace; }
   const Dimensions& h5_dims       (void)      const { return *hDims; }
   const hsize_t*    dims          (void)      const { return hDims->dimensions(); }
   hsize_t           dims          (int index) const { return hDims->dimensions()[index]; }
   int               numDimensions (void)      const { return hDims->numDimensions(); }

   hid_t setHyperslab (Dimensions &size, OffsetSize &offset);
   hid_t setChunkedHyperslab (Dimensions &size, OffsetSize &offset);
   bool  sizeEqual    (const H5_FixedSpace& rhs) const;

   void close (void) { H5Sclose (hSpace); } 

   void initHyperslabBuffers( const int dims );
   
private:
   hid_t      hSpace;
   Dimensions *hDims;
   hsize_t * m_count;
   hsize_t * m_stride;

   H5_FixedSpace& deepCopy      (const H5_FixedSpace &rhs);
   hid_t          createSpaceId (const Dimensions *dims);
};

class H5_Type
{
public:   
   // ctor / dtor
   H5_Type (int*)            : htype (H5T_NATIVE_INT)     {} 
   H5_Type (float*)          : htype (H5T_NATIVE_FLOAT)   {}
   H5_Type (double*)         : htype (H5T_NATIVE_DOUBLE)  {}
   H5_Type (char*)           : htype (H5Tcopy (H5T_C_S1)) {}  
   
   // public methods
   hid_t id (void) { return htype; }
 
   // public static methods
   static void convertToNativeType (hid_t &type);
private:
   hid_t htype;
};

#endif
