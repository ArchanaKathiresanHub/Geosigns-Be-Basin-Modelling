//
// Classes to read and write buffer data
// from a hd5f file
//

#include <string.h>
#include <iostream>

#include "buffer_readwrite.h"

using namespace std;

bool Buffer_ReadWrite::writeDataset (H5_Write_File *h5File, hid_t locId, 
                                     const char *dataset, const void *array, 
                                     BufferDimensions &dims, hid_t dataType, 
                                     H5_PropertyList *pList)
{
   if ( ! array )
   {
      cout << endl << "Buffer_ReadWrite::write Error: Empty Array" << endl;
      return false;
   }

   bool status = false;

   // create data space 
   H5_FixedSpace dataSpace (dims);
    
   // add dataset
   hid_t dataId = h5File->addDataset (dataset, locId, dataType, dataSpace);

   // write the array to dataset
   if ( dataId > -1 )
   {
      status = h5File->writeDataset (dataId, array, pList);
   }
   
   return status;
}
   
bool Buffer_ReadWrite::readDataset (H5_ReadOnly_File *h5File, hid_t locId, const char *dataset, 
                                    BufferDimensions &dims, void **data, 
                                    H5_PropertyList *pList)
{
   bool status = false;

   // open the dataset
   hid_t dataId = h5File->openDataset (dataset, locId); 

   if ( dataId < 0 )
   {    
      return status;
   }

   // get the dimensions
   const H5_FixedSpace fileSpace (H5Dget_space (dataId));
   dims = fileSpace.h5_dims();

   if ( dims[0] > 0 )
   {    
      // get size and type of data
      hsize_t linearSize = 1;
      for ( int i=0; i < dims.numDimensions(); ++i )
      {
         linearSize *= dims[i];
      }

      hid_t typeId = H5Dget_type (dataId);
      size_t sizeType = H5Tget_size (typeId);
      H5Tclose (typeId);
 
      *data = new char [linearSize * sizeType];

      status = h5File->readDataset (dataId, *data, pList);
   }

   return status;
}

// overload read dataset for when reading a string, as want to null terminate it
bool Buffer_ReadWrite::readDataset (H5_ReadOnly_File *h5File, hid_t locId, const char *dataset, 
                                    BufferDimensions &dims, char **data, 
                                    H5_PropertyList *pList)
{
  int ret = readDataset (h5File, locId, dataset, dims, (void**)data, pList);

  if ( ret )
  {
     char *nullTerminatedStr = new char [dims[0]+1];
     strncpy (nullTerminatedStr, *data, dims[0]);
     nullTerminatedStr[dims[0]] = '\0';
     delete [] *data;
     *data = nullTerminatedStr;
     ++dims[0];
  }

  return ret ? true : false;
}
  
bool Buffer_ReadWrite::writeAttribute (H5_Write_File *h5File, hid_t locId, const char *attribute, 
                                       void *data, BufferDimensions &dims, hid_t dataType, 
                                       H5_PropertyList *pList)
{
   if ( ! data )
   {
      cout << endl << "Buffer_ReadWrite::writeVoidAttribute Error: Empty Array" << endl;
      return false;
   }

   bool status = false;

   // create data space 
   H5_FixedSpace dataSpace (dims);
      
   // add attribute
   hid_t attId = h5File->addAttribute (attribute, locId, dataType, dataSpace);

   // write the array to dataset
   if ( attId > -1 )
   {
     status = h5File->writeAttribute (attId, dataSpace.space_id(), data, pList);
   }
    
   return status;
}

bool Buffer_ReadWrite::readAttribute (H5_ReadOnly_File *h5File, hid_t locId, const char *attribute, 
                                      BufferDimensions &dims, void **data, H5_PropertyList *pList)       
{
   bool status = false;

   // open the attribute
   hid_t attributeId = h5File->openAttribute (attribute, locId); 

   if ( attributeId < 0 )
   { 
      return status;
   }

   // get the dimensions
   const H5_FixedSpace fileSpace (H5Aget_space (attributeId));
   dims = fileSpace.h5_dims();
       
   if ( dims[0] > 0 )
   {
      // get the attribute size and type
      hsize_t linearSize = 1;
      for ( int i=0; i < dims.numDimensions(); ++i )
      {
         linearSize *= dims[i];
      }
   
      size_t sizeType = H5Tget_size (H5Aget_type (attributeId));

      *data = new char [linearSize * sizeType];

      // read the attribute data
      status = h5File->readAttribute (attributeId, fileSpace.space_id(), *data, pList);
   }

   return status;
}

// overload reader for character strings, so can add null terminater
bool Buffer_ReadWrite::readAttribute (H5_ReadOnly_File *h5File, hid_t locId, const char *attribute, 
                                      BufferDimensions &dims, char **data, H5_PropertyList *pList) 
{
  bool ret = readAttribute (h5File, locId, attribute, dims, (void**)data, pList);

  if ( ret )
  {
     char *nullTerminatedStr = new char [dims[0]+1];
     strncpy (nullTerminatedStr, *data, dims[0]);
     nullTerminatedStr[dims[0]] = '\0';
     delete [] *data;
     *data = nullTerminatedStr;
     ++dims[0];
  }

  return ret;
}
