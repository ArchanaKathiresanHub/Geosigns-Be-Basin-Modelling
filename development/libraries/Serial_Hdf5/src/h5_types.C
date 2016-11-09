//
// Creates H5 compound types for Cauldron
//

#include "h5_types.h"


//
// H5_FixedSpace METHODS
//
H5_FixedSpace::H5_FixedSpace (int numDims, hsize_t d1, hsize_t d2, hsize_t d3, hsize_t d4)
{
   hDims  = new Dimensions (numDims, d1, d2, d3, d4);
   hSpace = createSpaceId (hDims);
   
   initHyperslabBuffers ( numDims );
}

H5_FixedSpace::H5_FixedSpace (hid_t space)
{
   // create dimension memory
   int numDimensions = H5Sget_simple_extent_ndims (space);
   hDims = new Dimensions (numDimensions);
   
   // assign dimension values 
   H5Sget_simple_extent_dims (space, hDims->dimensions(), NULL );

   // copy space id
   hSpace = space;
   //hSpace = createSpaceId (hDims);

   initHyperslabBuffers ( numDimensions );
}

H5_FixedSpace::H5_FixedSpace (const Dimensions& dims)
{
   // create class on Dimensions values
   hDims  = new Dimensions (dims);
   hSpace = createSpaceId  (hDims);

   initHyperslabBuffers ( dims.numDims() );
}

H5_FixedSpace::H5_FixedSpace (const H5_FixedSpace& rhs)
{
   deepCopy (rhs); 
   
}

H5_FixedSpace:: ~H5_FixedSpace () { 

   delete hDims; 
   if( m_count != 0 ) delete [] m_count;
   if( m_stride != 0 ) delete [] m_stride;
} 

H5_FixedSpace& H5_FixedSpace::operator= (const H5_FixedSpace& rhs)
{
   if ( this == &rhs ) return *this;
   
   if ( hDims != NULL ) 
   {
      delete hDims;
   }

   return deepCopy (rhs);
}
	 
hid_t H5_FixedSpace::setHyperslab (Dimensions &size, OffsetSize &offset)
{
   
   return H5Sselect_hyperslab (hSpace, H5S_SELECT_SET, offset.dimensions(), NULL, 
                               size.dimensions(), NULL);
}

hid_t H5_FixedSpace::setChunkedHyperslab (Dimensions &size, OffsetSize &offset)
{
   return H5Sselect_hyperslab (hSpace, H5S_SELECT_SET, offset.dimensions(), m_stride, 
                               m_count , size.dimensions() );
 
}

bool H5_FixedSpace::sizeEqual (const H5_FixedSpace& rhs) const
{
   return hDims->sizeEqual (*rhs.hDims);
}

H5_FixedSpace& H5_FixedSpace::deepCopy (const H5_FixedSpace& rhs)
{
   hDims  = new Dimensions (*(rhs.hDims));
   hSpace = createSpaceId  (hDims);

   initHyperslabBuffers(rhs.numDimensions());
 
   return *this;
}

void H5_FixedSpace::initHyperslabBuffers ( const int dims )
{
   m_stride = new hsize_t[dims];
   m_count  = new hsize_t[dims];

   for( int i = 0; i < dims; ++ i ) {
      m_stride[i] = 1;
      m_count[i]  = 1;
   }

}

hid_t H5_FixedSpace::createSpaceId (const Dimensions *dims)
{
   return H5Screate_simple (dims->numDimensions(), dims->dimensions(), NULL);
}

void H5_Type::convertToNativeType (hid_t &type)
{
  // find the native type corresponding to the saved type
  // in case the saved type was done on another platform

  // float
  if ( H5Tequal (type, H5T_IEEE_F32BE) ||
       H5Tequal (type, H5T_IEEE_F32LE) )
  {
    H5Tclose (type);
    type = H5T_NATIVE_FLOAT;
  }

  // double
  else if ( H5Tequal (type, H5T_IEEE_F64BE) ||
	    H5Tequal (type, H5T_IEEE_F64LE) )
  {
    H5Tclose (type);
    type = H5T_NATIVE_DOUBLE;
  }

  // int
  else if ( H5Tequal (type, H5T_STD_I32BE) ||
	    H5Tequal (type, H5T_STD_I32LE) )
  {
    H5Tclose (type);
    type = H5T_NATIVE_INT;
  }
}
