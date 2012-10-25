//
// Creates H5 compound types for Cauldron
//

#ifndef __H5_TYPES__
#define __H5_TYPES__

#include "DllExport.h"
#include "hdf5.h"
#include "h5_struct_types.h"

#include <stdlib.h>

#include <string>
using namespace std;

const int MAX_DIMS = 4;

// should the h5 types like string, grid struct and strat struct inherit from a common base 
// which contains the hid_t id? Could extend the base to Doubles and floats aswell

//
// H5_String
//
class SERIALHDF5_DLL_EXPORT H5_String
{
public:
   // ctor / dtor
   H5_String (int size = MAX_STRING_SIZE); 

   H5_String (const H5_String& rhs) { deepCopy (rhs); }

   ~H5_String (void) { H5Tclose (hString); }

   // public methods
   int   getSize (void) const { return H5Tget_size (hString); }
   hid_t id	 (void) const { return hString; }

   H5_String& operator= (const H5_String& rhs);
   
private:
   hid_t hString;

   void       setSize  (int size, hid_t myId) { H5Tset_size (myId, size); }
   H5_String& deepCopy (const H5_String& rhs);
};

//
// H5_GridStruct
//
class SERIALHDF5_DLL_EXPORT H5_GridStruct
{
public:
   // ctor /dtor
   H5_GridStruct  (void);
   H5_GridStruct  (const H5_GridStruct& rhs);
   ~H5_GridStruct (void) { H5Tclose (hStruct); }

  // public methods
   hid_t id            (void) const { return hStruct; }
   hid_t stringType    (void) const { return hStr.id(); }
   int   getStructSize (void) const { return sizeof (hStruct); }
   int   getStringSize (void) const { return hStr.getSize(); }

   H5_GridStruct& operator= (const H5_GridStruct& rhs);

private:
   hid_t     hStruct;
   H5_String hStr;

   H5_GridStruct& deepCopy (const H5_GridStruct& rhs);
};

//
// H5_StratStruct
//
class SERIALHDF5_DLL_EXPORT H5_StratStruct
{
public:
   // ctor / dtor
   H5_StratStruct  ();
   H5_StratStruct  (const H5_StratStruct& rhs);
   ~H5_StratStruct () { H5Tclose (hStruct); }

   // public methods
   hid_t id            (void) const { return hStruct; }
   hid_t stringType    (void) const { return hString.id(); }
   hid_t gridType      (void) const { return hGrid.id(); }
   int   getStructSize (void) const { return sizeof (hStruct); }
   int   getStringSize (void) const { return hString.getSize(); }
   
   H5_StratStruct& operator= (const H5_StratStruct &rhs);
   
private:
   hid_t         hStruct;
   H5_String     hString;
   H5_GridStruct hGrid;

   H5_StratStruct& deepCopy (const H5_StratStruct &rhs);
};

//
// Dimensions
//
template <class SizeType>
class SERIALHDF5_DLL_EXPORT H5_Dimensions
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
class SERIALHDF5_DLL_EXPORT H5_FixedSpace
{
public:
   typedef H5_Dimensions<hsize_t>  Dimensions;

#if H5_VERS_MAJOR > 1 || H5_VERS_MINOR > 4
   typedef H5_Dimensions<hsize_t> OffsetSize;
#else
   typedef H5_Dimensions<hssize_t> OffsetSize;
#endif

   // ctor / dtor
   H5_FixedSpace  (int numDims, hsize_t d1, hsize_t d2=0, hsize_t d3=0, hsize_t d4=0);
   H5_FixedSpace  (hid_t space);
   H5_FixedSpace  (const Dimensions& dims);
   H5_FixedSpace  (const H5_FixedSpace& rhs);
   ~H5_FixedSpace () { delete hDims; } 

   // public methods
   H5_FixedSpace&    operator=     (const H5_FixedSpace& rhs);
   hid_t             space_id      (void)      const { return hSpace; }
   const Dimensions& h5_dims       (void)      const { return *hDims; }
   const hsize_t*    dims          (void)      const { return hDims->dimensions(); }
   hsize_t           dims          (int index) const { return hDims->dimensions()[index]; }
   int               numDimensions (void)      const { return hDims->numDimensions(); }

   hid_t setHyperslab (Dimensions &size, OffsetSize &offset);
   bool  sizeEqual    (const H5_FixedSpace& rhs) const;

   void close (void) { H5Sclose (hSpace); }
   
private:
   hid_t      hSpace;
   Dimensions *hDims;

   H5_FixedSpace& deepCopy      (const H5_FixedSpace &rhs);
   hid_t          createSpaceId (const Dimensions *dims);
};

class SERIALHDF5_DLL_EXPORT H5_Type
{
public:   
   // ctor / dtor
   H5_Type (int*)            : htype (H5T_NATIVE_INT)     {} 
   H5_Type (float*)          : htype (H5T_NATIVE_FLOAT)   {}
   H5_Type (double*)         : htype (H5T_NATIVE_DOUBLE)  {}
   H5_Type (char*)           : htype (H5Tcopy (H5T_C_S1)) {}  
   H5_Type (FixedString*)    { htype = h5String.id(); }
   H5_Type (StratRowStruct*) { htype = h5StratType.id(); }
   H5_Type (GridStruct*)     { htype = h5GridType.id(); }
   
   // public methods
   hid_t id (void) { return htype; }
 
   // public static methods
   static void convertToNativeType (hid_t &type);
private:
   hid_t htype;

   H5_StratStruct h5StratType;
   H5_GridStruct  h5GridType;
   H5_String      h5String;
};

#endif
