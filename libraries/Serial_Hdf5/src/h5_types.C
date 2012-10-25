#include "stdafx.h"
//
// Creates H5 compound types for Cauldron
//

#include "h5_types.h"

//
// H5_String METHODS
//
H5_String::H5_String (int size)
{
   // create string id
   hid_t stringId = H5Tcopy (H5T_C_S1);
   setSize (size, stringId);

   // create data type
   hString = H5Tcreate (H5T_COMPOUND, FixedString::getStructSize ());
   H5Tinsert (hString, "name", HOFFSET (FixedString, str), stringId);
}

H5_String& H5_String::operator= (const H5_String& rhs)
{
   if ( this == &rhs ) return *this;

   H5Tclose (hString);
   return deepCopy (rhs); 
}

H5_String& H5_String::deepCopy (const H5_String& rhs)
{
   hString = H5Tcopy (rhs.hString);
   return *this;
}

//
// H5_GridStruct METHODS
//
H5_GridStruct::H5_GridStruct (void)
{
   // create data type
   hStruct = H5Tcreate (H5T_COMPOUND, GridStruct::getStructSize() );

   // add fields to data type
   H5Tinsert (hStruct, "name", HOFFSET(GridStruct, gridName), hStr.id());
}

H5_GridStruct::H5_GridStruct (const H5_GridStruct& rhs)
{
   deepCopy (rhs);
}

H5_GridStruct& H5_GridStruct::operator= (const H5_GridStruct& rhs)
{
   if ( this == &rhs ) return *this;

   H5Tclose (hStruct);
   return deepCopy (rhs);
}

H5_GridStruct& H5_GridStruct::deepCopy (const H5_GridStruct& rhs)
{
   hStruct = H5Tcopy (rhs.hStruct);
   hStr    = rhs.hStr;
   return *this;
}

//
// H5_StratStruct METHODS
//
H5_StratStruct::H5_StratStruct ()
{
   // create data type
   hStruct = H5Tcreate (H5T_COMPOUND, StratRowStruct::getStructSize());
   
   // add fields to data type
   
   //                  hdf5 ref name        hdf5 offset in file                          field type
   H5Tinsert (hStruct, "SurfaceName",       HOFFSET (StratRowStruct, SurfaceName),       hString.id());
   H5Tinsert (hStruct, "DepoAge",           HOFFSET (StratRowStruct, DepoAge),           H5T_NATIVE_FLOAT);
   H5Tinsert (hStruct, "DepthGrid",         HOFFSET (StratRowStruct, DepthGrid),         hGrid.id());
   H5Tinsert (hStruct, "TwoWayTime",        HOFFSET (StratRowStruct, TwoWayTime),        H5T_NATIVE_FLOAT);
   H5Tinsert (hStruct, "LayerName",         HOFFSET (StratRowStruct, LayerName),         hString.id());
   H5Tinsert (hStruct, "DepoSequence",      HOFFSET (StratRowStruct, DepoSequence),      H5T_NATIVE_INT);
   H5Tinsert (hStruct, "ThicknessGrid",     HOFFSET (StratRowStruct, ThicknessGrid),     hGrid.id());
   H5Tinsert (hStruct, "LithoType1",        HOFFSET (StratRowStruct, LithoType1),        hString.id());
   H5Tinsert (hStruct, "LithoType2",        HOFFSET (StratRowStruct, LithoType2),        hString.id());
   H5Tinsert (hStruct, "LithoType3",        HOFFSET (StratRowStruct, LithoType3),        hString.id());
   H5Tinsert (hStruct, "Percent1",          HOFFSET (StratRowStruct, Percent1),          hString.id());
   H5Tinsert (hStruct, "Percent2",          HOFFSET (StratRowStruct, Percent2),          hString.id());
   H5Tinsert (hStruct, "Percent3",          HOFFSET (StratRowStruct, Percent3),          hString.id());
   H5Tinsert (hStruct, "PercentGrid1",      HOFFSET (StratRowStruct, PercentGrid1),      hGrid.id());
   H5Tinsert (hStruct, "PercentGrid2",      HOFFSET (StratRowStruct, PercentGrid2),      hGrid.id());
   H5Tinsert (hStruct, "PercentGrid3",      HOFFSET (StratRowStruct, PercentGrid3),      hGrid.id());
   H5Tinsert (hStruct, "MixModel",          HOFFSET (StratRowStruct, MixModel),          hString.id());
   H5Tinsert (hStruct, "SeisVelocityModel", HOFFSET (StratRowStruct, SeisVelocityModel), hString.id());
   H5Tinsert (hStruct, "FluidType",         HOFFSET (StratRowStruct, FluidType),         hString.id());
   H5Tinsert (hStruct, "SourceRock",        HOFFSET (StratRowStruct, SourceRock),        H5T_NATIVE_INT);
   H5Tinsert (hStruct, "MobileLayer",       HOFFSET (StratRowStruct, MobileLayer),       H5T_NATIVE_INT);
}

H5_StratStruct::H5_StratStruct (const H5_StratStruct& rhs)
{
   deepCopy (rhs);
}

H5_StratStruct& H5_StratStruct::operator= (const H5_StratStruct &rhs)
{
   if ( this == &rhs ) return *this;

   H5Tclose (hStruct);
   return deepCopy (rhs);
}

H5_StratStruct& H5_StratStruct::deepCopy (const H5_StratStruct &rhs)
{
   hString = rhs.hString;
   hGrid   = rhs.hGrid;
   hStruct = H5Tcopy (rhs.hStruct);

   return *this;
}

//
// H5_FixedSpace METHODS
//
H5_FixedSpace::H5_FixedSpace (int numDims, hsize_t d1, hsize_t d2, hsize_t d3, hsize_t d4)
{
   hDims  = new Dimensions (numDims, d1, d2, d3, d4);
   hSpace = createSpaceId (hDims);
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
}

H5_FixedSpace::H5_FixedSpace (const Dimensions& dims)
{
   // create class on Dimensions values
   hDims  = new Dimensions (dims);
   hSpace = createSpaceId  (hDims);
}

H5_FixedSpace::H5_FixedSpace (const H5_FixedSpace& rhs)
{
   deepCopy (rhs); 
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

bool H5_FixedSpace::sizeEqual (const H5_FixedSpace& rhs) const
{
   return hDims->sizeEqual (*rhs.hDims);
}

H5_FixedSpace& H5_FixedSpace::deepCopy (const H5_FixedSpace& rhs)
{
   hDims  = new Dimensions (*(rhs.hDims));
   hSpace = createSpaceId  (hDims);

   return *this;
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
